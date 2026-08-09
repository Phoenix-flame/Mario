#ifndef _CLOUD_LAYER_HPP_
#define _CLOUD_LAYER_HPP_

#include <string>

#include "rsdl.hpp"

// The parallax sky, shared by the game window and the RL playback window so the
// two never drift apart.
//
// A cloud is built from a left cap, a run of center pieces, and a right cap,
// each column two tiles tall. The number of center pieces is what makes a cloud
// small, medium, or large, the way the original games vary them.
struct CloudSprite
{
    int baseX;
    int y;
    int tileSize;
    int segments;       // center pieces between the caps: 1 small, 2 medium, 3 large
    int depth;          // larger means further away, so less camera parallax
    int driftPeriodMs;  // milliseconds per pixel of self-drift; larger is slower
};

// Draws every cloud for the current camera offset and clock.
// `backgroundX` is the camera's background offset and `assetPrefix` is prepended
// to the sprite paths for callers that do not run from the project root.
void drawCloudLayer(Window *window,
                    int backgroundX,
                    unsigned int milliseconds,
                    const std::string &assetPrefix = "");

#endif // !_CLOUD_LAYER_HPP_
