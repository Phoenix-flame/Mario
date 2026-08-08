#ifndef _TEXT_HPP_
#define _TEXT_HPP_

#include "object.hpp"

class Text : public Object
{
public:
    Text(int x, int y) : Object(Point(x, y),
                                Point(0, 0),
                                "",
                                G_TEXT) {}

    void update()
    {
        if (!textAnimation.isStarted())
        {
            textAnimation.start();
            done = false;
        }

        if (!done)
        {
            if (textAnimation.getTime() < 800)
            {
                _moveY(-1);
            }
            else
            {
                done = true;
                ghost_dead = true;
            }
        }
    }

    bool flipped = false;
    bool done = false;
    std::string text;
    int score;

private:
    int flip_cycle = 0;
    Timer textAnimation;
};

// Builds the floating "+ score" label that pops above `obj`.
inline Text *makeScoreText(Object *obj, int score)
{
    Text *text = new Text(obj->getPos().x, obj->getPos().y - 10);
    text->setPos(obj->getPos().x, obj->getPos().y - 10);
    text->ghost_dead = false;
    text->text = "+ " + std::to_string(score);
    text->score = score;
    return text;
}

// Queues a score label on `owner` so World::ghostCollector shows it and adds
// the points exactly once. `at` supplies the position the label pops from.
inline void queueScoreText(Object *owner, Object *at, int score)
{
    owner->ghost.push_back(makeScoreText(at, score));
    owner->has_ghost = true;
}

#endif // !_TEXT_HPP_
