/*****************************************************************************
 * LibreMines                                                                *
 * Copyright (C) 2020-2026  Bruno Bollos Correa                              *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 3 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************
 */

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
    
    soundEffects.append(soundGameBegin);
    soundEffects.append(soundGameWon);
    soundEffects.append(soundKeyboardControlMove);
    soundEffects.append(soundGameLost);
    soundEffects.append(soundReleaseCell);
    soundEffects.append(soundFlagCell);

    soundsInitialized = true;
}

void SoundEffects::SLOT_setVolume(const int vol, const bool playPreview)
{
    if (!soundsInitialized) {
        // Avoid setting volume before initialization
        return;
    }

    for(const QSharedPointer<QSoundEffect>& sound : soundEffects)
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
                // soundGameBegin->stop();
                soundGameBegin->play();
            }
        }break;
        case SoundEffects::GAME_WON:
        {
            if(!soundGameWon->isMuted())
            {
                // soundGameWon->stop();
                soundGameWon->play();
            }
        }break;
        case SoundEffects::GAME_LOST:
        {
            if(!soundGameLost->isMuted())
            {
                // soundGameLost->stop();
                soundGameLost->play();
            }
        }break;
        case SoundEffects::KEYBOARD_CONTROLLER_MOVE:
        {
            if(!soundKeyboardControlMove->isMuted())
            {
                // soundKeyboardControlMove->stop();
                soundKeyboardControlMove->play();
            }
        }break;
        case SoundEffects::RELEASE_CELL:
        {
            if(!soundReleaseCell->isMuted())
            {
                // soundReleaseCell->stop();
                soundReleaseCell->play();   
            }
        }break;
        case SoundEffects::FLAG_CELL:
        {
            if(!soundFlagCell->isMuted())
            {
                // soundFlagCell->stop();
                soundFlagCell->play();
            }
        }break;
    }
}

