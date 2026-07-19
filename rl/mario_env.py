"""A small Gymnasium-compatible wrapper around the native C++ game engine."""

from __future__ import annotations

import ctypes
import os
from pathlib import Path
from typing import Any

import numpy as np

try:  # Gymnasium is optional; the built-in API has the same reset/step shape.
    import gymnasium as gym
    from gymnasium import spaces

    BaseEnv = gym.Env
except ImportError:  # pragma: no cover - used on dependency-minimal installs
    gym = None
    spaces = None
    BaseEnv = object


ACTION_NAMES = (
    "idle",
    "left",
    "right",
    "jump",
    "left+jump",
    "right+jump",
    "shoot",
    "left+shoot",
    "right+shoot",
)


class _Discrete:
    def __init__(self, size: int) -> None:
        self.n = size

    def sample(self) -> int:
        return int(np.random.randint(self.n))


class _Box:
    def __init__(self, shape: tuple[int, ...]) -> None:
        self.shape = shape
        self.dtype = np.dtype(np.float32)


class _NativeStepResult(ctypes.Structure):
    _fields_ = [
        ("reward", ctypes.c_float),
        ("progress", ctypes.c_float),
        ("score", ctypes.c_int),
        ("episode_steps", ctypes.c_int),
        ("player_x", ctypes.c_int),
        ("terminated", ctypes.c_int),
        ("truncated", ctypes.c_int),
        ("won", ctypes.c_int),
    ]


def _default_project_root() -> Path:
    return Path(__file__).resolve().parents[1]


def _find_library(project_root: Path, library_path: str | os.PathLike[str] | None) -> Path:
    if library_path is not None:
        candidate = Path(library_path).expanduser().resolve()
        if candidate.is_file():
            return candidate
        raise FileNotFoundError(f"RL library not found: {candidate}")

    names = ("libmario_rl.so", "libmario_rl.dylib", "mario_rl.dll")
    build_roots = (project_root / "build", project_root / "cmake-build-debug")
    for build_root in build_roots:
        for name in names:
            direct = build_root / name
            if direct.is_file():
                return direct
            matches = list(build_root.glob(f"**/{name}")) if build_root.is_dir() else []
            if matches:
                return matches[0]

    raise FileNotFoundError(
        "The native RL library has not been built. Run "
        "`cmake -S . -B build -DBUILD_TESTING=ON && cmake --build build -j` first."
    )


def _configure_library(library: ctypes.CDLL) -> None:
    float_pointer = ctypes.POINTER(ctypes.c_float)
    library.mario_rl_create.argtypes = [ctypes.c_char_p, ctypes.c_int, ctypes.c_int, ctypes.c_int]
    library.mario_rl_create.restype = ctypes.c_void_p
    library.mario_rl_destroy.argtypes = [ctypes.c_void_p]
    library.mario_rl_observation_size.restype = ctypes.c_int
    library.mario_rl_action_count.restype = ctypes.c_int
    library.mario_rl_reset.argtypes = [ctypes.c_void_p, float_pointer]
    library.mario_rl_reset.restype = ctypes.c_int
    library.mario_rl_step.argtypes = [
        ctypes.c_void_p,
        ctypes.c_int,
        float_pointer,
        ctypes.POINTER(_NativeStepResult),
    ]
    library.mario_rl_step.restype = ctypes.c_int
    library.mario_rl_last_error.restype = ctypes.c_char_p


class MarioEnv(BaseEnv):
    """Fixed-step access to the real C++ level simulation.

    Observations contain normalized player features followed by four local
    tile-grid channels: terrain, reward blocks, enemies, and power-ups.
    Rendering and audio are intentionally skipped during training.
    """

    metadata = {"render_modes": []}

    def __init__(
        self,
        level: int = 1,
        max_episode_steps: int = 3000,
        frame_skip: int = 4,
        project_root: str | os.PathLike[str] | None = None,
        library_path: str | os.PathLike[str] | None = None,
    ) -> None:
        root = Path(project_root).expanduser().resolve() if project_root else _default_project_root()
        native_path = _find_library(root, library_path)
        self._library = ctypes.CDLL(str(native_path))
        _configure_library(self._library)

        self.observation_size = int(self._library.mario_rl_observation_size())
        self.action_count = int(self._library.mario_rl_action_count())
        if self.action_count != len(ACTION_NAMES):
            raise RuntimeError("Python action names do not match the native action space")

        self._handle = self._library.mario_rl_create(
            os.fsencode(root), int(level), int(max_episode_steps), int(frame_skip)
        )
        if not self._handle:
            self._raise_native_error("could not create Mario environment")

        self._observation = np.empty(self.observation_size, dtype=np.float32)
        if spaces is not None:
            self.action_space = spaces.Discrete(self.action_count)
            self.observation_space = spaces.Box(
                low=-1.0, high=2.0, shape=(self.observation_size,), dtype=np.float32
            )
        else:
            self.action_space = _Discrete(self.action_count)
            self.observation_space = _Box((self.observation_size,))

    def _observation_pointer(self) -> ctypes.POINTER(ctypes.c_float):
        return self._observation.ctypes.data_as(ctypes.POINTER(ctypes.c_float))

    def _raise_native_error(self, prefix: str) -> None:
        raw_error = self._library.mario_rl_last_error()
        detail = raw_error.decode("utf-8", errors="replace") if raw_error else "unknown native error"
        raise RuntimeError(f"{prefix}: {detail}")

    def reset(
        self,
        *,
        seed: int | None = None,
        options: dict[str, Any] | None = None,
    ) -> tuple[np.ndarray, dict[str, Any]]:
        del options
        if gym is not None:
            super().reset(seed=seed)
        elif seed is not None:
            np.random.seed(seed)
        if not self._handle:
            raise RuntimeError("cannot reset a closed environment")
        if self._library.mario_rl_reset(self._handle, self._observation_pointer()) != 0:
            self._raise_native_error("reset failed")
        return self._observation.copy(), {"score": 0, "progress": 0.0, "won": False}

    def step(self, action: int) -> tuple[np.ndarray, float, bool, bool, dict[str, Any]]:
        if not self._handle:
            raise RuntimeError("cannot step a closed environment")
        result = _NativeStepResult()
        status = self._library.mario_rl_step(
            self._handle,
            int(action),
            self._observation_pointer(),
            ctypes.byref(result),
        )
        if status != 0:
            self._raise_native_error("step failed")
        info = {
            "score": int(result.score),
            "progress": float(result.progress),
            "episode_steps": int(result.episode_steps),
            "player_x": int(result.player_x),
            "won": bool(result.won),
            "action_name": ACTION_NAMES[int(action)] if 0 <= int(action) < len(ACTION_NAMES) else "invalid",
        }
        return (
            self._observation.copy(),
            float(result.reward),
            bool(result.terminated),
            bool(result.truncated),
            info,
        )

    def close(self) -> None:
        if getattr(self, "_handle", None):
            self._library.mario_rl_destroy(self._handle)
            self._handle = None

    def __enter__(self) -> "MarioEnv":
        return self

    def __exit__(self, *_: object) -> None:
        self.close()

    def __del__(self) -> None:
        try:
            self.close()
        except Exception:
            pass
