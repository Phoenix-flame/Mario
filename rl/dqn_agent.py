"""Dependency-light Double DQN implementation using NumPy."""

from __future__ import annotations

from pathlib import Path

import numpy as np


class ReplayBuffer:
    def __init__(self, capacity: int, observation_size: int, seed: int = 0) -> None:
        self.capacity = int(capacity)
        self.observations = np.empty((capacity, observation_size), dtype=np.float32)
        self.next_observations = np.empty((capacity, observation_size), dtype=np.float32)
        self.actions = np.empty(capacity, dtype=np.int64)
        self.rewards = np.empty(capacity, dtype=np.float32)
        self.dones = np.empty(capacity, dtype=np.float32)
        self.position = 0
        self.size = 0
        self.rng = np.random.default_rng(seed)

    def add(
        self,
        observation: np.ndarray,
        action: int,
        reward: float,
        next_observation: np.ndarray,
        done: bool,
    ) -> None:
        index = self.position
        self.observations[index] = observation
        self.next_observations[index] = next_observation
        self.actions[index] = action
        self.rewards[index] = reward
        self.dones[index] = float(done)
        self.position = (self.position + 1) % self.capacity
        self.size = min(self.size + 1, self.capacity)

    def sample(self, batch_size: int) -> tuple[np.ndarray, ...]:
        indices = self.rng.integers(0, self.size, size=batch_size)
        return (
            self.observations[indices],
            self.actions[indices],
            self.rewards[indices],
            self.next_observations[indices],
            self.dones[indices],
        )


class MLPQNetwork:
    def __init__(self, observation_size: int, action_count: int, hidden_size: int, rng: np.random.Generator):
        self.parameters = {
            "w1": (rng.standard_normal((observation_size, hidden_size)) * np.sqrt(2 / observation_size)).astype(np.float32),
            "b1": np.zeros(hidden_size, dtype=np.float32),
            "w2": (rng.standard_normal((hidden_size, hidden_size)) * np.sqrt(2 / hidden_size)).astype(np.float32),
            "b2": np.zeros(hidden_size, dtype=np.float32),
            "w3": (rng.standard_normal((hidden_size, action_count)) * np.sqrt(2 / hidden_size)).astype(np.float32),
            "b3": np.zeros(action_count, dtype=np.float32),
        }

    def forward(self, observations: np.ndarray, cache: bool = False):
        p = self.parameters
        z1 = observations @ p["w1"] + p["b1"]
        a1 = np.maximum(z1, 0.0)
        z2 = a1 @ p["w2"] + p["b2"]
        a2 = np.maximum(z2, 0.0)
        q_values = a2 @ p["w3"] + p["b3"]
        if cache:
            return q_values, (observations, z1, a1, z2, a2)
        return q_values

    def copy_from(self, other: "MLPQNetwork") -> None:
        for name in self.parameters:
            self.parameters[name][...] = other.parameters[name]


