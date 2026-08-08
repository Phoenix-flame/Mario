"""CSV, TensorBoard, and PNG learning-curve logging for RL training."""

from __future__ import annotations

import csv
import json
from pathlib import Path
from typing import Any

import numpy as np
from torch.utils.tensorboard import SummaryWriter


EPISODE_FIELDS = (
    "episode",
    "environment_steps",
    "gradient_steps",
    "episode_steps",
    "reward",
    "reward_50",
    "score",
    "score_50",
    "progress",
    "progress_50",
    "won",
    "win_rate_50",
    "epsilon",
    "loss",
    "q_mean",
    "target_q_mean",
    "td_error_mean",
    "gradient_norm",
    "priority_beta",
    "learning_rate",
    "replay_size",
)

EVALUATION_FIELDS = (
    "episode",
    "environment_steps",
    "reward",
    "score",
    "progress",
    "win_rate",
)


class TrainingLogger:
    """Write machine-readable metrics and an always-current learning-curve image."""

    def __init__(self, log_dir: str | Path, config: dict[str, Any]) -> None:
        self.log_dir = Path(log_dir)
        self.log_dir.mkdir(parents=True, exist_ok=True)
        generated_files = ("config.json", "metrics.csv", "evaluations.csv", "learning_curves.png")
        existing = [name for name in generated_files if (self.log_dir / name).exists()]
        if existing:
            raise FileExistsError(
                f"log directory {self.log_dir} already contains training output "
                f"({', '.join(existing)}); choose a new --log-dir"
            )
        with (self.log_dir / "config.json").open("w", encoding="utf-8") as config_file:
            json.dump(config, config_file, indent=2, sort_keys=True, default=str)
            config_file.write("\n")

        self._episode_file = (self.log_dir / "metrics.csv").open(
            "w", newline="", encoding="utf-8"
        )
        self._episode_writer = csv.DictWriter(self._episode_file, fieldnames=EPISODE_FIELDS)
        self._episode_writer.writeheader()
        self._episode_file.flush()

        self._evaluation_file = (self.log_dir / "evaluations.csv").open(
            "w", newline="", encoding="utf-8"
        )
        self._evaluation_writer = csv.DictWriter(
            self._evaluation_file, fieldnames=EVALUATION_FIELDS
        )
        self._evaluation_writer.writeheader()
        self._evaluation_file.flush()

        self._writer = SummaryWriter(log_dir=str(self.log_dir / "tensorboard"))
        self._episodes: list[dict[str, float]] = []
        self._evaluations: list[dict[str, float]] = []

    def log_update(self, gradient_step: int, metrics: dict[str, float]) -> None:
        for name in (
            "loss",
            "q_mean",
            "target_q_mean",
            "td_error_mean",
            "gradient_norm",
            "priority_beta",
            "learning_rate",
        ):
            self._writer.add_scalar(f"updates/{name}", metrics[name], gradient_step)

    def log_episode(self, metrics: dict[str, float | int]) -> None:
        row = {field: metrics[field] for field in EPISODE_FIELDS}
        self._episode_writer.writerow(row)
        self._episode_file.flush()
        numeric_row = {name: float(value) for name, value in row.items()}
        self._episodes.append(numeric_row)

        episode = int(row["episode"])
        environment_steps = int(row["environment_steps"])
        for name in (
            "reward",
            "reward_50",
            "score",
            "score_50",
            "progress",
            "progress_50",
            "won",
            "win_rate_50",
            "epsilon",
            "loss",
            "q_mean",
            "target_q_mean",
            "td_error_mean",
            "gradient_norm",
            "priority_beta",
            "learning_rate",
            "replay_size",
            "episode_steps",
        ):
            value = float(row[name])
            if np.isfinite(value):
                self._writer.add_scalar(f"episodes/{name}", value, episode)
        self._writer.add_scalar("steps/episode", episode, environment_steps)
        self._writer.flush()

    def log_evaluation(self, metrics: dict[str, float | int]) -> None:
        row = {field: metrics[field] for field in EVALUATION_FIELDS}
        self._evaluation_writer.writerow(row)
        self._evaluation_file.flush()
        numeric_row = {name: float(value) for name, value in row.items()}
        self._evaluations.append(numeric_row)

        episode = int(row["episode"])
        for name in ("reward", "score", "progress", "win_rate"):
            self._writer.add_scalar(f"evaluation/{name}", float(row[name]), episode)
        self._writer.flush()

    def plot(self) -> Path | None:
        if not self._episodes:
            return None

        import matplotlib

        matplotlib.use("Agg")
        from matplotlib import pyplot as plt

        episodes = np.asarray([row["episode"] for row in self._episodes])
        evaluations = np.asarray([row["episode"] for row in self._evaluations])
        figure, axes = plt.subplots(2, 2, figsize=(14, 9), constrained_layout=True)

        reward_axis = axes[0, 0]
        reward_axis.plot(
            episodes,
            [row["reward"] for row in self._episodes],
            color="#8bb9e8",
            alpha=0.45,
            linewidth=1,
            label="episode reward",
        )
        reward_axis.plot(
            episodes,
            [row["reward_50"] for row in self._episodes],
            color="#1764ab",
            linewidth=2,
            label="reward (50-episode mean)",
        )
        if self._evaluations:
            reward_axis.plot(
                evaluations,
                [row["reward"] for row in self._evaluations],
                "o-",
                color="#e85d04",
                linewidth=1.5,
                markersize=4,
                label="greedy evaluation reward",
            )
        reward_axis.set(title="Reward", xlabel="Episode", ylabel="Return")
        reward_axis.legend(loc="best")
        reward_axis.grid(alpha=0.2)

        loss_axis = axes[0, 1]
        loss_axis.plot(
            episodes,
            [row["loss"] for row in self._episodes],
            color="#6a4c93",
            linewidth=1.5,
            label="Huber loss",
        )
        loss_axis.plot(
            episodes,
            [row["td_error_mean"] for row in self._episodes],
            color="#ff595e",
            linewidth=1.5,
            label="absolute TD error",
        )
        loss_axis.set(title="Optimization", xlabel="Episode", ylabel="Value")
        loss_axis.legend(loc="best")
        loss_axis.grid(alpha=0.2)

        progress_axis = axes[1, 0]
        progress_axis.plot(
            episodes,
            np.asarray([row["progress_50"] for row in self._episodes]) * 100.0,
            color="#2a9d8f",
            linewidth=2,
            label="progress (50-episode mean)",
        )
        progress_axis.plot(
            episodes,
            np.asarray([row["win_rate_50"] for row in self._episodes]) * 100.0,
            color="#f4a261",
            linewidth=2,
            label="win rate (last 50)",
        )
        if self._evaluations:
            progress_axis.plot(
                evaluations,
                np.asarray([row["progress"] for row in self._evaluations]) * 100.0,
                "o-",
                color="#006d77",
                linewidth=1.5,
                markersize=4,
                label="greedy evaluation progress",
            )
        progress_axis.set(title="Policy quality", xlabel="Episode", ylabel="Percent")
        progress_axis.set_ylim(-2.0, 102.0)
        progress_axis.legend(loc="best")
        progress_axis.grid(alpha=0.2)

        value_axis = axes[1, 1]
        value_axis.plot(
            episodes,
            [row["q_mean"] for row in self._episodes],
            color="#1982c4",
            linewidth=1.5,
            label="chosen Q",
        )
        value_axis.plot(
            episodes,
            [row["target_q_mean"] for row in self._episodes],
            color="#8ac926",
            linewidth=1.5,
            label="target Q",
        )
        value_axis.set(title="Value estimates and exploration", xlabel="Episode", ylabel="Q value")
        epsilon_axis = value_axis.twinx()
        epsilon_axis.plot(
            episodes,
            [row["epsilon"] for row in self._episodes],
            color="#ff924c",
            linestyle="--",
            linewidth=1.5,
            label="epsilon",
        )
        epsilon_axis.set_ylabel("Epsilon")
        epsilon_axis.set_ylim(0.0, 1.05)
        lines, labels = value_axis.get_legend_handles_labels()
        epsilon_lines, epsilon_labels = epsilon_axis.get_legend_handles_labels()
        value_axis.legend(lines + epsilon_lines, labels + epsilon_labels, loc="best")
        value_axis.grid(alpha=0.2)

        figure.suptitle("Mario RL learning curves", fontsize=15)
        output = self.log_dir / "learning_curves.png"
        temporary = self.log_dir / "learning_curves.tmp"
        figure.savefig(temporary, dpi=150, format="png")
        plt.close(figure)
        temporary.replace(output)
        return output

    def close(self) -> None:
        self._writer.flush()
        self._writer.close()
        self._episode_file.close()
        self._evaluation_file.close()

    def __enter__(self) -> "TrainingLogger":
        return self

    def __exit__(self, *_: object) -> None:
        self.close()
