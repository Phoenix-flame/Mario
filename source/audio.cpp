#include "audio.hpp"

Audio::Audio(Window* win){
    this->win = win;
    win->play_music(MAIN_THEME_SOUND);
}

void Audio::update(Player* player){
    if (player->jump_a){
        if (player->getLevel() == NORMAL){
            win->play_sound_effect(JUMP_SMALL_SOUND);
        }
        else{
            win->play_sound_effect(JUMP_SUPER_SOUND);
        }
        player->jump_a = false;
    }
    if (player->enemy_stomp_a){
        win->play_sound_effect(ENEMY_STOPM_SOUND);
        player->enemy_stomp_a = false;
    }
    if (player->coin_a){
        win->play_sound_effect(COIN_SOUND);
        player->coin_a = false;
    }
    if (player->death_a){
        win->stop_music();
        win->play_sound_effect(MARIO_DEATH_SOUND);
        player->death_a = false;
    }
}


void Audio::reset(){
    win->play_music(MAIN_THEME_SOUND);
}