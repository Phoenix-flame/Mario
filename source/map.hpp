#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "rsdl.hpp"
#include "objects/block.hpp"
#include "objects/ground.hpp"
#include "objects/brick.hpp"
#include "objects/coin_container.hpp"
#include "objects/fire_block.hpp"
#include "objects/health_block.hpp"
#include "objects/goomba.hpp"
#include "objects/koopa_troopa.hpp"
#include "objects/pipes.hpp"
#include "player.hpp"


class Map{
public:
    Map(std::string filepath);
    void drawObjects();
    Point offset;

    Player* player;
    
    std::vector<Block*> blocks;
    std::vector<Ground*> grounds;
    std::vector<Brick*> bricks;
    std::vector<CoinContainer*> coins;
    std::vector<FireContainer*> fires;
    std::vector<HealthContainer*> healths;
    std::vector<Goomba*> goombas;
    std::vector<Koopa*> koopas;
    std::vector<Pipe*> pipes;
    std::vector<Object*> objects;
private:
    

    

    

};