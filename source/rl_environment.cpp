#include "rl_environment.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

#include "objects/bullet.hpp"
#include "rl_renderer.hpp"

namespace
{
    const int SIMULATED_FRAME_MILLISECONDS = 33;
    const int SHOOT_COOLDOWN_MILLISECONDS = 250;
    const int GRID_LEFT_TILES = 3;
    const int GRID_TOP_TILES = 6;

    thread_local std::string lastError;

    float clampFloat(float value, float low, float high)
    {
        return std::max(low, std::min(value, high));
    }

    bool isTerrain(Type type)
    {
        return type == BLOCK || type == BRICK || type == GROUND || type == PIPE ||
               type == COIN_CONTAINER || type == FIRE_CONTAINER || type == HEALTH_CONTAINER;
    }

    bool isRewardBlock(Type type)
    {
        return type == COIN_CONTAINER || type == FIRE_CONTAINER || type == HEALTH_CONTAINER;
    }

    bool isEnemy(Type type)
    {
        return type == GOOMBA || type == KOOPA;
    }

    bool isPowerUp(Type type)
    {
        return type == G_MUSHROOM || type == G_FLOWER;
    }
}

MarioRLEnvironment::MarioRLEnvironment(const std::string &_assetRoot,
                                       int _level,
                                       int _maxEpisodeSteps,
                                       int _frameSkip,
                                       bool render,
                                       int renderFramesPerSecond) :
    assetRoot(_assetRoot),
    level(_level),
    maxEpisodeSteps(_maxEpisodeSteps),
    frameSkip(_frameSkip),
    episodeSteps(0),
    simulatedFrames(0),
    lastShotFrame(-1000000),
    lastScore(0),
    furthestX(0),
    finished(false),
    userQuit(false),
    lastAction(-1),
    world(nullptr),
    renderer(nullptr)
{
    if (level < 1 || level > 3)
    {
        throw std::invalid_argument("level must be between 1 and 3");
    }
    if (maxEpisodeSteps <= 0)
    {
        throw std::invalid_argument("maxEpisodeSteps must be positive");
    }
    if (frameSkip <= 0 || frameSkip > 20)
    {
        throw std::invalid_argument("frameSkip must be between 1 and 20");
    }
    if (render)
    {
        renderer = new MarioRLRenderer(assetRoot, renderFramesPerSecond);
    }
}

MarioRLEnvironment::~MarioRLEnvironment()
{
    delete renderer;
    delete world;
    Timer::useRealtime();
}

void MarioRLEnvironment::reset(float *observation)
{
    if (observation == nullptr)
    {
        throw std::invalid_argument("observation buffer cannot be null");
    }

    delete world;
    Timer::useSimulatedTime(0);
    world = new World(level, assetRoot);
    episodeSteps = 0;
    simulatedFrames = 0;
    lastShotFrame = -1000000;
    lastScore = 0;
    furthestX = world->getPlayer()->getPos().x;
    finished = false;
    userQuit = false;
    lastAction = -1;

    // Prime collision state so the initial observation reports Mario standing
    // on the floor instead of briefly falling.
    world->loop();
    fillObservation(observation);
}

void MarioRLEnvironment::updateCamera()
{
    Player *player = world->getPlayer();
    int screenX = player->getPos().x + world->camera->getPos().x;
    if (player->getSpeed() > 0 && screenX > 400)
    {
        world->camera->move(-5);
        world->camera->moveBackground(-1);
    }
    else if (player->getSpeed() < 0 && screenX < 200)
    {
        world->camera->move(5);
        world->camera->moveBackground(1);
    }
}

void MarioRLEnvironment::shoot()
{
    int elapsedMilliseconds = (simulatedFrames - lastShotFrame) * SIMULATED_FRAME_MILLISECONDS;
    Player *player = world->getPlayer();
    if (player->getLevel() != POWER || elapsedMilliseconds < SHOOT_COOLDOWN_MILLISECONDS)
    {
        return;
    }

    Point pos = player->getPos();
    Point size = player->getSize();
    Dir fireDirection = player->getDir() == LEFT ? LEFT : RIGHT;
    int bulletX = fireDirection == RIGHT ? pos.x + size.x : pos.x - 16;
    int bulletY = pos.y + size.y / 2 - 8;
    world->addGhost(new Bullet(bulletX, bulletY, fireDirection));
    player->shoot();
    lastShotFrame = simulatedFrames;
}

