import csv
import tempfile
import unittest
from pathlib import Path

from rl.training_logger import EPISODE_FIELDS, TrainingLogger


class TrainingLoggerTest(unittest.TestCase):
    def test_writes_csv_tensorboard_and_plot(self):
        with tempfile.TemporaryDirectory() as directory:
            log_dir = Path(directory)
            with TrainingLogger(log_dir, {"algorithm": "test"}) as logger:
                metrics = {field: 1.0 for field in EPISODE_FIELDS}
                metrics["episode"] = 1
                metrics["environment_steps"] = 10
                metrics["gradient_steps"] = 2
                logger.log_episode(metrics)
                logger.log_evaluation(
                    {
                        "episode": 1,
                        "environment_steps": 10,
                        "reward": 2.0,
                        "score": 100.0,
                        "progress": 0.5,
                        "win_rate": 0.0,
                    }
                )
                output = logger.plot()

            self.assertEqual(output, log_dir / "learning_curves.png")
            self.assertTrue((log_dir / "learning_curves.png").is_file())
            self.assertTrue((log_dir / "config.json").is_file())
            self.assertTrue(any((log_dir / "tensorboard").iterdir()))
            with (log_dir / "metrics.csv").open(newline="", encoding="utf-8") as metrics_file:
                rows = list(csv.DictReader(metrics_file))
            self.assertEqual(len(rows), 1)
            self.assertEqual(rows[0]["episode"], "1")


if __name__ == "__main__":
    unittest.main()
