#include "object.hpp"


class Pipe: public Object{
public:
    Pipe(int x, int y, std::string type):Object(Point(x, y), Point(24, 24), "assets/sprites/objects/pipe/right.png"){
        if (type == "r"){
            image = "assets/sprites/objects/pipe/right.png";
        }
        else if (type == "l"){
            image = "assets/sprites/objects/pipe/left.png";
        }
        else if (type == "hr"){
            image = "assets/sprites/objects/pipe/head-right.png";
        }
        else if (type == "hl"){
            image = "assets/sprites/objects/pipe/head-left.png";
        }
    }

private:
};