MarioRLStepResult MarioRLEnvironment::step(int action, float *observation)
{
    if (world == nullptr)
    {
        throw std::logic_error("reset must be called before step");
    }
    if (finished)
    {
        throw std::logic_error("reset must be called after an episode ends");
    }
    if (action < 0 || action >= ACTION_COUNT)
    {
        throw std::invalid_argument("action is outside the discrete action space");
    }
    if (observation == nullptr)
    {
        throw std::invalid_argument("observation buffer cannot be null");
    }

    const bool moveLeft = action == 1 || action == 4 || action == 7;
    const bool moveRight = action == 2 || action == 5 || action == 8;
    const bool jump = action == 3 || action == 4 || action == 5;
    const bool fire = action == 6 || action == 7 || action == 8;
    int direction = moveLeft ? -1 : (moveRight ? 1 : 0);
    lastAction = action;

    int previousFurthestX = furthestX;
    int previousScore = lastScore;
    bool won = false;
    bool dead = false;

    for (int frame = 0; frame < frameSkip; frame++)
    {
        Timer::advanceSimulatedTime(SIMULATED_FRAME_MILLISECONDS);
        simulatedFrames++;

        Player *player = world->getPlayer();
        player->update(direction);
        if (frame == 0 && jump)
        {
            player->startJump();
        }
        if (frame == 0 && fire)
        {
            shoot();
        }

        updateCamera();
        world->loop();
        furthestX = std::max(furthestX, player->getPos().x);
        won = player->getState() == WON;
        dead = player->getState() == DEAD;
        if (renderer != nullptr && !renderer->renderFrame(world, lastAction, episodeSteps + 1))
        {
            userQuit = true;
        }
        if (won || dead || userQuit)
        {
            break;
        }
    }

    episodeSteps++;
    lastScore = world->getGameState()->score;

    float reward = -0.01f;
    reward += static_cast<float>(furthestX - previousFurthestX) * 0.02f;
    reward += static_cast<float>(lastScore - previousScore) * 0.05f;
    if (won)
    {
        reward += 100.0f;
    }
    else if (dead)
    {
        reward -= 25.0f;
    }

    bool timedOut = !won && !dead && episodeSteps >= maxEpisodeSteps;
    bool truncated = !won && !dead && (timedOut || userQuit);
    if (timedOut)
    {
        reward -= 5.0f;
    }
    finished = won || dead || truncated;

    fillObservation(observation);

    MarioRLStepResult result;
    result.reward = reward;
    result.progress = clampFloat(static_cast<float>(furthestX) / std::max(1, world->getWinX()), 0.0f, 1.0f);
    result.score = lastScore;
    result.episodeSteps = episodeSteps;
    result.playerX = world->getPlayer()->getPos().x;
    result.terminated = won || dead;
    result.truncated = truncated;
    result.won = won;
    result.userQuit = userQuit;
    return result;
}

bool MarioRLEnvironment::render()
{
    if (renderer == nullptr)
    {
        throw std::logic_error("environment was not created with human rendering enabled");
    }
    if (world == nullptr)
    {
        throw std::logic_error("reset must be called before render");
    }
    if (!userQuit)
    {
        userQuit = !renderer->renderFrame(world, lastAction, episodeSteps);
    }
    return !userQuit;
}

void MarioRLEnvironment::setGridValue(float *observation,
                                      int channel,
                                      Object *object,
                                      float value) const
{
    Point playerPosition = world->getPlayer()->getPos();
    Point playerSize = world->getPlayer()->getSize();
    Point objectPosition = object->getPos();
    Point objectSize = object->getSize();

    int playerTileX = (playerPosition.x + playerSize.x / 2) / 24;
    int playerTileY = (playerPosition.y + playerSize.y / 2) / 24;
    int objectTileX = (objectPosition.x + objectSize.x / 2) / 24;
    int objectTileY = (objectPosition.y + objectSize.y / 2) / 24;
    int column = objectTileX - playerTileX + GRID_LEFT_TILES;
    int row = objectTileY - playerTileY + GRID_TOP_TILES;
    if (column < 0 || column >= GRID_COLUMNS || row < 0 || row >= GRID_ROWS)
    {
        return;
    }

    int channelOffset = FEATURE_COUNT + channel * GRID_COLUMNS * GRID_ROWS;
    int index = channelOffset + row * GRID_COLUMNS + column;
    observation[index] = std::max(observation[index], value);
}

