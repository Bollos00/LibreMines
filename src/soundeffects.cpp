#include "soundeffects.h"

#include <QFileInfo>

SoundEffects::SoundEffects(QObject* parent) :
    QObject( parent ),
    soundEffects(),
    soundsInitialized(false)
{
    // Sound effects will be initialized later via SLOT_initializeSounds()
    // after the object is moved to its target thread
}

void SoundEffects::SLOT_initializeSounds()
{
    if (soundsInitialized) {
        return; // Already initialized
    }

    soundGameBegin.reset(new QSoundEffect(this));
    soundGameWon.reset(new QSoundEffect(this));
    soundGameLost.reset(new QSoundEffect(this));
    soundKeyboardControlMove.reset(new QSoundEffect(this));
    soundReleaseCell.reset(new QSoundEffect(this));
    soundFlagCell.reset(new QSoundEffect(this));

    soundGameBegin->setSource(QUrl("qrc:/sound_effects/clock_tick.wav"));
    soundGameWon->setSource(QUrl("qrc:/sound_effects/game_won.wav"));
    soundGameLost->setSource(QUrl("qrc:/sound_effects/game_lost.wav"));
    
    soundKeyboardControlMove->setSource(QUrl("qrc:/sound_effects/move.wav"));
    soundReleaseCell->setSource(QUrl("qrc:/sound_effects/release_cell.wav"));
    soundFlagCell->setSource(QUrl("qrc:/sound_effects/flag_cell.wav"));

    soundEffects.append(soundGameBegin.data());
    soundEffects.append(soundGameWon.data());
    soundEffects.append(soundGameLost.data());
    soundEffects.append(soundKeyboardControlMove.data());
    soundEffects.append(soundReleaseCell.data());
    soundEffects.append(soundFlagCell.data());
    
    soundsInitialized = true;
}

void SoundEffects::SLOT_setVolume(const int vol, const bool playPreview)
{
    if (!soundsInitialized) {
        // Avoid setting volume before initialization
        return;
    }
    
    for(QSoundEffect* sound : soundEffects)
    {
        sound->setVolume(vol/100.f);
        sound->setMuted(vol == 0);
    }

    // Preview sound volume.
    if (playPreview)
    {
        SLOT_playSound(RELEASE_CELL);
    }
}

void SoundEffects::SLOT_playSound(const SoundType type)
{
    if (!soundsInitialized) {
        // Avoid playing sound before initialization
        return;
    }
    switch(type)
    {
        case SoundEffects::GAME_BEGIN:
        {
            if(!soundGameBegin->isMuted())
            {
                qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "Playing game begin sound";
                soundGameBegin->stop();
                qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "Game begin sound stopped";
                soundGameBegin->play();
                qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "Game begin sound started";
            }
        }break;
        case SoundEffects::GAME_WON:
        {
            if(!soundGameWon->isMuted())
            {
                qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "Playing game won sound";
                soundGameWon->stop();
                qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "Game won sound stopped";
                soundGameWon->play();
                qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "Game won sound started";
            }
        }break;
        case SoundEffects::GAME_LOST:
        {
            if(!soundGameLost->isMuted())
            {
                qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "Playing game lost sound";
                soundGameLost->stop();
                qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "Game lost sound stopped";
                soundGameLost->play();
                qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "Game lost sound started";
            }
        }break;
        case SoundEffects::KEYBOARD_CONTROLLER_MOVE:
        {
            if(!soundKeyboardControlMove->isMuted())
            {
                qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "Playing keyboard move sound";
                soundKeyboardControlMove->stop();
                qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "Keyboard move sound stopped";
                soundKeyboardControlMove->play();
                qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "Keyboard move sound started";
            }
        }break;
        case SoundEffects::RELEASE_CELL:
        {
            if(!soundReleaseCell->isMuted())
            {
                qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "Playing release cell sound";
                soundReleaseCell->stop();
                qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "Release cell sound stopped";
                soundReleaseCell->play();
                qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "Release cell sound started";
            }
        }break;
        case SoundEffects::FLAG_CELL:
        {
            if(!soundFlagCell->isMuted())
            {
                qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "Playing flag cell sound";
                soundFlagCell->stop();
                qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "Flag cell sound stopped";
                soundFlagCell->play();
                qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "Flag cell sound started";
            }
        }break;
    }
}

