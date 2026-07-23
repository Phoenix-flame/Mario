"""Evaluate a trained PyTorch Mario DQN checkpoint without exploration."""

from __future__ import annotations

import argparse
from pathlib import Path

from .dqn_agent import DQNAgent
from .mario_env import ACTION_NAMES, MarioEnv


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("checkpoint", type=Path)
    parser.add_argument("--level", type=int, choices=(1, 2, 3), default=1)
    parser.add_argument("--episodes", type=int, default=10)
    parser.add_argument("--max-steps", type=int, default=3000)
    parser.add_argument("--frame-skip", type=int, default=4)
    parser.add_argument("--device", choices=("auto", "cpu", "cuda", "mps"), default="auto")
    args = parser.parse_args()

    with MarioEnv(args.level, args.max_steps, args.frame_skip) as environment:
        agent = DQNAgent.from_checkpoint(args.checkpoint, device=args.device)
        checkpoint_shape = (agent.observation_size, agent.action_count)
        environment_shape = (environment.observation_size, environment.action_count)
        if checkpoint_shape != environment_shape:
            raise ValueError(
                f"checkpoint environment shape {checkpoint_shape} does not match "
                f"the native environment {environment_shape}"
            )
        wins = 0
        scores: list[int] = []

        for episode in range(1, args.episodes + 1):
            observation, _ = environment.reset()
            action_counts = [0] * environment.action_count
            while True:
                action = agent.act(observation, explore=False)
                action_counts[action] += 1
                observation, _, terminated, truncated, info = environment.step(action)
                if terminated or truncated:
                    break
            wins += int(info["won"])
            scores.append(info["score"])
            most_used = ACTION_NAMES[max(range(len(action_counts)), key=action_counts.__getitem__)]
            print(
                f"episode={episode:03d} score={info['score']:5d} "
                f"progress={info['progress'] * 100:6.2f}% won={int(info['won'])} "
                f"most_used_action={most_used}"
            )

        print(f"wins={wins}/{args.episodes} best_score={max(scores)} mean_score={sum(scores) / len(scores):.1f}")


if __name__ == "__main__":
    main()
