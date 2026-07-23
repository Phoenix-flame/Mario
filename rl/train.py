"""Train a stable PyTorch Rainbow-lite DQN agent on one Mario level."""

from __future__ import annotations

import argparse
from collections import deque
from datetime import datetime
from pathlib import Path

import numpy as np
import torch

from .dqn_agent import DQNAgent
from .mario_env import MarioEnv
from .training_logger import TrainingLogger


UPDATE_METRICS = (
    "loss",
    "q_mean",
    "target_q_mean",
    "td_error_mean",
    "gradient_norm",
    "priority_beta",
    "learning_rate",
)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--level", type=int, choices=(1, 2, 3), default=1)
    parser.add_argument("--episodes", type=int, default=1000)
    parser.add_argument("--max-steps", type=int, default=3000, help="Agent decisions per episode")
    parser.add_argument("--frame-skip", type=int, default=4, help="Engine frames per decision")
    parser.add_argument("--seed", type=int, default=7)
    parser.add_argument("--device", choices=("auto", "cpu", "cuda", "mps"), default="auto")
    parser.add_argument("--hidden-size", type=int, default=256)
    parser.add_argument("--batch-size", type=int, default=128)
    parser.add_argument("--replay-capacity", type=int, default=50_000)
    parser.add_argument("--learning-starts", type=int, default=5_000)
    parser.add_argument("--train-every", type=int, default=4)
    parser.add_argument("--learning-rate", type=float, default=1e-4)
    parser.add_argument("--gamma", type=float, default=0.99)
    parser.add_argument("--n-step", type=int, default=3)
    parser.add_argument("--tau", type=float, default=0.005)
    parser.add_argument("--priority-alpha", type=float, default=0.6)
    parser.add_argument("--priority-beta-start", type=float, default=0.4)
    parser.add_argument("--priority-beta-steps", type=int, default=250_000)
    parser.add_argument("--epsilon-start", type=float, default=1.0)
    parser.add_argument("--epsilon-end", type=float, default=0.05)
    parser.add_argument("--epsilon-decay-steps", type=int, default=250_000)
    parser.add_argument("--checkpoint", type=Path, default=Path("checkpoints/mario_dqn.pt"))
    parser.add_argument("--resume", type=Path)
    parser.add_argument("--save-every", type=int, default=25)
    parser.add_argument(
        "--eval-every",
        type=int,
        default=25,
        help="Run deterministic policy evaluation every N episodes; 0 disables it",
    )
    parser.add_argument("--eval-episodes", type=int, default=3)
    parser.add_argument(
        "--log-dir",
        type=Path,
        help="Output directory for CSV, TensorBoard, and PNG metrics",
    )
    parser.add_argument("--plot-every", type=int, default=10)
    parser.add_argument("--log-update-every", type=int, default=100)
    return parser


def evaluate_policy(
    environment: MarioEnv,
    agent: DQNAgent,
    episodes: int,
    seed: int,
) -> dict[str, float]:
    rewards: list[float] = []
    scores: list[int] = []
    progresses: list[float] = []
    wins: list[int] = []

    for evaluation_episode in range(episodes):
        observation, _ = environment.reset(seed=seed + evaluation_episode)
        episode_reward = 0.0
        while True:
            action = agent.act(observation, explore=False)
            observation, reward, terminated, truncated, info = environment.step(action)
            episode_reward += reward
            if terminated or truncated:
                break
        rewards.append(episode_reward)
        scores.append(int(info["score"]))
        progresses.append(float(info["progress"]))
        wins.append(int(bool(info["won"])))

    return {
        "reward": float(np.mean(rewards)),
        "score": float(np.mean(scores)),
        "progress": float(np.mean(progresses)),
        "win_rate": float(np.mean(wins)),
    }


def _mean(values: deque[float] | list[float]) -> float:
    return float(np.mean(values)) if values else float("nan")


def _validate_args(args: argparse.Namespace) -> None:
    positive_names = (
        "episodes",
        "max_steps",
        "frame_skip",
        "hidden_size",
        "batch_size",
        "replay_capacity",
        "learning_starts",
        "train_every",
        "save_every",
        "eval_episodes",
        "plot_every",
        "log_update_every",
    )
    for name in positive_names:
        if getattr(args, name) <= 0:
            raise ValueError(f"--{name.replace('_', '-')} must be positive")
    if args.eval_every < 0:
        raise ValueError("--eval-every cannot be negative")
    if args.batch_size > args.replay_capacity:
        raise ValueError("--batch-size cannot exceed --replay-capacity")


