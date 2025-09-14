/**
 * @file soundeffects.h
 * @brief Audio system for LibreMines game events
 * @author Bruno Bollos Correa
 * @date 2022-02-07
 * 
 * This file defines the SoundEffects class which manages all audio feedback
 * in the LibreMines application. It provides sound effects for various game
 * events including game state changes, user interactions, and keyboard
 * navigation to enhance the user experience.
 */

#ifndef SOUNDEFFECTS_H
#define SOUNDEFFECTS_H

#include <QSoundEffect>
#include <QScopedPointer>

/**
 * @brief Audio system for game sound effects
 * 
 * SoundEffects manages all audio feedback in LibreMines including sounds for
 * game events (start, win, lose), user interactions (cell clearing, flagging),
 * and keyboard navigation. It provides volume control and can be easily
 * enabled or disabled based on user preferences.
 * 
 * The class uses Qt's QSoundEffect system for low-latency audio playback
 * suitable for interactive applications. All sound files are managed as
 * Qt resources for reliable cross-platform deployment.
 * 
 * @since v1.9.0
 */
class SoundEffects : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor for the sound effects system
     * 
     * Initializes all sound effects with their respective audio files
     * and sets up the audio system for playback.
     * 
     * @param parent Parent QObject for memory management
     */
    SoundEffects(QObject* parent = nullptr);

    /**
     * @brief Types of sound effects available in the game
     * 
     * Enumeration of all sound events that can be triggered during
     * gameplay and user interaction.
     */
    enum SoundType {
        GAME_BEGIN,                    ///< Sound played when a new game starts
        GAME_WON,                      ///< Sound played when the player wins
        GAME_LOST,                     ///< Sound played when the player loses (hits a mine)
        KEYBOARD_CONTROLLER_MOVE,      ///< Sound played when navigating with keyboard
        RELEASE_CELL,                  ///< Sound played when revealing a cell
        FLAG_CELL                      ///< Sound played when placing or removing a flag
    };

public Q_SLOTS:
    /**
     * @brief Initialize sound effects after thread movement
     * 
     * This slot must be called after moveToThread() to ensure
     * QSoundEffect objects are created in the correct thread context.
     */
    void SLOT_initializeSounds();

    /**
     * @brief Play a specific sound effect
     * 
     * Triggers playback of the specified sound type. If the sound system
     * is disabled or the volume is set to 0, no sound will be played.
     * 
     * @param type The type of sound effect to play
     */
    void SLOT_playSound(const SoundType type);
    
    /**
     * @brief Set the volume for all sound effects
     * 
     * Adjusts the volume level for all sound effects in the system.
     * Volume changes apply to all sound types uniformly.
     * 
     * @param vol Volume level from 0 (muted) to 100 (maximum)
     */
    void SLOT_setVolume(const int vol);

private:
    QScopedPointer<QSoundEffect> soundGameBegin;         ///< Sound effect for game start
    QScopedPointer<QSoundEffect> soundGameWon;           ///< Sound effect for game victory
    QScopedPointer<QSoundEffect> soundGameLost;          ///< Sound effect for game loss

    QScopedPointer<QSoundEffect> soundKeyboardControlMove; ///< Sound effect for keyboard navigation
    QScopedPointer<QSoundEffect> soundReleaseCell;       ///< Sound effect for cell revealing
    QScopedPointer<QSoundEffect> soundFlagCell;          ///< Sound effect for flag operations

    QList<QSoundEffect*> soundEffects;  ///< List of all sound effects for batch operations
    bool soundsInitialized;              ///< Flag to track if sounds have been initialized
};

Q_DECLARE_METATYPE(SoundEffects::SoundType)

#endif // SOUNDEFFECTS_H
