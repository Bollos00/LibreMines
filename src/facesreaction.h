/**
 * @file facesreaction.h
 * @brief Emoji reaction system for LibreMines game events
 * @author Bruno Bollos Correa
 * @date 2021-03-29
 * 
 * This file defines the FacesReaction class which manages emoji/face
 * reactions that provide visual feedback to the player based on game
 * events and user interactions. The faces reflect the current game
 * state and add personality to the user interface.
 */

#ifndef FACESREACTION_H
#define FACESREACTION_H

#include <QScopedPointer>
#include <QPixmap>

/**
 * @brief Emoji reaction system for game state visualization
 * 
 * FacesReaction manages a collection of emoji faces that react to different
 * game events and states. These visual cues help communicate the current
 * game status to the player and add an engaging element to the interface.
 * 
 * The system supports theming, allowing different sets of emoji reactions
 * to be loaded and displayed. Face images are scaled appropriately for
 * the current display settings.
 * 
 * @since v1.4.0
 */
class FacesReaction
{
public:
    /**
     * @brief Types of game events that trigger face reactions
     * 
     * Enumeration of all game states and events that have associated
     * emoji reactions in the user interface.
     */
    enum GameEvent
    {
        GAME_LOST,                 ///< Player hit a mine and lost the game
        UNHIDDEN_CELL_PRESSED,     ///< Player is pressing on an already revealed cell
        GAME_WON,                  ///< Player successfully completed the game
        HIDDEN_CELL_PRESSED,       ///< Player is pressing on a hidden cell
        GAME_BEING_GENERATED,      ///< New game is being set up
        DEFAULT                    ///< Normal/idle game state
    };

    /**
     * @brief Default constructor
     * 
     * Initializes the face reaction system with default pixmaps.
     * Face images must be loaded using vSetFacesReactionTheme() before use.
     */
    FacesReaction();

    /**
     * @brief Set the theme for face reactions
     * 
     * Loads a set of emoji face images for the specified theme and scales
     * them to the requested size. This allows customization of the face
     * appearance based on user preferences or theme selection.
     * 
     * @param which Name of the theme to load
     * @param length Target size (width/height) for the face images in pixels
     */
    void vSetFacesReactionTheme(const QString &which, const int length);
    
    /**
     * @brief Get the appropriate face image for a game event
     * 
     * Returns the pixmap corresponding to the specified game event or state.
     * The returned pixmap can be directly displayed in the user interface.
     * 
     * @param ev The game event for which to retrieve the face reaction
     * @return Const reference to the pixmap for the specified event
     */
    const QPixmap& getPixmapFromGameEvent(const GameEvent ev);

private:
    QScopedPointer<QPixmap> pmDizzyFace;      ///< Face displayed when game is lost
    QScopedPointer<QPixmap> pmGrimacingFace;  ///< Face displayed when unhidden cell is pressed
    QScopedPointer<QPixmap> pmGrinningFace;   ///< Face displayed when game is won
    QScopedPointer<QPixmap> pmOpenMouthFace;  ///< Face displayed when hidden cell is pressed
    QScopedPointer<QPixmap> pmSleepingFace;   ///< Face displayed during game generation
    QScopedPointer<QPixmap> pmSmillingFace;   ///< Face displayed in default/idle state

signals:
};

#endif // FACESREACTION_H
