#include "object.hpp"

#define COIN_BLOCK_FULL "assets/sprites/objects/bricks_blocks/question-1.png"
#define COIN_BLOCK_EMPTY "assets/sprites/objects/bricks_blocks/question-empty.png"
#define COIN_IMAGE "assets/sprites/objects/coin.png"


enum CoinState{
    COIN,
    COIN_RELEASED,
    EMPTY
};



class CoinContainer: public Object{
public:
    CoinContainer(int x, int y):Object(Point(x, y),
        Point(24, 24), 
        COIN_BLOCK_FULL,
        COIN_CONTAINER){
        state = COIN;
    }

    void update(){
        if (startAnimation && (state == COIN || state == COIN_RELEASED)){
            if (state == COIN){
                if (!animationTimer.isStarted()){
                animationTimer.start();
                prevPos = pos;
            }
            if (animationTimer.getTime() < 150){
                if (!coinAnimation.isStarted()){
                    coinAnimation.start();
                    coinIsAvailable = true;
                    posCoin = pos - Point(0, -10);
                    sizeCoin = Point(20, 20);
                }
                _moveY(-1);
            }
            else if (prevPos.y != pos.y){
                image = COIN_BLOCK_EMPTY;
                _moveY(+1);
            }
            else{
                animationTimer.reset();
                
                state = COIN_RELEASED;
            }
            }
            
            if (coinIsAvailable){
                if (coinAnimation.getTime() < 200){
                    posCoin.y -= 15;
                }
                else if (coinAnimation.getTime() >= 200 &&
                coinAnimation.getTime() < 300){
                    posCoin.y += 15;
                }
                else{
                    coinIsAvailable = false;
                    state = EMPTY;
                    startAnimation = false;
                }
            }
            
        }
    }
    std::string getImageCoin(){
        return COIN_IMAGE;
    }
    Point getCoinPos(){
        return posCoin;
    }
    Point getCoinSize(){
        return sizeCoin;
    }
    bool coinIsAvailable = false;
private:
    Timer animationTimer;
    Timer coinAnimation;
    Point prevPos;

    Point posCoin;
    Point sizeCoin;

    CoinState state;
};