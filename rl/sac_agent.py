"""Soft Actor-Critic for Mario's discrete action space.

Mario exposes nine discrete actions, so this is the discrete-action formulation
of SAC (Christodoulou, 2019) rather than the Gaussian-policy version: the actor
emits categorical logits, the twin critics score every action at once, and both
the target value and the actor loss take exact expectations over the policy
instead of sampling.

The agent reuses the machinery that already exists for DQN - prioritized replay,
n-step returns, soft target updates - and the same Q-network bodies, so it works
with the feature-vector observation and with ``observation_mode="pixels"``
frame stacks without a second set of encoders.
"""

from __future__ import annotations

import math
from itertools import chain
from pathlib import Path
from typing import Any

import numpy as np
import torch
from torch import nn
from torch.nn import functional as F

from .dqn_agent import CHECKPOINT_VERSION, DQNAgent, DuelingQNetwork, PrioritizedReplayBuffer
from .pixel_dqn_agent import NatureDuelingQNetwork


SAC_CHECKPOINT_VERSION = 1
UPDATE_METRIC_NAMES = DQNAgent.update_metric_names() + (
    "actor_loss",
    "alpha",
    "policy_entropy",
)


class SACAgent(DQNAgent):
    """Discrete Soft Actor-Critic with twin critics and a tuned temperature.

    ``observation`` is either the flat feature-vector size or a
    ``(channels, height, width)`` frame-stack shape; the shape selects the
    convolutional encoder and a ``uint8`` replay, exactly as for
    :class:`~rl.pixel_dqn_agent.PixelDQNAgent`.
    """

    def __init__(
        self,
        observation: int | tuple[int, int, int],
        action_count: int,
        *,
        hidden_size: int = 256,
        replay_capacity: int = 50_000,
        gamma: float = 0.99,
        learning_rate: float = 3e-4,
        alpha_learning_rate: float | None = None,
        tau: float = 0.005,
        n_step: int = 3,
        priority_alpha: float = 0.6,
        priority_beta_start: float = 0.4,
        priority_beta_steps: int = 250_000,
        target_entropy_ratio: float = 0.6,
        initial_alpha: float = 0.2,
        autotune_alpha: bool = True,
        target_update_interval: int = 1,
        max_grad_norm: float = 10.0,
        seed: int = 0,
        device: str | torch.device = "auto",
    ) -> None:
        if isinstance(observation, (tuple, list)):
            shape = tuple(int(value) for value in observation)
            if len(shape) != 3:
                raise ValueError("pixel observations must be shaped (channels, height, width)")
            # Set before super().__init__ so the build hooks below can read it.
            self.observation_shape = shape
            observation_size = int(np.prod(shape))
        else:
            self.observation_shape = None
            observation_size = int(observation)
        if not 0.0 < target_entropy_ratio <= 1.0:
            raise ValueError("target entropy ratio must be in (0, 1]")
        if initial_alpha <= 0.0:
            raise ValueError("initial alpha must be positive")

        self.target_entropy_ratio = float(target_entropy_ratio)
        self.initial_alpha = float(initial_alpha)
        self.autotune_alpha = bool(autotune_alpha)
        self.alpha_learning_rate = float(
            learning_rate if alpha_learning_rate is None else alpha_learning_rate
        )

        # SAC explores through the policy's own entropy, so the inherited
        # epsilon schedule is pinned to zero and never consulted.
        super().__init__(
            observation_size,
            action_count,
            hidden_size=hidden_size,
            replay_capacity=replay_capacity,
            gamma=gamma,
            learning_rate=learning_rate,
            epsilon_start=0.0,
            epsilon_end=0.0,
            epsilon_decay_steps=1,
            target_update_interval=target_update_interval,
            tau=tau,
            n_step=n_step,
            priority_alpha=priority_alpha,
            priority_beta_start=priority_beta_start,
            priority_beta_steps=priority_beta_steps,
            max_grad_norm=max_grad_norm,
            seed=seed,
            device=device,
        )

        # The base class built critic one (``online``) and its target; SAC needs
        # a second, independently initialized critic to take the pessimistic min.
        self.critic_one = self.online
        self.critic_one_target = self.target
        self.critic_two = self._build_network().to(self.device)
        self.critic_two_target = self._build_network().to(self.device)
        self.critic_two_target.load_state_dict(self.critic_two.state_dict())
        self.critic_two_target.requires_grad_(False)
        self.critic_two_target.eval()

        self.actor = self._build_network().to(self.device)
        self.critic_optimizer = torch.optim.AdamW(
            chain(self.critic_one.parameters(), self.critic_two.parameters()),
            lr=self.learning_rate,
            eps=1e-5,
            weight_decay=1e-5,
        )
        # Keep the inherited attribute pointing at a real optimizer so shared
        # tooling that inspects ``agent.optimizer`` keeps working.
        self.optimizer = self.critic_optimizer
        self.actor_optimizer = torch.optim.AdamW(
            self.actor.parameters(),
            lr=self.learning_rate,
            eps=1e-5,
            weight_decay=1e-5,
        )

        # A near-uniform policy over nine actions has entropy log(9); the ratio
        # picks how much of that the temperature should defend.
        self.target_entropy = self.target_entropy_ratio * math.log(self.action_count)
        self.log_alpha = torch.tensor(
            math.log(self.initial_alpha),
            dtype=torch.float32,
            device=self.device,
            requires_grad=self.autotune_alpha,
        )
        self.alpha_optimizer = (
            torch.optim.Adam([self.log_alpha], lr=self.alpha_learning_rate)
            if self.autotune_alpha
            else None
        )

    @classmethod
    def update_metric_names(cls) -> tuple[str, ...]:
        return UPDATE_METRIC_NAMES

    def _build_network(self) -> nn.Module:
        """Critic and actor bodies.

        Both produce one output per action: Q values for the critics, logits for
        the actor. The dueling parametrization adds a state-dependent constant to
        every action, which a softmax cancels, so it is harmless for the actor.
        """

        if self.observation_shape is not None:
            return NatureDuelingQNetwork(self.observation_shape, self.action_count, self.hidden_size)
        return DuelingQNetwork(self.observation_size, self.action_count, self.hidden_size)

    def _build_replay(self) -> PrioritizedReplayBuffer:
        if self.observation_shape is not None:
            return PrioritizedReplayBuffer(
                self.replay_capacity,
                self.observation_shape,
                alpha=self.priority_alpha,
                seed=self.seed + 1,
                dtype=np.uint8,
            )
        return super()._build_replay()

    @property
    def alpha(self) -> float:
        return float(self.log_alpha.detach().exp().item())

    def replay_bytes(self) -> int:
        """Resident size of the replay arrays, useful before a long run."""

        return int(self.replay.observations.nbytes + self.replay.next_observations.nbytes)

    def _policy(self, observations: torch.Tensor) -> tuple[torch.Tensor, torch.Tensor]:
        """Return action probabilities and their logs for a batch of states."""

        logits = self.actor(observations)
        log_probabilities = F.log_softmax(logits, dim=1)
        return log_probabilities.exp(), log_probabilities

    def act(self, observation: np.ndarray, *, explore: bool = True) -> int:
        observation_tensor = torch.as_tensor(
            np.asarray(observation, dtype=np.float32),
            device=self.device,
        ).unsqueeze(0)
        with torch.no_grad():
            logits = self.actor(observation_tensor)
            if not explore:
                return int(logits.argmax(dim=1).item())
            probabilities = F.softmax(logits, dim=1)
            return int(torch.multinomial(probabilities, num_samples=1).item())

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
        alpha = self.log_alpha.detach().exp()

        # Soft target: expectation over the next policy of the pessimistic Q,
        # minus the entropy bonus the policy is being paid for.
        with torch.no_grad():
            next_probabilities, next_log_probabilities = self._policy(next_observations_tensor)
            next_q_values = torch.min(
                self.critic_one_target(next_observations_tensor),
                self.critic_two_target(next_observations_tensor),
            )
            soft_state_value = (
                next_probabilities * (next_q_values - alpha * next_log_probabilities)
            ).sum(dim=1)
            targets = rewards_tensor + discounts_tensor * soft_state_value

        q_one = self.critic_one(observations_tensor).gather(1, actions_tensor).squeeze(1)
        q_two = self.critic_two(observations_tensor).gather(1, actions_tensor).squeeze(1)
        critic_losses = F.smooth_l1_loss(q_one, targets, reduction="none") + F.smooth_l1_loss(
            q_two, targets, reduction="none"
        )
        critic_loss = (weights_tensor * critic_losses).mean()

        self.critic_optimizer.zero_grad(set_to_none=True)
        critic_loss.backward()
        gradient_norm = nn.utils.clip_grad_norm_(
            list(chain(self.critic_one.parameters(), self.critic_two.parameters())),
            self.max_grad_norm,
        )
        self.critic_optimizer.step()

        # Actor: maximize the pessimistic Q plus the entropy of the policy.
        probabilities, log_probabilities = self._policy(observations_tensor)
        with torch.no_grad():
            actor_q_values = torch.min(
                self.critic_one(observations_tensor),
                self.critic_two(observations_tensor),
            )
        actor_loss = (probabilities * (alpha * log_probabilities - actor_q_values)).sum(dim=1).mean()

        self.actor_optimizer.zero_grad(set_to_none=True)
        actor_loss.backward()
        nn.utils.clip_grad_norm_(self.actor.parameters(), self.max_grad_norm)
        self.actor_optimizer.step()

        policy_entropy = -(probabilities * log_probabilities).sum(dim=1).mean()

        if self.alpha_optimizer is not None:
            # Push alpha up while the policy is more deterministic than the
            # entropy target, and down once it is more random than the target.
            alpha_loss = (
                probabilities.detach()
                * (-self.log_alpha * (log_probabilities.detach() + self.target_entropy))
            ).sum(dim=1).mean()
            self.alpha_optimizer.zero_grad(set_to_none=True)
            alpha_loss.backward()
            self.alpha_optimizer.step()

        self.gradient_steps += 1
        td_errors = targets - q_one
        priorities = td_errors.detach().abs().cpu().numpy() + 1e-5
        self.replay.update_priorities(indices, priorities)
        if self.gradient_steps % self.target_update_interval == 0:
            self._soft_update_target()

        return {
            "loss": float(critic_loss.detach().cpu()),
            "q_mean": float(q_one.detach().mean().cpu()),
            "target_q_mean": float(targets.mean().cpu()),
            "td_error_mean": float(td_errors.detach().abs().mean().cpu()),
            "gradient_norm": float(torch.as_tensor(gradient_norm).detach().cpu()),
            "priority_beta": self.priority_beta,
            "learning_rate": float(self.critic_optimizer.param_groups[0]["lr"]),
            "actor_loss": float(actor_loss.detach().cpu()),
            "alpha": self.alpha,
            "policy_entropy": float(policy_entropy.detach().cpu()),
        }

    @torch.no_grad()
    def _soft_update_target(self) -> None:
        for target_network, online_network in (
            (self.critic_one_target, self.critic_one),
            (self.critic_two_target, self.critic_two),
        ):
            for target_parameter, online_parameter in zip(
                target_network.parameters(), online_network.parameters(), strict=True
            ):
                target_parameter.lerp_(online_parameter, self.tau)

    def _config(self) -> dict[str, Any]:
        config = {
            "action_count": self.action_count,
            "hidden_size": self.hidden_size,
            "replay_capacity": self.replay_capacity,
            "gamma": self.gamma,
            "learning_rate": self.learning_rate,
            "alpha_learning_rate": self.alpha_learning_rate,
            "tau": self.tau,
            "n_step": self.n_step,
            "priority_alpha": self.priority_alpha,
            "priority_beta_start": self.priority_beta_start,
            "priority_beta_steps": self.priority_beta_steps,
            "target_entropy_ratio": self.target_entropy_ratio,
            "initial_alpha": self.initial_alpha,
            "autotune_alpha": self.autotune_alpha,
            "target_update_interval": self.target_update_interval,
            "max_grad_norm": self.max_grad_norm,
            "seed": self.seed,
        }
        if self.observation_shape is not None:
            config["observation_shape"] = list(self.observation_shape)
        else:
            config["observation_size"] = self.observation_size
        return config

    @classmethod
    def _config_to_kwargs(cls, config: dict[str, Any]) -> dict[str, Any]:
        kwargs = dict(config)
        shape = kwargs.pop("observation_shape", None)
        size = kwargs.pop("observation_size", None)
        kwargs["observation"] = tuple(shape) if shape is not None else int(size)
        return kwargs

    @staticmethod
    def _read_checkpoint(path: str | Path, device: torch.device) -> dict[str, Any]:
        checkpoint = Path(path)
        payload = torch.load(checkpoint, map_location=device, weights_only=True)
        if payload.get("algorithm") != "sac":
            raise ValueError(f"{checkpoint} is not a Soft Actor-Critic checkpoint")
        if payload.get("version") != SAC_CHECKPOINT_VERSION:
            raise ValueError(f"unsupported SAC checkpoint version: {payload.get('version')}")
        return payload

    def save(self, path: str | Path) -> Path:
        checkpoint = Path(path)
        if checkpoint.suffix not in {".pt", ".pth"}:
            checkpoint = checkpoint.with_suffix(".pt")
        checkpoint.parent.mkdir(parents=True, exist_ok=True)
        payload = {
            "algorithm": "sac",
            "version": SAC_CHECKPOINT_VERSION,
            "dqn_checkpoint_version": CHECKPOINT_VERSION,
            "config": self._config(),
            "actor_state_dict": self.actor.state_dict(),
            "critic_one_state_dict": self.critic_one.state_dict(),
            "critic_two_state_dict": self.critic_two.state_dict(),
            "critic_one_target_state_dict": self.critic_one_target.state_dict(),
            "critic_two_target_state_dict": self.critic_two_target.state_dict(),
            "critic_optimizer_state_dict": self.critic_optimizer.state_dict(),
            "actor_optimizer_state_dict": self.actor_optimizer.state_dict(),
            "alpha_optimizer_state_dict": (
                self.alpha_optimizer.state_dict() if self.alpha_optimizer is not None else {}
            ),
            "log_alpha": self.log_alpha.detach().cpu(),
            "total_steps": self.total_steps,
            "gradient_steps": self.gradient_steps,
            "numpy_rng_state": self.rng.bit_generator.state,
            "torch_rng_state": torch.get_rng_state(),
        }
        temporary = checkpoint.with_suffix(checkpoint.suffix + ".tmp")
        torch.save(payload, temporary)
        temporary.replace(checkpoint)
        return checkpoint

    def load(self, path: str | Path, *, load_optimizer: bool = True) -> None:
        payload = self._read_checkpoint(path, self.device)
        config = payload["config"]
        expected = (
            int(np.prod(config["observation_shape"]))
            if "observation_shape" in config
            else int(config["observation_size"]),
            int(config["action_count"]),
            int(config["hidden_size"]),
        )
        actual = (self.observation_size, self.action_count, self.hidden_size)
        if expected != actual:
            raise ValueError(f"checkpoint shape {expected} does not match agent shape {actual}")

        self.actor.load_state_dict(payload["actor_state_dict"])
        self.critic_one.load_state_dict(payload["critic_one_state_dict"])
        self.critic_two.load_state_dict(payload["critic_two_state_dict"])
        self.critic_one_target.load_state_dict(payload["critic_one_target_state_dict"])
        self.critic_two_target.load_state_dict(payload["critic_two_target_state_dict"])
        with torch.no_grad():
            self.log_alpha.copy_(payload["log_alpha"].to(self.device))
        if load_optimizer:
            self.critic_optimizer.load_state_dict(payload["critic_optimizer_state_dict"])
            self.actor_optimizer.load_state_dict(payload["actor_optimizer_state_dict"])
            if self.alpha_optimizer is not None and payload["alpha_optimizer_state_dict"]:
                self.alpha_optimizer.load_state_dict(payload["alpha_optimizer_state_dict"])
            for optimizer in (self.critic_optimizer, self.actor_optimizer):
                for state in optimizer.state.values():
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
    ) -> "SACAgent":
        resolved_device = cls._resolve_device(device)
        payload = cls._read_checkpoint(path, resolved_device)
        kwargs = cls._config_to_kwargs(payload["config"])
        # Evaluation does not need a large replay allocation.
        kwargs["replay_capacity"] = 1
        kwargs["device"] = resolved_device
        agent = cls(**kwargs)
        agent.load(path, load_optimizer=False)
        agent.actor.eval()
        return agent
