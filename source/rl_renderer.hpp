#ifndef _RL_RENDERER_HPP_
#define _RL_RENDERER_HPP_

#include <string>

#include "rsdl.hpp"

class World;

class MarioRLRenderer
{
public:
    MarioRLRenderer(const std::string &assetRoot, int framesPerSecond);
    ~MarioRLRenderer();

    bool renderFrame(World *world, int action, int episodeSteps);

private:
    std::string assetPath(const std::string &path) const;
    void drawBackground(World *world);
    void drawObjects(World *world);
    void drawHud(World *world, int action, int episodeSteps);
    bool processEvents();

    std::string assetRoot;
    Window *window;
    int frameDelayMilliseconds;
    int cloudOffset;
    Uint32 lastCloudTick;
};

#endif