void MarioRLEnvironment::fillObservation(float *observation) const
{
    std::fill(observation, observation + OBSERVATION_SIZE, 0.0f);
    Player *player = world->getPlayer();
    Point position = player->getPos();
    int mapWidth = std::max(1, world->getMapPixelWidth());
    int mapHeight = std::max(1, world->getMapPixelHeight());

    observation[0] = clampFloat(static_cast<float>(position.x) / mapWidth, -0.1f, 1.1f);
    observation[1] = clampFloat(static_cast<float>(position.y) / mapHeight, -0.5f, 1.5f);
    observation[2] = clampFloat(static_cast<float>(player->getSpeed()) / 5.0f, -1.0f, 1.0f);
    observation[3] = clampFloat(static_cast<float>(player->getVerticalSpeed()) / 10.0f, -1.0f, 1.0f);
    observation[4] = clampFloat(static_cast<float>(world->getWinX() - position.x) / mapWidth, 0.0f, 1.0f);
    observation[5] = clampFloat(static_cast<float>(world->getGameState()->score) / 5000.0f, 0.0f, 2.0f);
    observation[6] = player->getDir() == LEFT ? -1.0f : (player->getDir() == RIGHT ? 1.0f : 0.0f);
    observation[7] = player->isInvincible() ? 1.0f : 0.0f;

    int state = static_cast<int>(player->getState());
    if (state >= 0 && state < 7)
    {
        observation[8 + state] = 1.0f;
    }
    int playerLevel = static_cast<int>(player->getLevel());
    if (playerLevel >= 0 && playerLevel < 3)
    {
        observation[15 + playerLevel] = 1.0f;
    }

    float nearestEnemyDistance = 1000000.0f;
    float nearestPowerUpDistance = 1000000.0f;
    observation[18] = 1.0f;
    observation[20] = 1.0f;

    for (auto object : world->getObjects())
    {
        if (object == player || object->dead)
        {
            continue;
        }
        Type type = object->getType();
        if (isTerrain(type))
        {
            setGridValue(observation, 0, object, 1.0f);
        }
        if (isRewardBlock(type))
        {
            setGridValue(observation, 1, object, 1.0f);
        }
        else if (type == BRICK)
        {
            setGridValue(observation, 1, object, 0.25f);
        }
        if (isEnemy(type))
        {
            setGridValue(observation, 2, object, 1.0f);
            Point enemy = object->getPos();
            float dx = static_cast<float>(enemy.x - position.x);
            float dy = static_cast<float>(enemy.y - position.y);
            float distance = std::sqrt(dx * dx + dy * dy);
            if (distance < nearestEnemyDistance)
            {
                nearestEnemyDistance = distance;
                observation[18] = clampFloat(dx / 240.0f, -1.0f, 1.0f);
                observation[19] = clampFloat(dy / 120.0f, -1.0f, 1.0f);
            }
        }
    }

    for (auto object : world->getGhosts())
    {
        if (object->ghost_dead || !isPowerUp(object->getType()))
        {
            continue;
        }
        setGridValue(observation, 3, object, 1.0f);
        Point powerUp = object->getPos();
        float dx = static_cast<float>(powerUp.x - position.x);
        float dy = static_cast<float>(powerUp.y - position.y);
        float distance = std::sqrt(dx * dx + dy * dy);
        if (distance < nearestPowerUpDistance)
        {
            nearestPowerUpDistance = distance;
            observation[20] = clampFloat(dx / 240.0f, -1.0f, 1.0f);
            observation[21] = clampFloat(dy / 120.0f, -1.0f, 1.0f);
        }
    }
}

extern "C"
{
    void *mario_rl_create(const char *assetRoot, int level, int maxEpisodeSteps, int frameSkip)
    {
        try
        {
            lastError.clear();
            return new MarioRLEnvironment(assetRoot == nullptr ? "" : assetRoot,
                                          level,
                                          maxEpisodeSteps,
                                          frameSkip);
        }
        catch (const std::exception &error)
        {
            lastError = error.what();
            return nullptr;
        }
    }

    void *mario_rl_create_rendered(const char *assetRoot,
                                   int level,
                                   int maxEpisodeSteps,
                                   int frameSkip,
                                   int renderFramesPerSecond)
    {
        try
        {
            lastError.clear();
            return new MarioRLEnvironment(assetRoot == nullptr ? "" : assetRoot,
                                          level,
                                          maxEpisodeSteps,
                                          frameSkip,
                                          true,
                                          renderFramesPerSecond);
        }
        catch (const std::exception &error)
        {
            lastError = error.what();
            return nullptr;
        }
    }

    void mario_rl_destroy(void *environment)
    {
        delete static_cast<MarioRLEnvironment *>(environment);
    }

    int mario_rl_observation_size()
    {
        return MarioRLEnvironment::OBSERVATION_SIZE;
    }

    int mario_rl_action_count()
    {
        return MarioRLEnvironment::ACTION_COUNT;
    }

    int mario_rl_reset(void *environment, float *observation)
    {
        try
        {
            if (environment == nullptr)
            {
                throw std::invalid_argument("environment cannot be null");
            }
            lastError.clear();
            static_cast<MarioRLEnvironment *>(environment)->reset(observation);
            return 0;
        }
        catch (const std::exception &error)
        {
            lastError = error.what();
            return -1;
        }
    }

    int mario_rl_step(void *environment, int action, float *observation, MarioRLStepResult *result)
    {
        try
        {
            if (environment == nullptr || result == nullptr)
            {
                throw std::invalid_argument("environment and result cannot be null");
            }
            lastError.clear();
            *result = static_cast<MarioRLEnvironment *>(environment)->step(action, observation);
            return 0;
        }
        catch (const std::exception &error)
        {
            lastError = error.what();
            return -1;
        }
    }

    int mario_rl_render(void *environment)
    {
        try
        {
            if (environment == nullptr)
            {
                throw std::invalid_argument("environment cannot be null");
            }
            lastError.clear();
            return static_cast<MarioRLEnvironment *>(environment)->render() ? 0 : 1;
        }
        catch (const std::exception &error)
        {
            lastError = error.what();
            return -1;
        }
    }

    const char *mario_rl_last_error()
    {
        return lastError.c_str();
    }
}
