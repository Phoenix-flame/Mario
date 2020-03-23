#include "object.hpp"

enum CoinState{
    COIN,
    COIN_RELEASED,
    EMPTY
};

class CoinContainer: public Object{
public:
    CoinContainer(int x, int y):Object(Point(x, y), Point(24, 24), "assets/sprites/objects/bricks_blocks/question-1.png"){
        state = COIN;
    }

    void update(){
        if (startAnimation && (state == COIN || state == COIN_RELEASED)){
            std::cout << "coin animation" << std::endl;
            if (state == COIN){
                if (!animationTimer.isStarted()){
                animationTimer.start();
                prevPos = pos;
            }
            if (animationTimer.getTime() < 150){
                if (!coinAnimation.isStarted()){
                    coinAnimation.start();
                    coinIsAvailable = true;
                    posCoin = pos - Point(-5, -10);
                    sizeCoin = Point(16, 16);
                }
                _moveY(-1);
            }
            else if (prevPos.y != pos.y){
                image = image_empty;
                _moveY(+1);
            }
            else{
                animationTimer.reset();
                
                state = COIN_RELEASED;
            }
            }
            
            if (coinIsAvailable){
                if (coinAnimation.getTime() < 200){
                    posCoin.y -= 10;
                }
                else if (coinAnimation.getTime() >= 200 &&
                coinAnimation.getTime() < 380){
                    posCoin.y += 10;
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
        return image_coin;
    }
    Point getCoinPos(){
        return posCoin;
    }
    Point getCoinSize(){
        return sizeCoin;
    }
    bool coinIsAvailable = false;
private:
    std::string image_empty = "assets/sprites/objects/bricks_blocks/question-empty.png";
    std::string image_coin = "assets/sprites/objects/coin.png";
    Timer animationTimer;
    Timer coinAnimation;
    Point prevPos;

    Point posCoin;
    Point sizeCoin;

    CoinState state;
};