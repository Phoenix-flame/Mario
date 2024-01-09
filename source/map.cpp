#include "map.hpp"

Map::Map(std::string filepath)
{
    this->offset.x = 0;
    this->offset.y = 0;

    std::ifstream map_file;
    map_file.open(filepath);
    std::string line;
    int y = 0;
    bool flag_pipes = false;
    bool flag_flag = false;
    std::vector<int> pipes_head;

    while (std::getline(map_file, line))
    {
        for (unsigned int x = 0; x < line.size(); x++)
        {
            if (line[x] == '@')
            {
                Block *tmp = new Block(x, y);
                this->blocks.push_back(tmp);
                this->objects.push_back(tmp);
                this->staticObjects.push_back(tmp);
            }
            else if (line[x] == '#')
            {
                Ground *tmp = new Ground(x, y);
                this->grounds.push_back(tmp);
                this->objects.push_back(tmp);
                this->staticObjects.push_back(tmp);
            }
            else if (line[x] == 'b')
            {
                Brick *tmp = new Brick(x, y);
                this->bricks.push_back(tmp);
                this->objects.push_back(tmp);
                this->staticObjects.push_back(tmp);
            }
            else if (line[x] == '?')
            {
                CoinContainer *tmp = new CoinContainer(x, y);
                this->coins.push_back(tmp);
                this->objects.push_back(tmp);
                this->staticObjects.push_back(tmp);
            }
            else if (line[x] == 'm')
            {
                FireContainer *tmp = new FireContainer(x, y);
                this->fires.push_back(tmp);
                this->objects.push_back(tmp);
                this->staticObjects.push_back(tmp);
            }
            else if (line[x] == 'h')
            {
                HealthContainer *tmp = new HealthContainer(x, y);
                this->healths.push_back(tmp);
                this->objects.push_back(tmp);
                this->staticObjects.push_back(tmp);
            }
            else if (line[x] == 'l')
            {
                Goomba *tmp = new Goomba(x, y);

                this->goombas.push_back(tmp);
                this->objects.push_back(tmp);
            }
            else if (line[x] == 'k')
            {
                Koopa *tmp = new Koopa(x, y);
                this->koopas.push_back(tmp);
                this->objects.push_back(tmp);
            }
            else if (line[x] == 'M')
            {
                // player = new Player(x, y);
                player = new Player(5, 17);
            }
            else if (line[x] == 'f')
            {
                if (!flag_flag)
                {
                    Flag *tmp = new Flag(x, y, "h");
                    this->objects.push_back(tmp);
                    flag_flag = true;
                }
                else
                {
                    Flag *tmp = new Flag(x, y, "b");
                    this->objects.push_back(tmp);
                }
            }
            else if (line[x] == '|')
            {
                bool exist = false;
                for (auto p : pipes_head)
                {
                    if (p == x)
                    {
                        exist = true;
                        if (flag_pipes)
                        { // Right
                            Pipe *tmp = new Pipe(x, y, "r");
                            pipes.push_back(tmp);
                            this->objects.push_back(tmp);
                            this->staticObjects.push_back(tmp);
                            flag_pipes = false;
                        }
                        else
                        { // Left
                            Pipe *tmp = new Pipe(x, y, "l");
                            pipes.push_back(tmp);
                            this->objects.push_back(tmp);
                            this->staticObjects.push_back(tmp);
                            flag_pipes = true;
                        }
                    }
                    continue;
                }
                if (!exist)
                {
                    if (flag_pipes)
                    { // Right
                        Pipe *tmp = new Pipe(x, y, "hr");
                        pipes.push_back(tmp);
                        this->objects.push_back(tmp);
                        this->staticObjects.push_back(tmp);
                        flag_pipes = false;
                    }
                    else
                    { // Left
                        Pipe *tmp = new Pipe(x, y, "hl");
                        pipes.push_back(tmp);
                        this->objects.push_back(tmp);
                        this->staticObjects.push_back(tmp);
                        flag_pipes = true;
                    }
                    pipes_head.push_back(x);
                }
            }
        }
        y += 1;
    }
    this->objects.push_back(player);
    map_file.close();
}
