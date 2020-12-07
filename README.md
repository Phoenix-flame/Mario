# Mario
This project is an implementation of Nintendo Mario game using SDL2, as a part of Advance Programming course at University of Tehran.

![alt text](https://github.com/Phoenix-flame/Mario/blob/master/images/1.png?raw=true)


### Game Structure
This game is a simple part of mario game(only world 1-1), so there is no need to use any game engine, I've implemented whole game from scratch using C++.

Main game's structure is implemented in Core class which contains all game engine functionality.
Game loop, rendering, input handling and ... are all handled in this class.

Game Map loads from a simple 'txt' file which is a grid of characters that each one represents an object.
This structure makes everything much harder, because for example, a small 'pipe' in game consists of 4 blocks that each one is stored in memory and considered in physics engine.
In my opinion, this is not an optimal way to create game objects(anyway this has been noted in project description) either for checking physics or from memory perspective.
By the way this was a hobby project, and maybe I will change game structure later. 


### Physics


### Objects
As mentioned above all objects are created from multiple blocks that are loaded from 'map.txt'.

- Object Types

|Type| Image|
|--|--|
|BLOCK|![alt text](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/objects/bricks_blocks/block.png)|
|BRICK|![alt text](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/objects/bricks_blocks/brick.png)|
|GROUND|![alt text](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/objects/bricks_blocks/clay.png)|
|FIRE_CONTAINER|![alt text](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/objects/bricks_blocks/question-3.png)|
|HEALTH_CONTAINER|![alt text](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/objects/bricks_blocks/question-2.png)|
|COIN_CONTAINER|![alt text](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/objects/bricks_blocks/question-1.png)|
|PIPE|![alt text](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/objects/pipe/head-right.png) ![alt text](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/objects/pipe/head-left.png) ![alt text](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/objects/pipe/right.png) ![alt text](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/objects/pipe/left.png)|
|FLAG|![alt text](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/objects/flag/body.png) ![alt text](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/objects/flag/head.png)|
|GOOMBA|![alt text](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/enemies/little_goomba/walking-1.png) ![alt text](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/enemies/little_goomba/walking-2.png) ![alt text](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/enemies/little_goomba/dead.png)|
|KOOPA|![alt text](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/enemies/koopa_troopa/walking-left-1.png) ![alt text](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/enemies/koopa_troopa/walking-left-2.png) ![alt text](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/enemies/koopa_troopa/walking-right-1.png) ![alt text](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/enemies/koopa_troopa/walking-right-2.png) ![alt text](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/enemies/koopa_troopa/dead.png)|
|PLAYER|![alt text](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/mario/normal/walking-right-1.png) ![alt text](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/mario/big/walking-right-1.png) ![alt text](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/mario/white/walking-right-1.png)|
|G_COIN|![alt text](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/objects/coin.png)|
|G_MUSHROOM|![alt text](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/objects/mushroom/health.png) ![alt text](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/objects/mushroom/red.png)|
|G_FLOWER|![alt text](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/objects/flower.png)|

- Object's variables 

All objects are inherited from 'Object' class. This class consists related functions and variables, e.g

|Variable<Type>| Usage|
|--|--|
|pos<Point>| Current Object position|
|size<Point>| object size |
|image<String>| Image path for rendering|
|type<Type>| Object's type (GROUND, GOOMBA, KOOPA, CLOUD, etc.)|
|startAnimation<bool>| Is used for starting animation. (COIN, MUSHROOM, LEVELUP, etc.)|



### Graphic


### Other
Debug visual tools :)

![alt text](https://github.com/Phoenix-flame/Mario/blob/master/images/3.png?raw=true)
