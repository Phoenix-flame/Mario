#include "object.hpp"


class Cloud: public Object{
public:
    Cloud(int x, int y):Object(Point(x, y),
        Point(0, 0), 
        "",
        CLOUD){

    }

private:
};