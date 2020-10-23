#ifndef _COIN_CONTAINER_HPP_
#define _COIN_CONTAINER_HPP_

#include "object.hpp"
#include "coin.hpp"
#include "text.hpp"
#include "../rsdl.hpp"

#define COIN_BLOCK_FULL "assets/sprites/objects/bricks_blocks/question-1.png"
#define COIN_BLOCK_EMPTY "assets/sprites/objects/bricks_blocks/question-empty.png"


enum CoinState{
    COIN_NORMAL,
    COIN_RELEASED,
    EMPTY
};



class CoinContainer: public Object{
public:
    CoinContainer(int x, int y):Object(Point(x, y),
        Point(24, 24), 
        COIN_BLOCK_FULL,
        COIN_CONTAINER){
        state = COIN_NORMAL;
    }

    void update(){
        if (startAnimation){
            if (state == COIN_NORMAL){
                if (!animationTimer.isStarted()){
                    animationTimer.start();
                    prevPos = pos;

                    Coin* coin = new Coin(pos.x, pos.y - 10);
                    ghost.push_back(coin);
                    coin->setPos(pos.x, pos.y - 10);
                    coin->ghost_dead = false;

                    Text* text = new Text(pos.x, pos.y - 10);
                    text->setPos(pos.x, pos.y - 10);
                    text->ghost_dead = false;
                    text->text = "+ 100";
                    text->score = 100;
                    ghost.push_back(text);

                    has_ghost = true;
                }
                if (animationTimer.getTime() < 150){
                    _moveY(-1);
                }
                else if (prevPos.y != pos.y){
                    image = COIN_BLOCK_EMPTY;
                    _moveY(+1);
                }
                else{
                    animationTimer.reset();
                    state = COIN_RELEASED;
                    coinIsAvailable = false;
                }
            }
        }
    }

    bool coinIsAvailable = true;
private:
    Timer animationTimer;
    
    Point prevPos;
    CoinState state;

};

#endif // !_COIN_CONTAINER_HPP_