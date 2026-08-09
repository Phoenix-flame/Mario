"""Checkpoint-to-agent dispatch shared by the trainer, evaluator, and player."""

from __future__ import annotations

from pathlib import Path
from typing import Any

import torch

from .dqn_agent import DQNAgent
from .pixel_dqn_agent import PixelDQNAgent
from .sac_agent import SACAgent


ALGORITHMS = ("dqn", "sac")


def read_checkpoint_header(path: str | Path) -> dict[str, Any]:
    """Read only what identifies a checkpoint, without building any network."""

    checkpoint = Path(path)
    if checkpoint.suffix == ".npz":
        raise ValueError(
            "NumPy DQN checkpoints are not compatible with the PyTorch networks; "
            "start a new .pt training run"
        )
    payload = torch.load(checkpoint, map_location="cpu", weights_only=True)
    config = payload.get("config", {})
    return {
        "algorithm": payload.get("algorithm", "dqn"),
        "uses_pixels": "observation_shape" in config,
        "config": config,
    }


def agent_class(algorithm: str, uses_pixels: bool) -> type[DQNAgent]:
    if algorithm == "sac":
        # One SAC class covers both observation kinds; the shape picks the encoder.
        return SACAgent
    if algorithm != "dqn":
        raise ValueError(f"unknown algorithm: {algorithm}")
    return PixelDQNAgent if uses_pixels else DQNAgent


def build_agent(
    algorithm: str,
    observation: int | tuple[int, int, int],
    action_count: int,
    **kwargs: Any,
) -> DQNAgent:
    """Construct a fresh agent for training."""

    uses_pixels = isinstance(observation, (tuple, list))
    return agent_class(algorithm, uses_pixels)(observation, action_count, **kwargs)


def load_agent(path: str | Path, *, device: str | torch.device = "auto") -> DQNAgent:
    """Rebuild whichever agent type produced this checkpoint."""

    header = read_checkpoint_header(path)
    selected = agent_class(header["algorithm"], header["uses_pixels"])
    return selected.from_checkpoint(path, device=device)
