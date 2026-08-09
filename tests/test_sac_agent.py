"""Checks for the discrete Soft Actor-Critic agent."""

from __future__ import annotations

import math
import sys
import tempfile
import unittest
from pathlib import Path

import numpy as np
import torch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from rl.agents import build_agent, load_agent
from rl.dqn_agent import SPATIAL_OBSERVATION_SIZE
from rl.sac_agent import SACAgent


ACTIONS = 9


def _fill_replay(agent: SACAgent, transitions: int, shape, dtype, seed: int = 0) -> None:
    rng = np.random.default_rng(seed)
    for index in range(transitions):
        if dtype is np.uint8:
            observation = rng.integers(0, 256, shape, dtype=np.uint8)
            next_observation = rng.integers(0, 256, shape, dtype=np.uint8)
        else:
            observation = rng.random(shape, dtype=np.float32)
            next_observation = rng.random(shape, dtype=np.float32)
        agent.remember(
            observation,
            int(rng.integers(ACTIONS)),
            float(rng.normal()),
            next_observation,
            terminated=index % 17 == 0,
            episode_end=index % 17 == 0,
        )


class SACVectorAgentTest(unittest.TestCase):
    def setUp(self) -> None:
        self.agent = SACAgent(
            SPATIAL_OBSERVATION_SIZE,
            ACTIONS,
            hidden_size=32,
            replay_capacity=512,
            n_step=2,
            seed=3,
            device="cpu",
        )

    def test_target_entropy_follows_the_action_count(self) -> None:
        self.assertAlmostEqual(
            self.agent.target_entropy,
            0.6 * math.log(ACTIONS),
            places=6,
        )
        self.assertAlmostEqual(self.agent.alpha, 0.2, places=6)

    def test_critics_are_independent_networks(self) -> None:
        one = next(self.agent.critic_one.parameters())
        two = next(self.agent.critic_two.parameters())
        self.assertFalse(torch.equal(one, two))
        self.assertIs(self.agent.critic_one, self.agent.online)

    def test_greedy_action_is_deterministic_and_sampling_varies(self) -> None:
        observation = np.random.default_rng(1).random(SPATIAL_OBSERVATION_SIZE, dtype=np.float32)
        greedy = {self.agent.act(observation, explore=False) for _ in range(5)}
        self.assertEqual(len(greedy), 1)
        sampled = {self.agent.act(observation, explore=True) for _ in range(200)}
        self.assertGreater(len(sampled), 1, "an untrained policy should not be deterministic")
        self.assertTrue(sampled.issubset(set(range(ACTIONS))))

    def test_train_step_reports_finite_actor_critic_and_alpha_metrics(self) -> None:
        _fill_replay(self.agent, 300, (SPATIAL_OBSERVATION_SIZE,), np.float32)
        update = self.agent.train_step(32)
        self.assertIsNotNone(update)
        for name in SACAgent.update_metric_names():
            self.assertIn(name, update)
            self.assertTrue(np.isfinite(update[name]), f"{name} was not finite")
        self.assertGreater(update["alpha"], 0.0)
        self.assertGreaterEqual(update["policy_entropy"], 0.0)
        self.assertLessEqual(update["policy_entropy"], math.log(ACTIONS) + 1e-5)
        self.assertEqual(self.agent.gradient_steps, 1)

    def test_temperature_rises_when_the_policy_is_too_deterministic(self) -> None:
        # Drive the policy towards one action, then confirm alpha responds by
        # buying entropy back.
        _fill_replay(self.agent, 300, (SPATIAL_OBSERVATION_SIZE,), np.float32)
        with torch.no_grad():
            final_layer = self.agent.actor.advantage_head[-1]
            final_layer.bias.zero_()
            final_layer.bias[0] = 25.0
        before = self.agent.alpha
        for _ in range(12):
            self.agent.train_step(32)
        self.assertGreater(self.agent.alpha, before)

    def test_fixed_temperature_never_moves(self) -> None:
        agent = SACAgent(
            SPATIAL_OBSERVATION_SIZE,
            ACTIONS,
            hidden_size=32,
            replay_capacity=512,
            initial_alpha=0.35,
            autotune_alpha=False,
            seed=5,
            device="cpu",
        )
        _fill_replay(agent, 200, (SPATIAL_OBSERVATION_SIZE,), np.float32)
        for _ in range(5):
            agent.train_step(32)
        self.assertIsNone(agent.alpha_optimizer)
        self.assertAlmostEqual(agent.alpha, 0.35, places=6)

    def test_checkpoint_round_trip_preserves_the_policy(self) -> None:
        _fill_replay(self.agent, 200, (SPATIAL_OBSERVATION_SIZE,), np.float32)
        self.agent.train_step(32)
        observation = np.random.default_rng(7).random(SPATIAL_OBSERVATION_SIZE, dtype=np.float32)
        greedy = self.agent.act(observation, explore=False)

        with tempfile.TemporaryDirectory() as directory:
            path = self.agent.save(Path(directory) / "sac.pt")
            restored = load_agent(path, device="cpu")
            self.assertIsInstance(restored, SACAgent)
            self.assertEqual(restored.act(observation, explore=False), greedy)
            self.assertAlmostEqual(restored.alpha, self.agent.alpha, places=6)
            self.assertEqual(restored.total_steps, self.agent.total_steps)


class SACPixelAgentTest(unittest.TestCase):
    shape = (4, 84, 84)

    def test_pixel_observations_use_the_cnn_and_a_uint8_replay(self) -> None:
        agent = build_agent(
            "sac",
            self.shape,
            ACTIONS,
            hidden_size=32,
            replay_capacity=256,
            seed=11,
            device="cpu",
        )
        self.assertIsInstance(agent, SACAgent)
        self.assertEqual(agent.observation_shape, self.shape)
        self.assertEqual(agent.replay.observations.dtype, np.uint8)
        self.assertEqual(agent.observation_size, int(np.prod(self.shape)))

        _fill_replay(agent, 120, self.shape, np.uint8, seed=2)
        update = agent.train_step(16)
        self.assertIsNotNone(update)
        self.assertTrue(np.isfinite(update["loss"]))
        self.assertTrue(np.isfinite(update["actor_loss"]))

        observation = np.random.default_rng(4).integers(0, 256, self.shape, dtype=np.uint8)
        greedy = agent.act(observation, explore=False)
        self.assertIn(greedy, range(ACTIONS))

        with tempfile.TemporaryDirectory() as directory:
            path = agent.save(Path(directory) / "sac_pixels.pt")
            restored = load_agent(path, device="cpu")
            self.assertIsInstance(restored, SACAgent)
            self.assertEqual(restored.observation_shape, self.shape)
            self.assertEqual(restored.act(observation, explore=False), greedy)


if __name__ == "__main__":
    unittest.main()
