
#include "object.hpp"


class Text: public Object{
public:
    Text(int x, int y):Object(Point(x, y),
        Point(0, 0),
        "",
        G_TEXT){}

    void update(){
        if (!textAnimation.isStarted()){
            textAnimation.start();
            done = false;
        }

        if (!done){
            if (textAnimation.getTime() < 800){
                _moveY(-1);
            }
            else{
                done = true;
                ghost_dead = true;
            }
        }
    }

    
    bool flipped = false;
    bool done = false;
    std::string text;
private:
    int flip_cycle = 0;
    Timer textAnimation;
};
