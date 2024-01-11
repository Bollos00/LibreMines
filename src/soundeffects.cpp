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

    connect(this, &SoundEffects::SIGNAL_gameWon,
            &soundGameWon, &QSoundEffect::play);

    connect(this, &SoundEffects::SIGNAL_gameLost,
            &soundGameLost, &QSoundEffect::play);


    connect(this, &SoundEffects::SIGNAL_keyboardControllerMove,
            &soundKeyboardControllMove, &QSoundEffect::play);

    connect(this, &SoundEffects::SIGNAL_releaseCell,
            &soundReleaseCell, &QSoundEffect::play);

    connect(this, &SoundEffects::SIGNAL_flagCell,
            &soundFlagCell, &QSoundEffect::play);
}

void SoundEffects::setVolume(const int vol)
{
    for(QSoundEffect* sound : soundEffects)
    {
        sound->setVolume(vol/100.f);
        sound->setMuted(vol == 0);
    }
}

