#ifndef _AUDIO_HPP_
#define _AUDIO_HPP_
#include "player.hpp"
#include "rsdl.hpp"

#define MAIN_THEME_SOUND "./assets/sounds/Super Mario Bros. theme music.ogg"
#define BRICK_SMASH_SOUND "./assets/sounds/sound_effects/brick-smash.wav"
#define COIN_SOUND "./assets/sounds/sound_effects/coin.wav"
#define JUMP_SMALL_SOUND "./assets/sounds/sound_effects/jump-small.wav"
#define JUMP_SUPER_SOUND "./assets/sounds/sound_effects/jump-super.wav"
#define POWER_UP_SOUND "./assets/sounds/sound_effects/powerup.wav"
#define ENEMY_STOPM_SOUND "./assets/sounds/sound_effects/enemy-stomp.wav"
#define POWER_UP_APPEAR_SOUND "./assets/sounds/sound_effects/powerup-appears.wav"
#define MARIO_DEATH_SOUND "./assets/sounds/sound_effects/mario-death.wav"
#define LEVEL_CLEAR_SOUND "./assets/sounds/sound_effects/level-clear.wav"


class Audio{
public:
    Audio(Window* win);
    
    void update(Player* player);
    void reset();
    
private:
    Window* win;
};

#endif // !_AUDIO_HPP_