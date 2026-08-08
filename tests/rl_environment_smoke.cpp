#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

#include "rl_environment.hpp"

#ifndef MARIO_SOURCE_DIR
#define MARIO_SOURCE_DIR "."
#endif

int main()
{
    const int observationSize = mario_rl_observation_size();
    assert(observationSize == MarioRLEnvironment::OBSERVATION_SIZE);
    assert(mario_rl_action_count() == 9);

    void *environment = mario_rl_create(MARIO_SOURCE_DIR, 1, 100, 4);
    if (environment == nullptr)
    {
        std::cerr << mario_rl_last_error() << std::endl;
        return 1;
    }

    std::vector<float> observation(observationSize);
    assert(mario_rl_reset(environment, observation.data()) == 0);
    int initialX = 0;
    MarioRLStepResult result = {};
    for (int i = 0; i < 10; i++)
    {
        assert(mario_rl_step(environment, 2, observation.data(), &result) == 0);
        assert(std::isfinite(result.reward));
        assert(result.userQuit == 0);
        initialX = result.playerX;
        if (result.terminated || result.truncated)
        {
            break;
        }
    }
    assert(initialX > 24);

    assert(mario_rl_reset(environment, observation.data()) == 0);
    assert(observation[0] >= 0.0f && observation[0] < 0.1f);

    const int actions[] = {2, 5, 5, 2, 0, 4, 2};
    for (int action : actions)
    {
        assert(mario_rl_step(environment, action, observation.data(), &result) == 0);
    }
    std::vector<float> firstRun = observation;
    MarioRLStepResult firstResult = result;

    assert(mario_rl_reset(environment, observation.data()) == 0);
    for (int action : actions)
    {
        assert(mario_rl_step(environment, action, observation.data(), &result) == 0);
    }
    for (int i = 0; i < observationSize; i++)
    {
        assert(std::abs(firstRun[i] - observation[i]) < 0.000001f);
    }
    assert(firstResult.playerX == result.playerX);
    assert(firstResult.score == result.score);
    assert(std::abs(firstResult.reward - result.reward) < 0.000001f);

    // Image observations: the visible view rasterized for convolutional agents.
    const int frameWidth = mario_rl_frame_width();
    const int frameHeight = mario_rl_frame_height();
    assert(frameWidth == MarioRLEnvironment::FRAME_WIDTH);
    assert(frameHeight == MarioRLEnvironment::FRAME_HEIGHT);

    std::vector<unsigned char> frame(frameWidth * frameHeight);
    assert(mario_rl_frame(environment, frame.data()) == 0);
    int litPixels = 0;
    int brightest = 0;
    for (unsigned int i = 0; i < frame.size(); i++)
    {
        litPixels += frame[i] > 0 ? 1 : 0;
        brightest = std::max(brightest, static_cast<int>(frame[i]));
    }
    // Mario plus the floor are always on screen, and Mario is the brightest ink.
    assert(litPixels > 0);
    assert(brightest == 255);

    mario_rl_destroy(environment);
    return 0;
}
