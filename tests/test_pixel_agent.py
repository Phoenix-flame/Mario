"""Checks for the frame observation mode and the convolutional DQN agent."""

from __future__ import annotations

import math
import sys
import tempfile
import unittest
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from rl.mario_env import MarioEnv
from rl.agents import load_agent
from rl.pixel_dqn_agent import NatureDuelingQNetwork, PixelDQNAgent, is_pixel_checkpoint


class PixelObservationTest(unittest.TestCase):
    def test_frames_have_the_declared_shape_and_content(self) -> None:
        with MarioEnv(level=1, max_episode_steps=40, frame_skip=4, observation_mode="pixels") as env:
            self.assertEqual(env.observation_shape, (4, env.frame_height, env.frame_width))

            observation, _ = env.reset()
            self.assertEqual(observation.shape, env.observation_shape)
            self.assertEqual(observation.dtype, np.uint8)
            # Mario and the ground are always on screen, so a frame is never blank.
            self.assertGreater(int(observation[-1].max()), 0)
            # reset() repeats one frame, so the stack starts free of stale pixels.
            for index in range(1, env.frame_stack):
                np.testing.assert_array_equal(observation[0], observation[index])

            for _ in range(6):
                observation, reward, terminated, truncated, _ = env.step(2)
                self.assertEqual(observation.shape, env.observation_shape)
                self.assertTrue(np.isfinite(reward))
                if terminated or truncated:
                    break

            # Running right moves the world, so the stack holds distinct frames.
            self.assertTrue((observation[0] != observation[-1]).any())

    def test_agent_observation_size_matches_the_pixel_observation(self) -> None:
        # observation_size stays the native feature-vector size even in pixel
        # mode, so callers must size an agent from observation_shape. play.py
        # compared the wrong one and rejected every pixel checkpoint.
        with MarioEnv(level=1, max_episode_steps=40, frame_skip=4, observation_mode="pixels") as env:
            agent = PixelDQNAgent(
                env.observation_shape,
                env.action_count,
                hidden_size=32,
                replay_capacity=8,
                seed=0,
                device="cpu",
            )
            self.assertEqual(agent.observation_size, math.prod(env.observation_shape))
            self.assertNotEqual(agent.observation_size, env.observation_size)

    def test_frames_are_deterministic_for_the_same_actions(self) -> None:
        actions = [2, 5, 2, 2, 5, 2]
        runs = []
        with MarioEnv(level=1, max_episode_steps=40, frame_skip=4, observation_mode="pixels") as env:
            for _ in range(2):
                env.reset(seed=3)
                for action in actions:
                    observation, *_ = env.step(action)
                runs.append(observation.copy())
        np.testing.assert_array_equal(runs[0], runs[1])

    def test_vector_mode_is_unchanged(self) -> None:
        with MarioEnv(level=1, max_episode_steps=20, frame_skip=4) as env:
            observation, _ = env.reset()
            self.assertEqual(observation.shape, (env.observation_size,))
            self.assertEqual(observation.dtype, np.float32)


class PixelAgentTest(unittest.TestCase):
    shape = (4, 84, 84)

    def _agent(self, **kwargs) -> PixelDQNAgent:
        defaults = dict(hidden_size=64, replay_capacity=256, seed=0, device="cpu")
        defaults.update(kwargs)
        return PixelDQNAgent(self.shape, 9, **defaults)

    def test_network_maps_frames_to_action_values(self) -> None:
        import torch

        network = NatureDuelingQNetwork(self.shape, 9, 64)
        frames = torch.randint(0, 256, (3, *self.shape), dtype=torch.uint8)
        values = network(frames)
        self.assertEqual(values.shape, (3, 9))
        self.assertTrue(torch.isfinite(values).all())

    def test_replay_keeps_frames_as_bytes(self) -> None:
        agent = self._agent()
        self.assertEqual(agent.replay.observations.dtype, np.uint8)
        expected = 2 * 256 * int(np.prod(self.shape))
        self.assertEqual(agent.replay_bytes(), expected)

    def test_train_step_runs_and_checkpoints_round_trip(self) -> None:
        agent = self._agent()
        rng = np.random.default_rng(0)
        for _ in range(80):
            observation = rng.integers(0, 256, self.shape, dtype=np.uint8)
            next_observation = rng.integers(0, 256, self.shape, dtype=np.uint8)
            agent.remember(observation, int(rng.integers(9)), float(rng.random()), next_observation, False)

        update = agent.train_step(16)
        self.assertIsNotNone(update)
        self.assertTrue(np.isfinite(update["loss"]))

        observation = rng.integers(0, 256, self.shape, dtype=np.uint8)
        greedy = agent.act(observation, explore=False)
        self.assertIn(greedy, range(9))

        with tempfile.TemporaryDirectory() as directory:
            path = agent.save(Path(directory) / "pixel.pt")
            self.assertTrue(is_pixel_checkpoint(path))
            restored = load_agent(path, device="cpu")
            self.assertIsInstance(restored, PixelDQNAgent)
            self.assertEqual(restored.observation_shape, self.shape)
            self.assertEqual(restored.act(observation, explore=False), greedy)


if __name__ == "__main__":
    unittest.main()
