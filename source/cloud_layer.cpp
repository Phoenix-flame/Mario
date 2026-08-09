#include "cloud_layer.hpp"

namespace
{
    const RGB CLOUD_TRANSPARENT(255, 0, 255);
    const int WRAP_PERIOD = 760;
    const int SCREEN_WIDTH = 640;
    const char *CLOUD_ROOT = "assets/sprites/objects/cloud/";

    // Three sizes at three heights, on two parallax planes, each drifting at its
    // own rate so the sky never reads as one rigid strip sliding past.
    const CloudSprite CLOUDS[] = {
        {30, 64, 22, 3, 2, 45},   // large, near
        {150, 128, 15, 1, 3, 72}, // small, far, low
        {265, 48, 18, 2, 2, 50},  // medium, near, high
        {380, 96, 24, 3, 2, 42},  // large, near, fastest
        {470, 150, 14, 1, 3, 80}, // small, far, lowest and slowest
        {560, 70, 20, 2, 2, 47},  // medium, near
        {650, 112, 16, 1, 3, 66}, // small, far
        {720, 40, 17, 2, 3, 58},  // medium, far, highest
    };

    const int CLOUD_COUNT = sizeof(CLOUDS) / sizeof(CLOUDS[0]);

    void drawTile(Window *window, const std::string &path, int x, int y, int size)
    {
        window->draw_img_with_color_key(
            path,
            CLOUD_TRANSPARENT,
            Rectangle(Point(x, y), Point(x + size, y + size)),
            NULL_RECT,
            0,
            false
        );
    }

    // Draws one column of the cloud: `topTile` over `bottomTile`.
    void drawColumn(Window *window,
                    const std::string &root,
                    const char *topTile,
                    const char *bottomTile,
                    int x,
                    int y,
                    int size)
    {
        drawTile(window, root + topTile, x, y, size);
        drawTile(window, root + bottomTile, x, y + size, size);
    }

    void drawCloud(Window *window, const std::string &root, const CloudSprite &cloud, int x)
    {
        const int size = cloud.tileSize;
        drawColumn(window, root, "cloud_left_top.bmp", "cloud_left_bot.bmp", x, cloud.y, size);
        for (int segment = 0; segment < cloud.segments; segment++)
        {
            drawColumn(
                window,
                root,
                "cloud_center_top.bmp",
                "cloud_center_bot.bmp",
                x + size * (segment + 1),
                cloud.y,
                size
            );
        }
        drawColumn(
            window,
            root,
            "cloud_right_top.bmp",
            "cloud_right_bot.bmp",
            x + size * (cloud.segments + 1),
            cloud.y,
            size
        );
    }
}

void drawCloudLayer(Window *window,
                    int backgroundX,
                    unsigned int milliseconds,
                    const std::string &assetPrefix)
{
    if (window == nullptr)
    {
        return;
    }

    std::string root = assetPrefix;
    if (!root.empty() && root[root.size() - 1] != '/')
    {
        root += "/";
    }
    root += CLOUD_ROOT;

    for (int index = 0; index < CLOUD_COUNT; index++)
    {
        const CloudSprite &cloud = CLOUDS[index];
        const int width = (cloud.segments + 2) * cloud.tileSize;
        const int drift = static_cast<int>(milliseconds / cloud.driftPeriodMs);

        int x = (cloud.baseX + backgroundX / cloud.depth - drift) % WRAP_PERIOD;
        while (x < -width)
        {
            x += WRAP_PERIOD;
        }
        while (x > SCREEN_WIDTH)
        {
            x -= WRAP_PERIOD;
        }
        if (x + width < 0 || x > SCREEN_WIDTH)
        {
            continue;
        }

        drawCloud(window, root, cloud, x);
    }
}
