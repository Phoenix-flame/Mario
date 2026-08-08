#include "object.hpp"

class Flag : public Object
{
public:
    Flag(int x, int y, std::string type) : Object(Point(x, y),
                                                  Point(24, 24),
                                                  "",
                                                  PIPE)
    {

        if (type == "h")
        {
            image = "assets/sprites/objects/flag/head.png";
        }
        else if (type == "b")
        {
            image = "assets/sprites/objects/flag/body.png";
        }
    }

private:
};