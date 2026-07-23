"""Stable PyTorch DQN components for the native Mario environment."""

from __future__ import annotations

from collections import deque
from pathlib import Path
from typing import Any

import numpy as np
import torch
from torch import nn
from torch.nn import functional as F


FEATURE_COUNT = 22
GRID_CHANNELS = 4
GRID_ROWS = 9
GRID_COLUMNS = 13
SPATIAL_OBSERVATION_SIZE = FEATURE_COUNT + GRID_CHANNELS * GRID_ROWS * GRID_COLUMNS
CHECKPOINT_VERSION = 2


class PrioritizedReplayBuffer:
    """Proportional prioritized replay backed by a sum tree."""

    def __init__(
        self,
        capacity: int,
        observation_size: int,
        *,
        alpha: float = 0.6,
        seed: int = 0,
    ) -> None:
        if capacity <= 0:
            raise ValueError("replay capacity must be positive")
        if not 0.0 <= alpha <= 1.0:
            raise ValueError("priority alpha must be in [0, 1]")

        self.capacity = int(capacity)
        self.alpha = float(alpha)
        self.observations = np.empty((capacity, observation_size), dtype=np.float32)
        self.next_observations = np.empty((capacity, observation_size), dtype=np.float32)
        self.actions = np.empty(capacity, dtype=np.int64)
        self.rewards = np.empty(capacity, dtype=np.float32)
        self.discounts = np.empty(capacity, dtype=np.float32)

        tree_capacity = 1
        while tree_capacity < capacity:
            tree_capacity *= 2
        self._tree_capacity = tree_capacity
        self._sum_tree = np.zeros(2 * tree_capacity, dtype=np.float64)
        self._max_priority = 1.0
        self.position = 0
        self.size = 0
        self.rng = np.random.default_rng(seed)

    def __len__(self) -> int:
        return self.size

    def _set_priority(self, index: int, priority: float) -> None:
        tree_index = index + self._tree_capacity
        change = priority - self._sum_tree[tree_index]
        while tree_index >= 1:
            self._sum_tree[tree_index] += change
            tree_index //= 2

    def add(
        self,
        observation: np.ndarray,
        action: int,
        reward: float,
        next_observation: np.ndarray,
        discount: float,
    ) -> None:
        index = self.position
        self.observations[index] = observation
        self.next_observations[index] = next_observation
        self.actions[index] = action
        self.rewards[index] = reward
        self.discounts[index] = discount
        self._set_priority(index, self._max_priority**self.alpha)
        self.position = (self.position + 1) % self.capacity
        self.size = min(self.size + 1, self.capacity)

    def _find_prefix_index(self, mass: float) -> int:
        tree_index = 1
        while tree_index < self._tree_capacity:
            left = tree_index * 2
            if mass < self._sum_tree[left]:
                tree_index = left
            else:
                mass -= self._sum_tree[left]
                tree_index = left + 1
        return tree_index - self._tree_capacity

    def sample(self, batch_size: int, beta: float) -> tuple[np.ndarray, ...]:
        if batch_size <= 0:
            raise ValueError("batch size must be positive")
        if self.size < batch_size:
            raise ValueError("not enough replay entries to sample a batch")
        if not 0.0 <= beta <= 1.0:
            raise ValueError("importance-sampling beta must be in [0, 1]")

        total_priority = float(self._sum_tree[1])
        if total_priority <= 0.0:
            raise RuntimeError("replay priorities must have a positive sum")

        segment = total_priority / batch_size
        masses = (np.arange(batch_size) + self.rng.random(batch_size)) * segment
        indices = np.fromiter(
            (self._find_prefix_index(float(mass)) for mass in masses),
            dtype=np.int64,
            count=batch_size,
        )
        probabilities = self._sum_tree[indices + self._tree_capacity] / total_priority
        weights = np.power(self.size * probabilities, -beta)
        weights /= weights.max()

        return (
            self.observations[indices],
            self.actions[indices],
            self.rewards[indices],
            self.next_observations[indices],
            self.discounts[indices],
            indices,
            weights.astype(np.float32),
        )

    def update_priorities(self, indices: np.ndarray, priorities: np.ndarray) -> None:
        for index, priority in zip(indices, priorities, strict=True):
            raw_priority = max(float(priority), 1e-6)
            if not np.isfinite(raw_priority):
                raise ValueError("replay priorities must be finite")
            self._max_priority = max(self._max_priority, raw_priority)
            self._set_priority(int(index), raw_priority**self.alpha)