def main() -> None:
    args = build_parser().parse_args()
    _validate_args(args)
    np.random.seed(args.seed)
    torch.manual_seed(args.seed)

    timestamp = datetime.now().strftime("%Y%m%d-%H%M%S")
    log_dir = args.log_dir or Path("runs") / f"mario_level{args.level}_{timestamp}"
    recent_wins: deque[float] = deque(maxlen=50)
    recent_rewards: deque[float] = deque(maxlen=50)
    recent_scores: deque[float] = deque(maxlen=50)
    recent_progress: deque[float] = deque(maxlen=50)
    best_evaluation_rank = (-1.0, -1.0, -1.0)

    with MarioEnv(
        level=args.level,
        max_episode_steps=args.max_steps,
        frame_skip=args.frame_skip,
    ) as environment:
        agent = DQNAgent(
            environment.observation_size,
            environment.action_count,
            hidden_size=args.hidden_size,
            replay_capacity=args.replay_capacity,
            gamma=args.gamma,
            learning_rate=args.learning_rate,
            epsilon_start=args.epsilon_start,
            epsilon_end=args.epsilon_end,
            epsilon_decay_steps=args.epsilon_decay_steps,
            tau=args.tau,
            n_step=args.n_step,
            priority_alpha=args.priority_alpha,
            priority_beta_start=args.priority_beta_start,
            priority_beta_steps=args.priority_beta_steps,
            seed=args.seed,
            device=args.device,
        )
        if args.resume:
            agent.load(args.resume)
            print(f"resumed {args.resume} at environment step {agent.total_steps}")

        config = vars(args).copy()
        config.update(
            {
                "algorithm": "PyTorch dueling Double DQN + PER + n-step returns",
                "resolved_device": str(agent.device),
                "observation_size": environment.observation_size,
                "action_count": environment.action_count,
            }
        )

        print(f"training on {agent.device}; metrics will be written to {log_dir}")
        with TrainingLogger(log_dir, config) as logger:
            for episode in range(1, args.episodes + 1):
                observation, _ = environment.reset(seed=args.seed + episode)
                episode_reward = 0.0
                updates: list[dict[str, float]] = []
                info = {"score": 0, "progress": 0.0, "won": False, "episode_steps": 0}

                while True:
                    action = agent.act(observation, explore=True)
                    next_observation, reward, terminated, truncated, info = environment.step(action)
                    episode_end = terminated or truncated
                    agent.remember(
                        observation,
                        action,
                        reward,
                        next_observation,
                        terminated,
                        episode_end=episode_end,
                    )
                    observation = next_observation
                    episode_reward += reward

                    if (
                        agent.total_steps >= args.learning_starts
                        and agent.total_steps % args.train_every == 0
                    ):
                        update = agent.train_step(args.batch_size)
                        if update is not None:
                            updates.append(update)
                            if agent.gradient_steps % args.log_update_every == 0:
                                logger.log_update(agent.gradient_steps, update)
                    if episode_end:
                        break

                won = float(bool(info["won"]))
                recent_wins.append(won)
                recent_rewards.append(episode_reward)
                recent_scores.append(float(info["score"]))
                recent_progress.append(float(info["progress"]))
                update_means = {
                    name: _mean([update[name] for update in updates]) for name in UPDATE_METRICS
                }
                episode_metrics: dict[str, float | int] = {
                    "episode": episode,
                    "environment_steps": agent.total_steps,
                    "gradient_steps": agent.gradient_steps,
                    "episode_steps": int(info["episode_steps"]),
                    "reward": episode_reward,
                    "reward_50": _mean(recent_rewards),
                    "score": int(info["score"]),
                    "score_50": _mean(recent_scores),
                    "progress": float(info["progress"]),
                    "progress_50": _mean(recent_progress),
                    "won": int(won),
                    "win_rate_50": _mean(recent_wins),
                    "epsilon": agent.epsilon,
                    "replay_size": len(agent.replay),
                    **update_means,
                }
                logger.log_episode(episode_metrics)

                print(
                    f"episode={episode:04d} steps={info['episode_steps']:4d} "
                    f"reward={episode_reward:8.2f} score={info['score']:5d} "
                    f"progress={info['progress'] * 100:6.2f}% won={int(won)} "
                    f"epsilon={agent.epsilon:.3f} loss={update_means['loss']:.4f} "
                    f"reward_50={_mean(recent_rewards):.2f} "
                    f"win_rate_50={_mean(recent_wins):.2f}"
                )

                if args.eval_every and episode % args.eval_every == 0:
                    evaluation = evaluate_policy(
                        environment,
                        agent,
                        args.eval_episodes,
                        args.seed + 1_000_000 + episode * args.eval_episodes,
                    )
                    logger.log_evaluation(
                        {
                            "episode": episode,
                            "environment_steps": agent.total_steps,
                            **evaluation,
                        }
                    )
                    print(
                        f"evaluation episode={episode:04d} reward={evaluation['reward']:.2f} "
                        f"score={evaluation['score']:.1f} "
                        f"progress={evaluation['progress'] * 100:.2f}% "
                        f"win_rate={evaluation['win_rate']:.2f}"
                    )
                    evaluation_rank = (
                        evaluation["win_rate"],
                        evaluation["progress"],
                        evaluation["score"],
                    )
                    if evaluation_rank > best_evaluation_rank:
                        best_evaluation_rank = evaluation_rank
                        best_path = args.checkpoint.with_name(
                            args.checkpoint.stem + "_best" + args.checkpoint.suffix
                        )
                        agent.save(best_path)

                if episode % args.plot_every == 0:
                    logger.plot()
                if episode % args.save_every == 0:
                    agent.save(args.checkpoint)

            saved = agent.save(args.checkpoint)
            curves = logger.plot()
            print(f"saved final checkpoint to {saved}")
            if curves is not None:
                print(f"saved learning curves to {curves}")


if __name__ == "__main__":
    main()
