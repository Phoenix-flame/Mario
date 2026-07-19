"""Train a score-seeking Double DQN agent on one Mario level."""

from __future__ import annotations

import argparse
from collections import deque
from pathlib import Path

import numpy as np

from .dqn_agent import DQNAgent
from .mario_env import MarioEnv


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--level", type=int, choices=(1, 2, 3), default=1)
    parser.add_argument("--episodes", type=int, default=1000)
    parser.add_argument("--max-steps", type=int, default=3000, help="Agent decisions per episode")
    parser.add_argument("--frame-skip", type=int, default=4, help="Engine frames per decision")
    parser.add_argument("--seed", type=int, default=7)
    parser.add_argument("--hidden-size", type=int, default=128)
    parser.add_argument("--batch-size", type=int, default=64)
    parser.add_argument("--learning-starts", type=int, default=1000)
    parser.add_argument("--train-every", type=int, default=4)
    parser.add_argument("--checkpoint", type=Path, default=Path("checkpoints/mario_dqn.npz"))
    parser.add_argument("--resume", type=Path)
    parser.add_argument("--save-every", type=int, default=25)
    return parser


def main() -> None:
    args = build_parser().parse_args()
    np.random.seed(args.seed)
    recent_wins: deque[int] = deque(maxlen=50)
    recent_rewards: deque[float] = deque(maxlen=50)
    best_rank = (-1, -1, -1.0)

    with MarioEnv(
        level=args.level,
        max_episode_steps=args.max_steps,
        frame_skip=args.frame_skip,
    ) as environment:
        agent = DQNAgent(
            environment.observation_size,
            environment.action_count,
            hidden_size=args.hidden_size,
            seed=args.seed,
        )
        if args.resume:
            agent.load(args.resume)
            print(f"resumed {args.resume} at environment step {agent.total_steps}")

        for episode in range(1, args.episodes + 1):
            observation, _ = environment.reset(seed=args.seed + episode)
            episode_reward = 0.0
            losses: list[float] = []
            info = {"score": 0, "progress": 0.0, "won": False, "episode_steps": 0}

            while True:
                action = agent.act(observation, explore=True)
                next_observation, reward, terminated, truncated, info = environment.step(action)
                done = terminated or truncated
                agent.remember(observation, action, reward, next_observation, done)
                observation = next_observation
                episode_reward += reward

                if agent.total_steps >= args.learning_starts and agent.total_steps % args.train_every == 0:
                    loss = agent.train_step(args.batch_size)
                    if loss is not None:
                        losses.append(loss)
                if done:
                    break

            won = int(bool(info["won"]))
            recent_wins.append(won)
            recent_rewards.append(episode_reward)
            rank = (won, int(info["score"]), float(info["progress"]))
            if rank > best_rank:
                best_rank = rank
                best_path = args.checkpoint.with_name(args.checkpoint.stem + "_best.npz")
                agent.save(best_path)

            mean_loss = float(np.mean(losses)) if losses else float("nan")
            print(
                f"episode={episode:04d} steps={info['episode_steps']:4d} "
                f"reward={episode_reward:8.2f} score={info['score']:5d} "
                f"progress={info['progress'] * 100:6.2f}% won={won} "
                f"epsilon={agent.epsilon:.3f} loss={mean_loss:.4f} "
                f"reward_50={sum(recent_rewards) / len(recent_rewards):.2f} "
                f"win_rate_50={sum(recent_wins) / len(recent_wins):.2f}"
            )

            if episode % args.save_every == 0:
                agent.save(args.checkpoint)

        saved = agent.save(args.checkpoint)
        print(f"saved final checkpoint to {saved}")


if __name__ == "__main__":
    main()