class DuelingQNetwork(nn.Module):
    """Dueling network with a spatial encoder for Mario's local tile grid."""

    def __init__(self, observation_size: int, action_count: int, hidden_size: int) -> None:
        super().__init__()
        self.observation_size = int(observation_size)
        self.action_count = int(action_count)
        self.hidden_size = int(hidden_size)
        self.uses_spatial_encoder = observation_size == SPATIAL_OBSERVATION_SIZE

        if self.uses_spatial_encoder:
            self.feature_encoder = nn.Sequential(
                nn.Linear(FEATURE_COUNT, 64),
                nn.LayerNorm(64),
                nn.ReLU(),
            )
            self.grid_encoder = nn.Sequential(
                nn.Conv2d(GRID_CHANNELS, 32, kernel_size=3, padding=1),
                nn.ReLU(),
                nn.Conv2d(32, 64, kernel_size=3, stride=2, padding=1),
                nn.ReLU(),
                nn.Flatten(),
                nn.Linear(64 * 5 * 7, hidden_size),
                nn.LayerNorm(hidden_size),
                nn.ReLU(),
            )
            trunk_input_size = hidden_size + 64
        else:
            self.observation_encoder = nn.Sequential(
                nn.Linear(observation_size, hidden_size),
                nn.LayerNorm(hidden_size),
                nn.ReLU(),
                nn.Linear(hidden_size, hidden_size),
                nn.ReLU(),
            )
            trunk_input_size = hidden_size

        self.trunk = nn.Sequential(
            nn.Linear(trunk_input_size, hidden_size),
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
        if self.uses_spatial_encoder:
            features = self.feature_encoder(observations[:, :FEATURE_COUNT])
            grid = observations[:, FEATURE_COUNT:].reshape(
                -1, GRID_CHANNELS, GRID_ROWS, GRID_COLUMNS
            )
            encoded = torch.cat((features, self.grid_encoder(grid)), dim=1)
        else:
            encoded = self.observation_encoder(observations)

        latent = self.trunk(encoded)
        values = self.value_head(latent)
        advantages = self.advantage_head(latent)
        return values + advantages - advantages.mean(dim=1, keepdim=True)


class DQNAgent:
    """Rainbow-lite DQN with PyTorch, PER, n-step returns, and soft targets."""

    def __init__(
        self,
        observation_size: int,
        action_count: int,
        *,
        hidden_size: int = 256,
        replay_capacity: int = 50_000,
        gamma: float = 0.99,
        learning_rate: float = 1e-4,
        epsilon_start: float = 1.0,
        epsilon_end: float = 0.05,
        epsilon_decay_steps: int = 250_000,
        target_update_interval: int = 1,
        tau: float = 0.005,
        n_step: int = 3,
        priority_alpha: float = 0.6,
        priority_beta_start: float = 0.4,
        priority_beta_steps: int = 250_000,
        max_grad_norm: float = 10.0,
        seed: int = 0,
        device: str | torch.device = "auto",
    ) -> None:
        if observation_size <= 0 or action_count <= 0 or hidden_size <= 0:
            raise ValueError("network dimensions must be positive")
        if not 0.0 <= gamma <= 1.0:
            raise ValueError("gamma must be in [0, 1]")
        if learning_rate <= 0.0:
            raise ValueError("learning rate must be positive")
        if not 0.0 <= epsilon_end <= epsilon_start <= 1.0:
            raise ValueError("epsilon values must satisfy 0 <= end <= start <= 1")
        if epsilon_decay_steps <= 0:
            raise ValueError("epsilon decay steps must be positive")
        if n_step <= 0:
            raise ValueError("n_step must be positive")
        if not 0.0 < tau <= 1.0:
            raise ValueError("tau must be in (0, 1]")
        if target_update_interval <= 0:
            raise ValueError("target update interval must be positive")
        if not 0.0 <= priority_beta_start <= 1.0:
            raise ValueError("priority beta must be in [0, 1]")
        if priority_beta_steps <= 0:
            raise ValueError("priority beta steps must be positive")
        if max_grad_norm <= 0.0:
            raise ValueError("maximum gradient norm must be positive")

        self.observation_size = int(observation_size)
        self.action_count = int(action_count)
        self.hidden_size = int(hidden_size)
        self.replay_capacity = int(replay_capacity)
        self.gamma = float(gamma)
        self.learning_rate = float(learning_rate)
        self.epsilon_start = float(epsilon_start)
        self.epsilon_end = float(epsilon_end)
        self.epsilon_decay_steps = int(epsilon_decay_steps)
        self.target_update_interval = int(target_update_interval)
        self.tau = float(tau)
        self.n_step = int(n_step)
        self.priority_alpha = float(priority_alpha)
        self.priority_beta_start = float(priority_beta_start)
        self.priority_beta_steps = int(priority_beta_steps)
        self.max_grad_norm = float(max_grad_norm)
        self.seed = int(seed)
        self.device = self._resolve_device(device)

        self.rng = np.random.default_rng(seed)
        torch.manual_seed(seed)
        if torch.cuda.is_available():
            torch.cuda.manual_seed_all(seed)

        self.online = DuelingQNetwork(observation_size, action_count, hidden_size).to(self.device)
        self.target = DuelingQNetwork(observation_size, action_count, hidden_size).to(self.device)
        self.target.load_state_dict(self.online.state_dict())
        self.target.requires_grad_(False)
        self.target.eval()
        self.optimizer = torch.optim.AdamW(
            self.online.parameters(),
            lr=learning_rate,
            eps=1e-5,
            weight_decay=1e-5,
        )
        self.replay = PrioritizedReplayBuffer(
            replay_capacity,
            observation_size,
            alpha=priority_alpha,
            seed=seed + 1,
        )
        self._n_step_buffer: deque[tuple[np.ndarray, int, float, np.ndarray, bool]] = deque()
        self.total_steps = 0
        self.gradient_steps = 0

    @staticmethod
    def _resolve_device(device: str | torch.device) -> torch.device:
        requested = str(device)
        if requested == "auto":
            if torch.cuda.is_available():
                return torch.device("cuda")
            if torch.backends.mps.is_available():
                return torch.device("mps")
            return torch.device("cpu")
        resolved = torch.device(requested)
        if resolved.type == "cuda" and not torch.cuda.is_available():
            raise ValueError("CUDA was requested but is not available")
        if resolved.type == "mps" and not torch.backends.mps.is_available():
            raise ValueError("MPS was requested but is not available")
        return resolved

    @property
    def epsilon(self) -> float:
        fraction = min(1.0, self.total_steps / max(1, self.epsilon_decay_steps))
        return self.epsilon_start + fraction * (self.epsilon_end - self.epsilon_start)

    @property
    def priority_beta(self) -> float:
        fraction = min(1.0, self.total_steps / max(1, self.priority_beta_steps))
        return self.priority_beta_start + fraction * (1.0 - self.priority_beta_start)

    def act(self, observation: np.ndarray, *, explore: bool = True) -> int:
        if explore and self.rng.random() < self.epsilon:
            return int(self.rng.integers(self.action_count))
        observation_tensor = torch.as_tensor(
            np.asarray(observation, dtype=np.float32),
            device=self.device,
        ).unsqueeze(0)
        with torch.no_grad():
            q_values = self.online(observation_tensor)
        return int(q_values.argmax(dim=1).item())

    def remember(
        self,
        observation: np.ndarray,
        action: int,
        reward: float,
        next_observation: np.ndarray,
        terminated: bool,
        *,
        episode_end: bool | None = None,
    ) -> None:
        """Store a transition, keeping time-limit truncation bootstrap-safe.

        ``terminated`` means the environment reached a true terminal state.
        ``episode_end`` additionally marks a time-limit truncation and flushes
        the pending n-step transitions without zeroing their bootstrap value.
        """

        if episode_end is None:
            episode_end = bool(terminated)
        episode_end = bool(episode_end or terminated)
        if not 0 <= int(action) < self.action_count:
            raise ValueError("action is outside the discrete action space")
        transition = (
            np.asarray(observation, dtype=np.float32).copy(),
            int(action),
            float(reward),
            np.asarray(next_observation, dtype=np.float32).copy(),
            bool(terminated),
        )
        self._n_step_buffer.append(transition)
        self.total_steps += 1

        if len(self._n_step_buffer) >= self.n_step:
            self._commit_n_step_transition()
            self._n_step_buffer.popleft()

        if episode_end:
            while self._n_step_buffer:
                self._commit_n_step_transition()
                self._n_step_buffer.popleft()

    def _commit_n_step_transition(self) -> None:
        observation, action = self._n_step_buffer[0][:2]
        cumulative_reward = 0.0
        steps = 0
        terminal = False
        next_observation = self._n_step_buffer[0][3]

        for _, _, reward, candidate_next, candidate_terminal in list(self._n_step_buffer)[: self.n_step]:
            cumulative_reward += (self.gamma**steps) * reward
            steps += 1
            next_observation = candidate_next
            terminal = candidate_terminal
            if terminal:
                break

        discount = 0.0 if terminal else self.gamma**steps
        self.replay.add(observation, action, cumulative_reward, next_observation, discount)

    def train_step(self, batch_size: int = 64) -> dict[str, float] | None:
        if len(self.replay) < batch_size:
            return None

        (
            observations,
            actions,
            rewards,
            next_observations,
            discounts,
            indices,
            weights,
        ) = self.replay.sample(batch_size, self.priority_beta)

        observations_tensor = torch.as_tensor(observations, device=self.device)
        actions_tensor = torch.as_tensor(actions, device=self.device).unsqueeze(1)
        rewards_tensor = torch.as_tensor(rewards, device=self.device)
        next_observations_tensor = torch.as_tensor(next_observations, device=self.device)
        discounts_tensor = torch.as_tensor(discounts, device=self.device)
        weights_tensor = torch.as_tensor(weights, device=self.device)

        q_values = self.online(observations_tensor).gather(1, actions_tensor).squeeze(1)
        with torch.no_grad():
            next_actions = self.online(next_observations_tensor).argmax(dim=1, keepdim=True)
            next_q_values = self.target(next_observations_tensor).gather(1, next_actions).squeeze(1)
            targets = rewards_tensor + discounts_tensor * next_q_values

        td_errors = targets - q_values
        element_losses = F.smooth_l1_loss(q_values, targets, reduction="none")
        loss = (weights_tensor * element_losses).mean()

        self.optimizer.zero_grad(set_to_none=True)
        loss.backward()
        gradient_norm = nn.utils.clip_grad_norm_(self.online.parameters(), self.max_grad_norm)
        self.optimizer.step()
        self.gradient_steps += 1

        priorities = td_errors.detach().abs().cpu().numpy() + 1e-5
        self.replay.update_priorities(indices, priorities)
        if self.gradient_steps % self.target_update_interval == 0:
            self._soft_update_target()

        return {
            "loss": float(loss.detach().cpu()),
            "q_mean": float(q_values.detach().mean().cpu()),
            "target_q_mean": float(targets.mean().cpu()),
            "td_error_mean": float(td_errors.detach().abs().mean().cpu()),
            "gradient_norm": float(torch.as_tensor(gradient_norm).detach().cpu()),
            "priority_beta": self.priority_beta,
            "learning_rate": float(self.optimizer.param_groups[0]["lr"]),
        }

    @torch.no_grad()
    def _soft_update_target(self) -> None:
        for target_parameter, online_parameter in zip(
            self.target.parameters(), self.online.parameters(), strict=True
        ):
            target_parameter.lerp_(online_parameter, self.tau)

    def _config(self) -> dict[str, Any]:
        return {
            "observation_size": self.observation_size,
            "action_count": self.action_count,
            "hidden_size": self.hidden_size,
            "replay_capacity": self.replay_capacity,
            "gamma": self.gamma,
            "learning_rate": self.learning_rate,
            "epsilon_start": self.epsilon_start,
            "epsilon_end": self.epsilon_end,
            "epsilon_decay_steps": self.epsilon_decay_steps,
            "target_update_interval": self.target_update_interval,
            "tau": self.tau,
            "n_step": self.n_step,
            "priority_alpha": self.priority_alpha,
            "priority_beta_start": self.priority_beta_start,
            "priority_beta_steps": self.priority_beta_steps,
            "max_grad_norm": self.max_grad_norm,
            "seed": self.seed,
        }

    def save(self, path: str | Path) -> Path:
        checkpoint = Path(path)
        if checkpoint.suffix not in {".pt", ".pth"}:
            checkpoint = checkpoint.with_suffix(".pt")
        checkpoint.parent.mkdir(parents=True, exist_ok=True)
        payload = {
            "version": CHECKPOINT_VERSION,
            "config": self._config(),
            "online_state_dict": self.online.state_dict(),
            "target_state_dict": self.target.state_dict(),
            "optimizer_state_dict": self.optimizer.state_dict(),
            "total_steps": self.total_steps,
            "gradient_steps": self.gradient_steps,
            "numpy_rng_state": self.rng.bit_generator.state,
            "torch_rng_state": torch.get_rng_state(),
        }
        temporary = checkpoint.with_suffix(checkpoint.suffix + ".tmp")
        torch.save(payload, temporary)
        temporary.replace(checkpoint)
        return checkpoint

    @staticmethod
    def _read_checkpoint(path: str | Path, device: torch.device) -> dict[str, Any]:
        checkpoint = Path(path)
        if checkpoint.suffix == ".npz":
            raise ValueError(
                "NumPy DQN checkpoints are not compatible with the new PyTorch network; "
                "start a new .pt training run"
            )
        payload = torch.load(checkpoint, map_location=device, weights_only=True)
        if payload.get("version") != CHECKPOINT_VERSION:
            raise ValueError(f"unsupported checkpoint version: {payload.get('version')}")
        return payload

    def load(self, path: str | Path, *, load_optimizer: bool = True) -> None:
        payload = self._read_checkpoint(path, self.device)
        config = payload["config"]
        expected = (
            int(config["observation_size"]),
            int(config["action_count"]),
            int(config["hidden_size"]),
        )
        actual = (self.observation_size, self.action_count, self.hidden_size)
        if expected != actual:
            raise ValueError(f"checkpoint shape {expected} does not match agent shape {actual}")

        self.online.load_state_dict(payload["online_state_dict"])
        self.target.load_state_dict(payload["target_state_dict"])
        if load_optimizer:
            self.optimizer.load_state_dict(payload["optimizer_state_dict"])
            for state in self.optimizer.state.values():
                for name, value in state.items():
                    if isinstance(value, torch.Tensor):
                        state[name] = value.to(self.device)
        self.total_steps = int(payload["total_steps"])
        self.gradient_steps = int(payload["gradient_steps"])
        if "numpy_rng_state" in payload:
            self.rng.bit_generator.state = payload["numpy_rng_state"]
        if "torch_rng_state" in payload:
            torch.set_rng_state(payload["torch_rng_state"].cpu())

    @classmethod
    def from_checkpoint(
        cls,
        path: str | Path,
        *,
        device: str | torch.device = "auto",
    ) -> "DQNAgent":
        resolved_device = cls._resolve_device(device)
        payload = cls._read_checkpoint(path, resolved_device)
        config = dict(payload["config"])
        # Evaluation does not need a large replay allocation.
        config["replay_capacity"] = 1
        config["device"] = resolved_device
        agent = cls(**config)
        agent.online.load_state_dict(payload["online_state_dict"])
        agent.target.load_state_dict(payload["target_state_dict"])
        agent.total_steps = int(payload["total_steps"])
        agent.gradient_steps = int(payload["gradient_steps"])
        agent.online.eval()
        return agent
