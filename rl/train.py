"""Train a stable PyTorch Rainbow-lite DQN agent on one Mario level."""

from __future__ import annotations

import argparse
from collections import deque
from contextlib import nullcontext
from datetime import datetime
from pathlib import Path
from typing import Any

import numpy as np
import torch

try:  # tqdm drives the live progress bars; plain printing is used without it.
    from tqdm.auto import tqdm
except ImportError:  # pragma: no cover - exercised only on minimal installs
    tqdm = None

from .agents import ALGORITHMS, build_agent
from .dqn_agent import DQNAgent
from .mario_env import MarioEnv
from .training_logger import EPISODE_FIELDS, TrainingLogger


# Frame stacks are far larger than feature vectors, so the pixel agent gets a
# smaller default replay unless the user asks for a specific size.
PIXEL_REPLAY_CAPACITY = 20_000

# SAC's actor, twin critics, and temperature prefer a livelier step size than
# the DQN default.
SAC_LEARNING_RATE = 3e-4


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--level", type=int, choices=(1, 2, 3), default=1)
    parser.add_argument(
        "--algorithm",
        choices=ALGORITHMS,
        default="dqn",
        help="Dueling Double DQN, or discrete Soft Actor-Critic",
    )
    parser.add_argument(
        "--observation",
        choices=("vector", "pixels"),
        default="vector",
        help="Hand-designed feature vector, or stacked game frames for the CNN agent",
    )
    parser.add_argument(
        "--frame-stack",
        type=int,
        default=4,
        help="Frames per pixel observation; ignored for --observation vector",
    )
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
    parser.add_argument("--epsilon-start", type=float, default=1.0, help="DQN exploration only")
    parser.add_argument("--epsilon-end", type=float, default=0.05, help="DQN exploration only")
    parser.add_argument("--epsilon-decay-steps", type=int, default=250_000, help="DQN only")
    parser.add_argument(
        "--target-entropy-ratio",
        type=float,
        default=0.6,
        help="SAC only: entropy target as a fraction of log(action count)",
    )
    parser.add_argument(
        "--initial-alpha",
        type=float,
        default=0.2,
        help="SAC only: starting entropy temperature",
    )
    parser.add_argument(
        "--alpha-learning-rate",
        type=float,
        help="SAC only: temperature step size; defaults to --learning-rate",
    )
    parser.add_argument(
        "--fixed-alpha",
        action="store_true",
        help="SAC only: keep the temperature at --initial-alpha instead of tuning it",
    )
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
    parser.add_argument(
        "--progress",
        choices=("bar", "plain", "none"),
        default="bar",
        help="tqdm progress bars, one metrics line per episode, or quiet",
    )
    parser.add_argument(
        "--require-cuda",
        action="store_true",
        help="Abort before training if the agent did not land on a CUDA device",
    )
    return parser


def verify_device(agent: DQNAgent, require_cuda: bool) -> dict[str, Any]:
    """Report the device the network really sits on before training starts.

    A silent fallback to CPU is the usual reason a run is unexpectedly slow, so
    this runs one forward pass and checks where the result was produced instead
    of trusting ``torch.cuda.is_available()`` alone.
    """

    report: dict[str, Any] = {
        "torch_version": torch.__version__,
        "cuda_build": torch.version.cuda,
        "cuda_available": bool(torch.cuda.is_available()),
        "device_count": int(torch.cuda.device_count()) if torch.cuda.is_available() else 0,
        "device": str(agent.device),
    }

    probe = torch.zeros((1, agent.observation_size), dtype=torch.float32, device=agent.device)
    with torch.no_grad():
        output = agent.online(probe)
    report["forward_device"] = str(output.device)

    print(f"torch {torch.__version__} (CUDA build {torch.version.cuda or 'none'})")
    print(f"cuda available: {report['cuda_available']}, visible devices: {report['device_count']}")

    if agent.device.type == "cuda":
        index = agent.device.index if agent.device.index is not None else torch.cuda.current_device()
        properties = torch.cuda.get_device_properties(index)
        report["gpu_name"] = properties.name
        report["gpu_memory_gib"] = properties.total_memory / (1024**3)
        print(
            f"gpu: {properties.name}, {report['gpu_memory_gib']:.2f} GiB, "
            f"compute capability {properties.major}.{properties.minor}"
        )
        print(
            f"forward pass ran on {output.device}; "
            f"{torch.cuda.memory_allocated(index) / (1024**2):.1f} MiB allocated"
        )
        if output.device.type != "cuda":
            raise SystemExit("the network reports a CUDA device but its output came back on CPU")
    else:
        message = f"training will run on {agent.device}"
        if torch.cuda.is_available():
            message += " even though CUDA is available; pass --device cuda to force the GPU"
        else:
            message += "; no CUDA device is visible to this PyTorch build"
        print(message + " - expect substantially slower training")
        if require_cuda:
            raise SystemExit("--require-cuda was set but the agent is not on a CUDA device")

    return report


