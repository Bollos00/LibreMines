#include "soundeffects.h"

#include <QFileInfo>

SoundEffects::SoundEffects(QObject* parent) :
    QObject(parent),
    player( new QMediaPlayer() )
{
    connect(this, &SoundEffects::SIGNAL_gameBegin,
            this, &SoundEffects::SLOT_gameBegin);

    connect(this, &SoundEffects::SIGNAL_gameWon,
            this, &SoundEffects::SLOT_gameWon);

    connect(this, &SoundEffects::SIGNAL_gameLost,
            this, &SoundEffects::SLOT_gameLost);

    connect(this, &SoundEffects::SIGNAL_clockTick,
            this, &SoundEffects::SLOT_clockTick);

    connect(this, &SoundEffects::SIGNAL_keyboardControllerMove,
            this, &SoundEffects::SLOT_keyboardControllerMove);

    connect(this, &SoundEffects::SIGNAL_releaseCell,
            this, &SoundEffects::SLOT_releaseCell);

    connect(this, &SoundEffects::SIGNAL_flagCell,
            this, &SoundEffects::SLOT_flagCell);
}

void SoundEffects::setVolume(const int vol)
{
    player->setVolume(vol);
}

void SoundEffects::setMuted(const bool mute)
{
    player->setMuted(mute);
}

void SoundEffects::SLOT_gameBegin()
{
    play("qrc:/sound_effects/game_begin.wav");
}

void SoundEffects::SLOT_gameWon()
{
    play("qrc:/sound_effects/game_won.wav");
}

void SoundEffects::SLOT_gameLost()
{

    play("qrc:/sound_effects/game_lost.wav");
}

void SoundEffects::SLOT_clockTick()
{
    play("qrc:/sound_effects/clock_tick.wav");
}

void SoundEffects::SLOT_keyboardControllerMove()
{
    play("qrc:/sound_effects/keyboard_controller_move.wav");
}

void SoundEffects::SLOT_releaseCell()
{
    play("qrc:/sound_effects/release_cell.wav");
}

void SoundEffects::SLOT_flagCell()
{
    play("qrc:/sound_effects/flag_cell.wav");
}

void SoundEffects::play(const QString& file_path)
{
    if(player->isMuted())
        return;

    player->setMedia(QUrl(file_path));
    player->play();
}
