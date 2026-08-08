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
    def __init__(self, shape: tuple[int, ...], dtype: Any = np.float32) -> None:
        self.shape = shape
        self.dtype = np.dtype(dtype)


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
        ("user_quit", ctypes.c_int),
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
    byte_pointer = ctypes.POINTER(ctypes.c_ubyte)
    library.mario_rl_create.argtypes = [ctypes.c_char_p, ctypes.c_int, ctypes.c_int, ctypes.c_int]
    library.mario_rl_create.restype = ctypes.c_void_p
    if hasattr(library, "mario_rl_create_rendered"):
        library.mario_rl_create_rendered.argtypes = [
            ctypes.c_char_p,
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
        ]
        library.mario_rl_create_rendered.restype = ctypes.c_void_p
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
    if hasattr(library, "mario_rl_render"):
        library.mario_rl_render.argtypes = [ctypes.c_void_p]
        library.mario_rl_render.restype = ctypes.c_int
    if hasattr(library, "mario_rl_frame"):
        library.mario_rl_frame_width.restype = ctypes.c_int
        library.mario_rl_frame_height.restype = ctypes.c_int
        library.mario_rl_frame.argtypes = [ctypes.c_void_p, byte_pointer]
        library.mario_rl_frame.restype = ctypes.c_int
    library.mario_rl_last_error.restype = ctypes.c_char_p


class MarioEnv(BaseEnv):
    """Fixed-step access to the real C++ level simulation.

    With ``observation_mode="vector"`` (the default) observations contain
    normalized player features followed by four local tile-grid channels:
    terrain, reward blocks, enemies, and power-ups. With
    ``observation_mode="pixels"`` each observation is instead a stack of
    ``frame_stack`` grayscale 84x84 images of the visible view, shaped
    ``(frame_stack, 84, 84)`` and typed ``uint8``, for convolutional agents.
    Rendering is skipped by default for training. ``render_mode="human"``
    displays the same native world while an agent chooses actions.
    """

    metadata = {"render_modes": ["human"], "render_fps": 30}

    def __init__(
        self,
        level: int = 1,
        max_episode_steps: int = 3000,
        frame_skip: int = 4,
        project_root: str | os.PathLike[str] | None = None,
        library_path: str | os.PathLike[str] | None = None,
        render_mode: str | None = None,
        render_fps: int = 30,
        observation_mode: str = "vector",
        frame_stack: int = 4,
    ) -> None:
        if render_mode not in (None, "human"):
            raise ValueError("render_mode must be None or 'human'")
        if render_fps <= 0 or render_fps > 240:
            raise ValueError("render_fps must be between 1 and 240")
        if observation_mode not in ("vector", "pixels"):
            raise ValueError("observation_mode must be 'vector' or 'pixels'")
        if frame_stack <= 0 or frame_stack > 16:
            raise ValueError("frame_stack must be between 1 and 16")
        root = Path(project_root).expanduser().resolve() if project_root else _default_project_root()
        native_path = _find_library(root, library_path)
        self._library = ctypes.CDLL(str(native_path))
        _configure_library(self._library)
        self.render_mode = render_mode

        self.observation_size = int(self._library.mario_rl_observation_size())
        self.action_count = int(self._library.mario_rl_action_count())
        if self.action_count != len(ACTION_NAMES):
            raise RuntimeError("Python action names do not match the native action space")

        if render_mode == "human":
            if not hasattr(self._library, "mario_rl_create_rendered"):
                raise RuntimeError(
                    "the native RL library does not support rendering; rebuild it with CMake"
                )
            self._handle = self._library.mario_rl_create_rendered(
                os.fsencode(root),
                int(level),
                int(max_episode_steps),
                int(frame_skip),
                int(render_fps),
            )
        else:
            self._handle = self._library.mario_rl_create(
                os.fsencode(root), int(level), int(max_episode_steps), int(frame_skip)
            )
        if not self._handle:
            self._raise_native_error("could not create Mario environment")

        self._observation = np.empty(self.observation_size, dtype=np.float32)
        self.observation_mode = observation_mode
        self.frame_stack = int(frame_stack)
        self._frames: np.ndarray | None = None
        if observation_mode == "pixels":
            if not hasattr(self._library, "mario_rl_frame"):
                raise RuntimeError(
                    "the native RL library has no frame API; rebuild it with CMake"
                )
            self.frame_height = int(self._library.mario_rl_frame_height())
            self.frame_width = int(self._library.mario_rl_frame_width())
            self.observation_shape = (self.frame_stack, self.frame_height, self.frame_width)
            self._frames = np.zeros(self.observation_shape, dtype=np.uint8)
        else:
            self.frame_height = 0
            self.frame_width = 0
            self.observation_shape = (self.observation_size,)

        if spaces is not None:
            self.action_space = spaces.Discrete(self.action_count)
            if observation_mode == "pixels":
                self.observation_space = spaces.Box(
                    low=0, high=255, shape=self.observation_shape, dtype=np.uint8
                )
            else:
                self.observation_space = spaces.Box(
                    low=-1.0, high=2.0, shape=self.observation_shape, dtype=np.float32
                )
        else:
            self.action_space = _Discrete(self.action_count)
            self.observation_space = _Box(
                self.observation_shape,
                np.uint8 if observation_mode == "pixels" else np.float32,
            )

    def _observation_pointer(self) -> ctypes.POINTER(ctypes.c_float):
        return self._observation.ctypes.data_as(ctypes.POINTER(ctypes.c_float))

    def _read_frame(self, destination: np.ndarray) -> None:
        pointer = destination.ctypes.data_as(ctypes.POINTER(ctypes.c_ubyte))
        if self._library.mario_rl_frame(self._handle, pointer) != 0:
            self._raise_native_error("reading the frame failed")

    def _push_frame(self) -> None:
        """Shift the stack by one and write the newest frame into the last slot."""

        self._frames[:-1] = self._frames[1:]
        self._read_frame(self._frames[-1])

    def _current_observation(self) -> np.ndarray:
        if self.observation_mode == "pixels":
            return self._frames.copy()
        return self._observation.copy()

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
        if self.observation_mode == "pixels":
            # Start the episode with the same frame repeated so the stack never
            # carries pixels from the previous episode.
            self._read_frame(self._frames[-1])
            self._frames[:] = self._frames[-1]
        return self._current_observation(), {
            "score": 0,
            "progress": 0.0,
            "won": False,
            "user_quit": False,
        }

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
        if self.observation_mode == "pixels":
            self._push_frame()
        info = {
            "score": int(result.score),
            "progress": float(result.progress),
            "episode_steps": int(result.episode_steps),
            "player_x": int(result.player_x),
            "won": bool(result.won),
            "user_quit": bool(result.user_quit),
            "action_name": ACTION_NAMES[int(action)] if 0 <= int(action) < len(ACTION_NAMES) else "invalid",
        }
        return (
            self._current_observation(),
            float(result.reward),
            bool(result.terminated),
            bool(result.truncated),
            info,
        )

    def render(self) -> bool:
        """Draw one GUI frame and return false after the user closes it."""

        if self.render_mode != "human":
            raise RuntimeError("render() requires render_mode='human'")
        if not self._handle:
            raise RuntimeError("cannot render a closed environment")
        if not hasattr(self._library, "mario_rl_render"):
            raise RuntimeError("the native RL library does not support rendering")
        status = int(self._library.mario_rl_render(self._handle))
        if status < 0:
            self._raise_native_error("render failed")
        return status == 0

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