def exploration_postfix(agent: DQNAgent, update_means: dict[str, float]) -> dict[str, str]:
    """Show whichever exploration knob the algorithm actually uses."""

    if "alpha" in update_means:
        return {
            "alpha": f"{update_means['alpha']:.3f}",
            "entropy": f"{update_means['policy_entropy']:.2f}",
        }
    return {"eps": f"{agent.epsilon:.3f}"}


def _emit(message: str, bar: "tqdm | None" = None) -> None:
    """Write above a live tqdm bar so the bar stays pinned to the bottom line."""

    if bar is not None:
        bar.write(message)
    else:
        print(message)


def _make_bar(enabled: bool, **kwargs: Any) -> "tqdm | None":
    return tqdm(dynamic_ncols=True, **kwargs) if enabled else None


def evaluate_policy(
    environment: MarioEnv,
    agent: DQNAgent,
    episodes: int,
    seed: int,
    show_progress: bool = False,
) -> dict[str, float]:
    rewards: list[float] = []
    scores: list[int] = []
    progresses: list[float] = []
    wins: list[int] = []

    evaluation_range = range(episodes)
    if show_progress:
        evaluation_range = tqdm(
            evaluation_range,
            desc="evaluating",
            unit="ep",
            leave=False,
            dynamic_ncols=True,
        )

    for evaluation_episode in evaluation_range:
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

    show_bars = args.progress == "bar"
    if show_bars and tqdm is None:
        print("tqdm is not installed; falling back to --progress plain")
        show_bars = False

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
        observation_mode=args.observation,
        frame_stack=args.frame_stack,
    ) as environment:
        uses_pixels = args.observation == "pixels"
        uses_sac = args.algorithm == "sac"
        defaults = build_parser()
        observation_argument = (
            environment.observation_shape if uses_pixels else environment.observation_size
        )
        replay_capacity = args.replay_capacity
        if uses_pixels and replay_capacity == defaults.get_default("replay_capacity"):
            replay_capacity = PIXEL_REPLAY_CAPACITY
            print(
                f"pixel observations: using a {replay_capacity} transition replay "
                "by default; override with --replay-capacity"
            )
        learning_rate = args.learning_rate
        if uses_sac and learning_rate == defaults.get_default("learning_rate"):
            learning_rate = SAC_LEARNING_RATE
            print(f"soft actor-critic: using a {learning_rate} learning rate by default")

        algorithm_kwargs: dict[str, Any] = (
            {
                "target_entropy_ratio": args.target_entropy_ratio,
                "initial_alpha": args.initial_alpha,
                "alpha_learning_rate": args.alpha_learning_rate,
                "autotune_alpha": not args.fixed_alpha,
            }
            if uses_sac
            else {
                "epsilon_start": args.epsilon_start,
                "epsilon_end": args.epsilon_end,
                "epsilon_decay_steps": args.epsilon_decay_steps,
            }
        )
        agent = build_agent(
            args.algorithm,
            observation_argument,
            environment.action_count,
            hidden_size=args.hidden_size,
            replay_capacity=replay_capacity,
            gamma=args.gamma,
            learning_rate=learning_rate,
            tau=args.tau,
            n_step=args.n_step,
            priority_alpha=args.priority_alpha,
            priority_beta_start=args.priority_beta_start,
            priority_beta_steps=args.priority_beta_steps,
            seed=args.seed,
            device=args.device,
            **algorithm_kwargs,
        )
        metric_names = agent.update_metric_names()
        extra_metric_fields = tuple(
            name for name in metric_names if name not in EPISODE_FIELDS
        )
        if args.resume:
            agent.load(args.resume)
            print(f"resumed {args.resume} at environment step {agent.total_steps}")

        encoder = "Nature CNN over stacked frames" if uses_pixels else "tile-grid encoder"
        learner = (
            "discrete Soft Actor-Critic with twin critics and tuned temperature"
            if uses_sac
            else "dueling Double DQN"
        )
        config = vars(args).copy()
        config.update(
            {
                "algorithm": f"PyTorch {learner} + PER + n-step returns ({encoder})",
                "learning_rate": learning_rate,
                "resolved_device": str(agent.device),
                "observation_size": environment.observation_size,
                "observation_shape": list(environment.observation_shape),
                "replay_capacity": replay_capacity,
                "action_count": environment.action_count,
            }
        )

        device_report = verify_device(agent, args.require_cuda)
        config["device_report"] = device_report
        if uses_pixels:
            print(
                f"observation: {environment.observation_shape} uint8 frames, "
                f"replay footprint {agent.replay_bytes() / (1024 ** 3):.2f} GiB"
            )
        print(f"training on {agent.device}; metrics will be written to {log_dir}")
        episode_bar = _make_bar(
            show_bars,
            total=args.episodes,
            desc=f"level {args.level}",
            unit="ep",
        )
        bar_context = episode_bar if episode_bar is not None else nullcontext()
        with (
            TrainingLogger(log_dir, config, extra_episode_fields=extra_metric_fields) as logger,
            bar_context,
        ):
            for episode in range(1, args.episodes + 1):
                observation, _ = environment.reset(seed=args.seed + episode)
                episode_reward = 0.0
                updates: list[dict[str, float]] = []
                info = {"score": 0, "progress": 0.0, "won": False, "episode_steps": 0}
                step_bar = _make_bar(
                    show_bars,
                    total=args.max_steps,
                    desc=f"episode {episode}",
                    unit="step",
                    leave=False,
                )

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
                    if step_bar is not None:
                        step_bar.update(1)

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

                if step_bar is not None:
                    step_bar.close()

                won = float(bool(info["won"]))
                recent_wins.append(won)
                recent_rewards.append(episode_reward)
                recent_scores.append(float(info["score"]))
                recent_progress.append(float(info["progress"]))
                update_means = {
                    name: _mean([update[name] for update in updates]) for name in metric_names
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

                if episode_bar is not None:
                    episode_bar.set_postfix(
                        {
                            "reward": f"{episode_reward:.1f}",
                            "reward_50": f"{_mean(recent_rewards):.1f}",
                            "score": int(info["score"]),
                            "progress": f"{info['progress'] * 100:.1f}%",
                            "win_50": f"{_mean(recent_wins):.2f}",
                            **exploration_postfix(agent, update_means),
                            "loss": f"{update_means['loss']:.4f}",
                        },
                        refresh=False,
                    )
                    episode_bar.update(1)
                elif args.progress == "plain":
                    exploration = " ".join(
                        f"{name}={value}"
                        for name, value in exploration_postfix(agent, update_means).items()
                    )
                    print(
                        f"episode={episode:04d} steps={info['episode_steps']:4d} "
                        f"reward={episode_reward:8.2f} score={info['score']:5d} "
                        f"progress={info['progress'] * 100:6.2f}% won={int(won)} "
                        f"{exploration} loss={update_means['loss']:.4f} "
                        f"reward_50={_mean(recent_rewards):.2f} "
                        f"win_rate_50={_mean(recent_wins):.2f}"
                    )

                if args.eval_every and episode % args.eval_every == 0:
                    evaluation = evaluate_policy(
                        environment,
                        agent,
                        args.eval_episodes,
                        args.seed + 1_000_000 + episode * args.eval_episodes,
                        show_progress=show_bars,
                    )
                    logger.log_evaluation(
                        {
                            "episode": episode,
                            "environment_steps": agent.total_steps,
                            **evaluation,
                        }
                    )
                    _emit(
                        f"evaluation episode={episode:04d} reward={evaluation['reward']:.2f} "
                        f"score={evaluation['score']:.1f} "
                        f"progress={evaluation['progress'] * 100:.2f}% "
                        f"win_rate={evaluation['win_rate']:.2f}",
                        episode_bar,
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
            _emit(f"saved final checkpoint to {saved}", episode_bar)
            if curves is not None:
                _emit(f"saved learning curves to {curves}", episode_bar)


if __name__ == "__main__":
    main()