class DQNAgent:
    """Double DQN with replay, a target network, Huber loss, and Adam."""

    def __init__(
        self,
        observation_size: int,
        action_count: int,
        *,
        hidden_size: int = 128,
        replay_capacity: int = 25_000,
        gamma: float = 0.99,
        learning_rate: float = 3e-4,
        epsilon_start: float = 1.0,
        epsilon_end: float = 0.05,
        epsilon_decay_steps: int = 75_000,
        target_update_interval: int = 1_000,
        seed: int = 0,
    ) -> None:
        self.observation_size = int(observation_size)
        self.action_count = int(action_count)
        self.hidden_size = int(hidden_size)
        self.gamma = float(gamma)
        self.learning_rate = float(learning_rate)
        self.epsilon_start = float(epsilon_start)
        self.epsilon_end = float(epsilon_end)
        self.epsilon_decay_steps = int(epsilon_decay_steps)
        self.target_update_interval = int(target_update_interval)
        self.rng = np.random.default_rng(seed)
        self.online = MLPQNetwork(observation_size, action_count, hidden_size, self.rng)
        self.target = MLPQNetwork(observation_size, action_count, hidden_size, self.rng)
        self.target.copy_from(self.online)
        self.replay = ReplayBuffer(replay_capacity, observation_size, seed + 1)
        self.total_steps = 0
        self.gradient_steps = 0
        self._adam_m = {name: np.zeros_like(value) for name, value in self.online.parameters.items()}
        self._adam_v = {name: np.zeros_like(value) for name, value in self.online.parameters.items()}

    @property
    def epsilon(self) -> float:
        fraction = min(1.0, self.total_steps / max(1, self.epsilon_decay_steps))
        return self.epsilon_start + fraction * (self.epsilon_end - self.epsilon_start)

    def act(self, observation: np.ndarray, *, explore: bool = True) -> int:
        if explore and self.rng.random() < self.epsilon:
            return int(self.rng.integers(self.action_count))
        q_values = self.online.forward(np.asarray(observation, dtype=np.float32)[None, :])
        return int(np.argmax(q_values[0]))

    def remember(
        self,
        observation: np.ndarray,
        action: int,
        reward: float,
        next_observation: np.ndarray,
        done: bool,
    ) -> None:
        self.replay.add(observation, action, reward, next_observation, done)
        self.total_steps += 1

    def train_step(self, batch_size: int = 64) -> float | None:
        if self.replay.size < batch_size:
            return None

        observations, actions, rewards, next_observations, dones = self.replay.sample(batch_size)
        next_online = self.online.forward(next_observations)
        next_actions = np.argmax(next_online, axis=1)
        next_target = self.target.forward(next_observations)
        bootstrap = next_target[np.arange(batch_size), next_actions]
        targets = rewards + self.gamma * (1.0 - dones) * bootstrap

        q_values, cache = self.online.forward(observations, cache=True)
        chosen_q = q_values[np.arange(batch_size), actions]
        errors = chosen_q - targets
        absolute_errors = np.abs(errors)
        loss = np.where(absolute_errors <= 1.0, 0.5 * errors**2, absolute_errors - 0.5).mean()

        selected_gradient = np.where(absolute_errors <= 1.0, errors, np.sign(errors)) / batch_size
        q_gradient = np.zeros_like(q_values)
        q_gradient[np.arange(batch_size), actions] = selected_gradient
        observations, z1, a1, z2, a2 = cache
        p = self.online.parameters

        gradients: dict[str, np.ndarray] = {}
        gradients["w3"] = a2.T @ q_gradient
        gradients["b3"] = q_gradient.sum(axis=0)
        a2_gradient = q_gradient @ p["w3"].T
        z2_gradient = a2_gradient * (z2 > 0.0)
        gradients["w2"] = a1.T @ z2_gradient
        gradients["b2"] = z2_gradient.sum(axis=0)
        a1_gradient = z2_gradient @ p["w2"].T
        z1_gradient = a1_gradient * (z1 > 0.0)
        gradients["w1"] = observations.T @ z1_gradient
        gradients["b1"] = z1_gradient.sum(axis=0)

        global_norm = np.sqrt(sum(float(np.sum(gradient * gradient)) for gradient in gradients.values()))
        if global_norm > 10.0:
            scale = 10.0 / global_norm
            gradients = {name: gradient * scale for name, gradient in gradients.items()}

        self.gradient_steps += 1
        beta1, beta2 = 0.9, 0.999
        correction1 = 1.0 - beta1**self.gradient_steps
        correction2 = 1.0 - beta2**self.gradient_steps
        for name, parameter in p.items():
            gradient = gradients[name]
            self._adam_m[name] = beta1 * self._adam_m[name] + (1.0 - beta1) * gradient
            self._adam_v[name] = beta2 * self._adam_v[name] + (1.0 - beta2) * (gradient * gradient)
            parameter -= self.learning_rate * (self._adam_m[name] / correction1) / (
                np.sqrt(self._adam_v[name] / correction2) + 1e-8
            )

        if self.gradient_steps % self.target_update_interval == 0:
            self.target.copy_from(self.online)
        return float(loss)

    def save(self, path: str | Path) -> Path:
        checkpoint = Path(path)
        if checkpoint.suffix != ".npz":
            checkpoint = checkpoint.with_suffix(".npz")
        checkpoint.parent.mkdir(parents=True, exist_ok=True)
        values = {
            "observation_size": np.array(self.observation_size),
            "action_count": np.array(self.action_count),
            "hidden_size": np.array(self.hidden_size),
            "total_steps": np.array(self.total_steps),
            "gradient_steps": np.array(self.gradient_steps),
        }
        values.update({f"online_{name}": value for name, value in self.online.parameters.items()})
        values.update({f"target_{name}": value for name, value in self.target.parameters.items()})
        np.savez_compressed(checkpoint, **values)
        return checkpoint

    def load(self, path: str | Path) -> None:
        with np.load(Path(path)) as checkpoint:
            expected = (
                int(checkpoint["observation_size"]),
                int(checkpoint["action_count"]),
                int(checkpoint["hidden_size"]),
            )
            actual = (self.observation_size, self.action_count, self.hidden_size)
            if expected != actual:
                raise ValueError(f"checkpoint shape {expected} does not match agent shape {actual}")
            for name in self.online.parameters:
                self.online.parameters[name][...] = checkpoint[f"online_{name}"]
                self.target.parameters[name][...] = checkpoint[f"target_{name}"]
            self.total_steps = int(checkpoint["total_steps"])
            self.gradient_steps = int(checkpoint["gradient_steps"])
