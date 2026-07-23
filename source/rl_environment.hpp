#ifndef _RL_ENVIRONMENT_HPP_
#define _RL_ENVIRONMENT_HPP_

#include <string>
#include "world.hpp"

class MarioRLRenderer;

struct MarioRLStepResult
{
    float reward;
    float progress;
    int score;
    int episodeSteps;
    int playerX;
    int terminated;
    int truncated;
    int won;
    int userQuit;
};

class MarioRLEnvironment
{
public:
    static const int ACTION_COUNT = 9;
    static const int GRID_COLUMNS = 13;
    static const int GRID_ROWS = 9;
    static const int GRID_CHANNELS = 4;
    static const int FEATURE_COUNT = 22;
    static const int OBSERVATION_SIZE = FEATURE_COUNT + GRID_COLUMNS * GRID_ROWS * GRID_CHANNELS;

    MarioRLEnvironment(const std::string &assetRoot,
                       int level,
                       int maxEpisodeSteps,
                       int frameSkip,
                       bool render = false,
                       int renderFramesPerSecond = 30);
    ~MarioRLEnvironment();

    void reset(float *observation);
    MarioRLStepResult step(int action, float *observation);
    bool render();

private:
    void fillObservation(float *observation) const;
    void updateCamera();
    void shoot();
    void setGridValue(float *observation, int channel, Object *object, float value) const;

    std::string assetRoot;
    int level;
    int maxEpisodeSteps;
    int frameSkip;
    int episodeSteps;
    int simulatedFrames;
    int lastShotFrame;
    int lastScore;
    int furthestX;
    bool finished;
    bool userQuit;
    int lastAction;
    World *world;
    MarioRLRenderer *renderer;
};

extern "C"
{
    void *mario_rl_create(const char *assetRoot, int level, int maxEpisodeSteps, int frameSkip);
    void *mario_rl_create_rendered(const char *assetRoot,
                                   int level,
                                   int maxEpisodeSteps,
                                   int frameSkip,
                                   int renderFramesPerSecond);
    void mario_rl_destroy(void *environment);
    int mario_rl_observation_size();
    int mario_rl_action_count();
    int mario_rl_reset(void *environment, float *observation);
    int mario_rl_step(void *environment, int action, float *observation, MarioRLStepResult *result);
    int mario_rl_render(void *environment);
    const char *mario_rl_last_error();
}

#endif
