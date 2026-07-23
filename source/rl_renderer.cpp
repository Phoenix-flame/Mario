#include "rl_renderer.hpp"

#include <algorithm>
#include <stdexcept>

#include "objects/coin.hpp"
#include "objects/text.hpp"
#include "world.hpp"

namespace
{
    const int WINDOW_WIDTH = 640;
    const int WINDOW_HEIGHT = 480;
    const char *ACTION_NAMES[] = {
        "idle",
        "left",
        "right",
        "jump",
        "left+jump",
        "right+jump",
        "shoot",
        "left+shoot",
        "right+shoot",
    };
}

MarioRLRenderer::MarioRLRenderer(const std::string &_assetRoot, int framesPerSecond) :
    assetRoot(_assetRoot),
    window(nullptr),
    frameDelayMilliseconds(0),
    cloudOffset(0),
    lastCloudTick(0)
{
    if (framesPerSecond <= 0 || framesPerSecond > 240)
    {
        throw std::invalid_argument("render FPS must be between 1 and 240");
    }
    while (!assetRoot.empty() && assetRoot[assetRoot.size() - 1] == '/')
    {
        assetRoot.erase(assetRoot.size() - 1);
    }
    frameDelayMilliseconds = std::max(1, 1000 / framesPerSecond);
    window = new Window(WINDOW_WIDTH, WINDOW_HEIGHT, "Mario - RL Policy", false);
    lastCloudTick = SDL_GetTicks();
}

MarioRLRenderer::~MarioRLRenderer()
{
    delete window;
}

std::string MarioRLRenderer::assetPath(const std::string &path) const
{
    if (path.empty() || assetRoot.empty() || path[0] == '/')
    {
        return path;
    }
    std::string relative = path;
    if (relative.size() >= 2 && relative[0] == '.' && relative[1] == '/')
    {
        relative.erase(0, 2);
    }
    return assetRoot + "/" + relative;
}

bool MarioRLRenderer::processEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0)
    {
        if (event.type == SDL_QUIT)
        {
            return false;
        }
        if (event.type == SDL_KEYDOWN &&
            (event.key.keysym.sym == SDLK_q || event.key.keysym.sym == SDLK_ESCAPE))
        {
            return false;
        }
    }
    return true;
}

void MarioRLRenderer::drawBackground(World *world)
{
    const RGB skyBlue(92, 148, 252);
    const RGB transparentPurple(255, 0, 255);
    window->fill_rect(
        Rectangle(Point(0, 0), Point(WINDOW_WIDTH, WINDOW_HEIGHT)),
        skyBlue
    );

    Uint32 currentTick = SDL_GetTicks();
    if (currentTick - lastCloudTick > 45)
    {
        cloudOffset = (cloudOffset + 1) % 760;
        lastCloudTick = currentTick;
    }

    const int period = 760;
    const int bases[] = {80, 230, 390, 560, 720};
    const int ys[] = {70, 120, 55, 145, 95};
    const int tileSizes[] = {20, 16, 22, 18, 16};
    const std::string cloudRoot = "assets/sprites/objects/cloud/";
    const char *tiles[] = {
        "cloud_left_top.bmp",
        "cloud_center_top.bmp",
        "cloud_right_top.bmp",
        "cloud_left_bot.bmp",
        "cloud_center_bot.bmp",
        "cloud_right_bot.bmp",
    };

    for (int cloud = 0; cloud < 5; cloud++)
    {
        int x = (bases[cloud] + world->camera->getPosBackground().x / 2 - cloudOffset) % period;
        while (x < -100)
        {
            x += period;
        }
        while (x > 680)
        {
            x -= period;
        }

        int size = tileSizes[cloud];
        for (int tile = 0; tile < 6; tile++)
        {
            int column = tile % 3;
            int row = tile / 3;
            Point topLeft(x + column * size, ys[cloud] + row * size);
            window->draw_img_with_color_key(
                assetPath(cloudRoot + tiles[tile]),
                transparentPurple,
                Rectangle(topLeft, topLeft + Point(size, size)),
                NULL_RECT,
                0,
                false
            );
        }
    }
}

void MarioRLRenderer::drawObjects(World *world)
{
    Point offset = world->camera->getPos();
    const std::string font = assetPath("assets/Roboto-Regular.ttf");

    for (auto ghost : world->getGhosts())
    {
        if (ghost->getType() == G_TEXT)
        {
            Text *text = static_cast<Text *>(ghost);
            window->show_text(text->text, text->getPos() + offset, WHITE, font, 12);
            continue;
        }
        bool flipped = ghost->getType() == G_COIN && static_cast<Coin *>(ghost)->flipped;
        window->draw_img(
            assetPath(ghost->getImage()),
            Rectangle(ghost->getPos() + offset, ghost->getPos() + ghost->getSize() + offset),
            NULL_RECT,
            0,
            flipped
        );
    }

    for (auto object : world->getObjects())
    {
        if (object->getType() == PLAYER)
        {
            continue;
        }
        window->draw_img(
            assetPath(object->getImage()),
            Rectangle(object->getPos() + offset, object->getPos() + object->getSize() + offset),
            NULL_RECT,
            0,
            false
        );
    }

    Player *player = world->getPlayer();
    if (player->shouldDraw())
    {
        window->draw_img(
            assetPath(player->getImage()),
            Rectangle(player->getPos() + offset, player->getPos() + player->getSize() + offset),
            NULL_RECT,
            0,
            false
        );
    }
}

void MarioRLRenderer::drawHud(World *world, int action, int episodeSteps)
{
    const std::string font = assetPath("assets/Roboto-Regular.ttf");
    window->fill_rect(Rectangle(Point(0, 0), Point(WINDOW_WIDTH, 52)), BLACK);
    window->show_text(
        "Level " + std::to_string(world->getLevel()),
        Point(10, 6),
        WHITE,
        font,
        17
    );
    window->show_text(
        "Score " + std::to_string(world->getGameState()->score),
        Point(115, 6),
        WHITE,
        font,
        17
    );
    window->show_text(
        "Step " + std::to_string(episodeSteps),
        Point(270, 6),
        WHITE,
        font,
        17
    );
    std::string actionName = action >= 0 && action < 9 ? ACTION_NAMES[action] : "waiting";
    window->show_text("AI: " + actionName, Point(10, 29), CYAN, font, 15);
    window->show_text("Q / Esc: close", Point(500, 29), YELLOW, font, 13);

    State state = world->getPlayer()->getState();
    if (state == WON || state == DEAD)
    {
        window->fill_rect(Rectangle(Point(165, 195), Point(475, 285)), BLACK);
        const std::string message = state == WON ? "LEVEL COMPLETE!" : "GAME OVER";
        window->show_text(
            message,
            state == WON ? Point(220, 215) : Point(258, 215),
            state == WON ? GREEN : RED,
            font,
            27
        );
        window->show_text(
            "Score: " + std::to_string(world->getGameState()->score),
            Point(267, 252),
            WHITE,
            font,
            16
        );
    }
}

bool MarioRLRenderer::renderFrame(World *world, int action, int episodeSteps)
{
    Uint32 frameStart = SDL_GetTicks();
    if (!processEvents())
    {
        return false;
    }

    window->clear();
    drawBackground(world);
    drawObjects(world);
    drawHud(world, action, episodeSteps);
    window->update_screen();

    Uint32 elapsed = SDL_GetTicks() - frameStart;
    if (elapsed < static_cast<Uint32>(frameDelayMilliseconds))
    {
        SDL_Delay(frameDelayMilliseconds - elapsed);
    }
    return true;
}
