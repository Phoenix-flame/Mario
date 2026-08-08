import tempfile
import unittest
from pathlib import Path

import numpy as np
import torch

from rl.dqn_agent import DQNAgent


class DQNAgentTest(unittest.TestCase):
    def test_training_and_checkpoint_round_trip(self):
        agent = DQNAgent(
            6,
            3,
            hidden_size=16,
            replay_capacity=64,
            n_step=1,
            tau=0.5,
            seed=2,
            device="cpu",
        )
        rng = np.random.default_rng(3)
        for index in range(32):
            observation = rng.normal(size=6).astype(np.float32)
            next_observation = rng.normal(size=6).astype(np.float32)
            terminated = index % 7 == 0
            agent.remember(
                observation,
                index % 3,
                float(index % 5),
                next_observation,
                terminated,
            )

        before = next(agent.online.parameters()).detach().clone()
        metrics = agent.train_step(batch_size=8)
        self.assertIsNotNone(metrics)
        assert metrics is not None
        self.assertTrue(np.isfinite(metrics["loss"]))
        self.assertTrue(np.isfinite(metrics["td_error_mean"]))
        self.assertFalse(torch.equal(before, next(agent.online.parameters()).detach()))

        with tempfile.TemporaryDirectory() as directory:
            checkpoint = agent.save(Path(directory) / "agent")
            self.assertEqual(checkpoint.suffix, ".pt")
            restored = DQNAgent(
                6,
                3,
                hidden_size=16,
                replay_capacity=64,
                n_step=1,
                seed=4,
                device="cpu",
            )
            restored.load(checkpoint)
            for name, parameter in agent.online.state_dict().items():
                torch.testing.assert_close(parameter, restored.online.state_dict()[name])
            self.assertEqual(agent.total_steps, restored.total_steps)
            self.assertEqual(agent.gradient_steps, restored.gradient_steps)

            inference_agent = DQNAgent.from_checkpoint(checkpoint, device="cpu")
            self.assertEqual(len(inference_agent.replay), 0)
            self.assertEqual(
                agent.act(np.zeros(6, dtype=np.float32), explore=False),
                inference_agent.act(np.zeros(6, dtype=np.float32), explore=False),
            )

    def test_n_step_returns_bootstrap_after_time_limit(self):
        agent = DQNAgent(
            2,
            2,
            hidden_size=16,
            replay_capacity=16,
            gamma=0.5,
            n_step=3,
            seed=1,
            device="cpu",
        )
        first = np.array([0.0, 0.0], dtype=np.float32)
        second = np.array([1.0, 0.0], dtype=np.float32)
        final = np.array([2.0, 0.0], dtype=np.float32)
        agent.remember(first, 0, 1.0, second, False)
        agent.remember(second, 1, 2.0, final, False, episode_end=True)

        self.assertEqual(len(agent.replay), 2)
        self.assertAlmostEqual(float(agent.replay.rewards[0]), 2.0)
        self.assertAlmostEqual(float(agent.replay.discounts[0]), 0.25)
        self.assertAlmostEqual(float(agent.replay.rewards[1]), 2.0)
        self.assertAlmostEqual(float(agent.replay.discounts[1]), 0.5)

    def test_true_terminal_disables_bootstrap(self):
        agent = DQNAgent(
            2,
            2,
            hidden_size=16,
            replay_capacity=16,
            gamma=0.5,
            n_step=3,
            seed=1,
            device="cpu",
        )
        observation = np.zeros(2, dtype=np.float32)
        agent.remember(observation, 0, -1.0, observation, True)
        self.assertEqual(len(agent.replay), 1)
        self.assertEqual(float(agent.replay.discounts[0]), 0.0)


if __name__ == "__main__":
    unittest.main()
