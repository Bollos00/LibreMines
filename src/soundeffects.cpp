#include "soundeffects.h"

#include <QFileInfo>

SoundEffects::SoundEffects(QObject* parent) :
    QObject( parent ),
    soundEffects( {&soundGameBegin, &soundGameWon, &soundGameLost,
                  &soundKeyboardControllMove, &soundReleaseCell, &soundFlagCell} )
{
    soundGameBegin.setSource(QUrl("qrc:/sound_effects/clock_tick.wav"));
    soundGameWon.setSource(QUrl("qrc:/sound_effects/game_won.wav"));
    soundGameLost.setSource(QUrl("qrc:/sound_effects/game_lost.wav"));
    
    soundKeyboardControllMove.setSource(QUrl("qrc:/sound_effects/move.wav"));
    soundReleaseCell.setSource(QUrl("qrc:/sound_effects/release_cell.wav"));
    soundFlagCell.setSource(QUrl("qrc:/sound_effects/flag_cell.wav"));
    
}

void SoundEffects::SLOT_setVolume(const int vol)
{
    for(QSoundEffect* sound : soundEffects)
    {
        sound->setVolume(vol/100.f);
        sound->setMuted(vol == 0);
    }
    // Preview sound volume.
    SLOT_playSound(RELEASE_CELL);
}

void SoundEffects::SLOT_playSound(const SoundType type)
{
    switch(type)
    {
        case SoundEffects::GAME_BEGIN:
        {
            if(!soundGameBegin.isMuted())
            {
// #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
//                 if(soundGameBegin.isPlaying())
//                 {
//                     soundGameBegin.stop();
//                 }
// #endif
                soundGameBegin.play();
            }
        }break;
        case SoundEffects::GAME_WON:
        {
            if(!soundGameWon.isMuted())
            {
// #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
//                 if(soundGameWon.isPlaying())
//                 {
//                     soundGameWon.stop();
//                 }
// #endif
                soundGameWon.play();
            }
        }break;
        case SoundEffects::GAME_LOST:
        {
            if(!soundGameLost.isMuted())
            {
// #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
//                 if(soundGameLost.isPlaying())
//                 {
//                     soundGameLost.stop();
//                 }
// #endif
                soundGameLost.play();
            }
        }break;
        case SoundEffects::KEYBOARD_CONTROLLER_MOVE:
        {
            if(!soundKeyboardControllMove.isMuted())
            {
// #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
//                 if(soundKeyboardControllMove.isPlaying())
//                 {
//                     soundKeyboardControllMove.stop();
//                 }
// #endif
                soundKeyboardControllMove.play();
            }
        }break;
        case SoundEffects::RELEASE_CELL:
        {
            if(!soundReleaseCell.isMuted())
            {
// #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
//                 if(soundReleaseCell.isPlaying())
//                 {
//                     soundReleaseCell.stop();
//                 }
// #endif
                soundReleaseCell.play();
            }
        }break;
        case SoundEffects::FLAG_CELL:
        {
            if(!soundFlagCell.isMuted())
            {
// #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
//                 if(soundFlagCell.isPlaying())
//                 {
//                     soundFlagCell.stop();
//                 }
//                 soundFlagCell.play();
// #endif
            }
        }break;
    }
}

