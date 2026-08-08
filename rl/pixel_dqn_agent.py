"""DQN variant that learns from stacked game frames with a convolutional encoder.

The agent reuses the whole training machinery of :class:`DQNAgent` - prioritized
replay, n-step returns, Double DQN targets, and soft target updates - and only
swaps the encoder and the replay dtype. Observations are ``uint8`` frame stacks
of shape ``(frame_stack, height, width)`` produced by
``MarioEnv(observation_mode="pixels")``.
"""

from __future__ import annotations

from pathlib import Path
from typing import Any

import numpy as np
import torch
from torch import nn

from .dqn_agent import DQNAgent, PrioritizedReplayBuffer


PIXEL_SCALE = 255.0


class NatureDuelingQNetwork(nn.Module):
    """Nature-DQN convolutional trunk with dueling value and advantage heads."""

    def __init__(
        self,
        observation_shape: tuple[int, int, int],
        action_count: int,
        hidden_size: int,
    ) -> None:
        super().__init__()
        if len(observation_shape) != 3:
            raise ValueError("pixel observations must be shaped (channels, height, width)")
        channels, height, width = (int(value) for value in observation_shape)
        if channels <= 0 or height <= 0 or width <= 0:
            raise ValueError("pixel observation dimensions must be positive")

        self.observation_shape = (channels, height, width)
        self.action_count = int(action_count)
        self.hidden_size = int(hidden_size)

        self.encoder = nn.Sequential(
            nn.Conv2d(channels, 32, kernel_size=8, stride=4),
            nn.ReLU(),
            nn.Conv2d(32, 64, kernel_size=4, stride=2),
            nn.ReLU(),
            nn.Conv2d(64, 64, kernel_size=3, stride=1),
            nn.ReLU(),
            nn.Flatten(),
        )
        with torch.no_grad():
            encoded_size = self.encoder(torch.zeros(1, channels, height, width)).shape[1]
        if encoded_size <= 0:
            raise ValueError("the frame is too small for this convolutional encoder")

        self.projection = nn.Sequential(
            nn.Linear(encoded_size, hidden_size),
            nn.LayerNorm(hidden_size),
            nn.ReLU(),
        )
        head_size = max(16, hidden_size // 2)
        self.value_head = nn.Sequential(
            nn.Linear(hidden_size, head_size),
            nn.ReLU(),
            nn.Linear(head_size, 1),
        )
        self.advantage_head = nn.Sequential(
            nn.Linear(hidden_size, head_size),
            nn.ReLU(),
            nn.Linear(head_size, action_count),
        )
        self.apply(self._initialize)

    @staticmethod
    def _initialize(module: nn.Module) -> None:
        if isinstance(module, (nn.Linear, nn.Conv2d)):
            nn.init.orthogonal_(module.weight, gain=np.sqrt(2.0))
            if module.bias is not None:
                nn.init.zeros_(module.bias)

    def forward(self, observations: torch.Tensor) -> torch.Tensor:
        # Callers hand over uint8 frames from replay or float copies from act();
        # both arrive on the 0-255 scale, so normalize here rather than in the
        # hot loops that build the batches.
        images = observations.reshape(-1, *self.observation_shape).float() / PIXEL_SCALE
        latent = self.projection(self.encoder(images))
        values = self.value_head(latent)
        advantages = self.advantage_head(latent)
        return values + advantages - advantages.mean(dim=1, keepdim=True)


class PixelDQNAgent(DQNAgent):
    """DQN over stacked frames instead of hand-designed features."""

    def __init__(
        self,
        observation_shape: tuple[int, int, int],
        action_count: int,
        *,
        replay_capacity: int = 20_000,
        **kwargs: Any,
    ) -> None:
        shape = tuple(int(value) for value in observation_shape)
        if len(shape) != 3:
            raise ValueError("pixel observations must be shaped (channels, height, width)")
        # Set before super().__init__ so the build hooks below can read it.
        self.observation_shape = shape
        super().__init__(
            int(np.prod(shape)),
            action_count,
            replay_capacity=replay_capacity,
            **kwargs,
        )

    def _build_network(self) -> nn.Module:
        return NatureDuelingQNetwork(self.observation_shape, self.action_count, self.hidden_size)

    def _build_replay(self) -> PrioritizedReplayBuffer:
        return PrioritizedReplayBuffer(
            self.replay_capacity,
            self.observation_shape,
            alpha=self.priority_alpha,
            seed=self.seed + 1,
            dtype=np.uint8,
        )

    def replay_bytes(self) -> int:
        """Resident size of the replay arrays, useful before a long run."""

        return int(self.replay.observations.nbytes + self.replay.next_observations.nbytes)

    def _config(self) -> dict[str, Any]:
        config = super()._config()
        config["observation_shape"] = list(self.observation_shape)
        return config

    @classmethod
    def _config_to_kwargs(cls, config: dict[str, Any]) -> dict[str, Any]:
        kwargs = dict(config)
        kwargs.pop("observation_size", None)
        kwargs["observation_shape"] = tuple(kwargs["observation_shape"])
        return kwargs


def is_pixel_checkpoint(path: str | Path, *, device: str | torch.device = "cpu") -> bool:
    """Report whether a checkpoint was trained on frames rather than features."""

    payload = DQNAgent._read_checkpoint(path, DQNAgent._resolve_device(device))
    return "observation_shape" in payload["config"]


def load_agent(path: str | Path, *, device: str | torch.device = "auto") -> DQNAgent:
    """Rebuild whichever agent type produced this checkpoint."""

    agent_class = PixelDQNAgent if is_pixel_checkpoint(path) else DQNAgent
    return agent_class.from_checkpoint(path, device=device)
