import tempfile
import unittest
from pathlib import Path

import numpy as np

from rl.dqn_agent import DQNAgent


class DQNAgentTest(unittest.TestCase):
    def test_training_and_checkpoint_round_trip(self):
        agent = DQNAgent(6, 3, hidden_size=8, replay_capacity=64, target_update_interval=2, seed=2)
        rng = np.random.default_rng(3)
        for index in range(32):
            observation = rng.normal(size=6).astype(np.float32)
            next_observation = rng.normal(size=6).astype(np.float32)
            agent.remember(observation, index % 3, float(index % 5), next_observation, index % 7 == 0)

        before = agent.online.parameters["w1"].copy()
        loss = agent.train_step(batch_size=8)
        self.assertIsNotNone(loss)
        self.assertTrue(np.isfinite(loss))
        self.assertFalse(np.array_equal(before, agent.online.parameters["w1"]))

        with tempfile.TemporaryDirectory() as directory:
            checkpoint = agent.save(Path(directory) / "agent.npz")
            restored = DQNAgent(6, 3, hidden_size=8, replay_capacity=64, seed=4)
            restored.load(checkpoint)
            for name in agent.online.parameters:
                np.testing.assert_allclose(agent.online.parameters[name], restored.online.parameters[name])
            self.assertEqual(agent.total_steps, restored.total_steps)


if __name__ == "__main__":
    unittest.main()
