"""Show a trained PyTorch Mario policy playing in the native SDL window."""

from __future__ import annotations

import argparse
import time
from pathlib import Path

from .dqn_agent import DQNAgent
from .mario_env import MarioEnv


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("checkpoint", type=Path, help="Best .pt checkpoint to play")
    parser.add_argument("--level", type=int, choices=(1, 2, 3), default=1)
    parser.add_argument("--episodes", type=int, default=1)
    parser.add_argument("--max-steps", type=int, default=3000)
    parser.add_argument("--frame-skip", type=int, default=4)
    parser.add_argument("--fps", type=int, default=30, help="Rendered engine frames per second")
    parser.add_argument(
        "--end-delay",
        type=float,
        default=2.0,
        help="Seconds to display the terminal frame before reset/exit",
    )
    parser.add_argument("--device", choices=("auto", "cpu", "cuda", "mps"), default="auto")
    return parser


def _hold_terminal_frame(environment: MarioEnv, seconds: float) -> bool:
    deadline = time.monotonic() + seconds
    while time.monotonic() < deadline:
        if not environment.render():
            return False
    return True


def main() -> None:
    args = build_parser().parse_args()
    if args.episodes <= 0:
        raise ValueError("--episodes must be positive")
    if args.max_steps <= 0:
        raise ValueError("--max-steps must be positive")
    if args.frame_skip <= 0:
        raise ValueError("--frame-skip must be positive")
    if not 1 <= args.fps <= 240:
        raise ValueError("--fps must be between 1 and 240")
    if args.end_delay < 0.0:
        raise ValueError("--end-delay cannot be negative")

    agent = DQNAgent.from_checkpoint(args.checkpoint, device=args.device)
    print(f"loaded {args.checkpoint} at step {agent.total_steps}; inference device={agent.device}")
    print("close the window or press Q/Esc to stop")

    with MarioEnv(
        level=args.level,
        max_episode_steps=args.max_steps,
        frame_skip=args.frame_skip,
        render_mode="human",
        render_fps=args.fps,
    ) as environment:
        checkpoint_shape = (agent.observation_size, agent.action_count)
        environment_shape = (environment.observation_size, environment.action_count)
        if checkpoint_shape != environment_shape:
            raise ValueError(
                f"checkpoint environment shape {checkpoint_shape} does not match "
                f"the native environment {environment_shape}"
            )

        for episode in range(1, args.episodes + 1):
            observation, _ = environment.reset()
            if not environment.render():
                return
            episode_reward = 0.0

            while True:
                action = agent.act(observation, explore=False)
                observation, reward, terminated, truncated, info = environment.step(action)
                episode_reward += reward
                if info["user_quit"]:
                    return
                if terminated or truncated:
                    break

            print(
                f"episode={episode:03d} reward={episode_reward:.2f} "
                f"score={info['score']} progress={info['progress'] * 100:.2f}% "
                f"won={int(info['won'])}"
            )
            if args.end_delay > 0.0 and not _hold_terminal_frame(environment, args.end_delay):
                return


if __name__ == "__main__":
    main()
