#include "soundeffects.h"



SoundEffects::SoundEffects() :
    player( new QMediaPlayer() )
{

}

void SoundEffects::setVolume(const int vol)
{
    player->setVolume(vol);
}

void SoundEffects::gameBegin()
{
    play(":/sound_effects/game_begin.wav");
}

void SoundEffects::gameWon()
{
    play(":/sound_effects/game_won.wav");
}

void SoundEffects::gameLost()
{

    play(":/sound_effects/game_lost.wav");
}

void SoundEffects::clockTick()
{
    play(":/sound_effects/clock_tick.wav");
}

void SoundEffects::keyboardControllerMove()
{
    play(":/sound_effects/keyboard_controller_move.wav");
}

void SoundEffects::releaseCell()
{
    play(":/sound_effects/release_cell.wav");
}

void SoundEffects::flagCell()
{
    play(":/sound_effects/flag_cell.wav");
}

void SoundEffects::play(const QString& file_path)
{
    player->setMedia(QUrl::fromLocalFile(file_path));
    player->play();
}
