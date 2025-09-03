/**
 * @file minefieldtheme.h
 * @brief Visual theme management for LibreMines minefield cells
 * @author Bruno Bollos Correa
 * @date 2021-01-16
 * 
 * This file defines the MinefieldTheme class which manages the visual
 * appearance of minesweeper cells including numbers, mines, flags, and
 * other game elements. It provides theme loading and pixmap management
 * for customizable visual styles.
 */

#ifndef MINEFIELDTHEME_H
#define MINEFIELDTHEME_H

#include <QScopedPointer>
#include <QPixmap>

#include "common.h"

/**
 * @brief Visual theme management for minefield cell graphics
 * 
 * MinefieldTheme handles the loading and management of all graphical
 * elements used to display the minesweeper grid. It supports multiple
 * themes and automatically scales graphics to match the current cell
 * size requirements.
 * 
 * The class manages pixmaps for:
 * - Number cells (0-8 adjacent mines)
 * - Mine cells and mine explosion effects
 * - Flag states (no flag, flag, question mark)
 * - Special states (wrong flags, boom effects)
 * 
 * Themes are loaded from resource files or external directories and
 * can be dynamically switched during gameplay to provide visual
 * customization options for users.
 * 
 * @since v1.2.2
 */
class MinefieldTheme
{
public:
    /**
     * @brief Default constructor
     * 
     * Initializes the theme system with empty pixmaps. A theme must be
     * loaded using vSetMinefieldTheme() before the theme can be used.
     */
    MinefieldTheme();

    /**
     * @brief Load and set a minefield theme
     * 
     * Loads all graphical elements for the specified theme and scales
     * them to the requested cell size. This method handles theme
     * validation and fallback to default themes if loading fails.
     * 
     * @param theme Name or path of the theme to load
     * @param cellLength Target size (width/height) for cell graphics in pixels
     */
    void vSetMinefieldTheme(const QString &theme, const int cellLength);

    /**
     * @brief Get pixmap for a specific cell value
     * 
     * Returns the appropriate graphic for a revealed cell based on its
     * value (mine or number of adjacent mines).
     * 
     * @param value The CellValue to get the pixmap for
     * @return Const reference to the pixmap for the specified cell value
     */
    const QPixmap& getPixmapFromCellValue(const CellValue value)const;
    
    /**
     * @brief Get pixmap for a button in a specific flag state
     * 
     * Returns the appropriate graphic for a hidden cell button based
     * on its current flag state.
     * 
     * @param state The FlagState to get the pixmap for
     * @return Const reference to the pixmap for the specified flag state
     */
    const QPixmap& getPixmapButton(const FlagState state);
    
    /**
     * @brief Get pixmap for question mark flag
     * 
     * @return Const reference to the question mark flag pixmap
     */
    const QPixmap& getPixmapQuestion();
    
    /**
     * @brief Get pixmap for incorrectly placed flag
     * 
     * Used to display flags that were placed on non-mine cells when
     * the game ends in defeat.
     * 
     * @return Const reference to the wrong flag pixmap
     */
    const QPixmap& getPixmapWrongFlag();
    
    /**
     * @brief Get pixmap for mine explosion effect
     * 
     * Used to display the mine that was clicked to end the game,
     * typically with a red background or explosion effect.
     * 
     * @return Const reference to the explosion effect pixmap
     */
    const QPixmap& getPixmapBoom();
    
private:
    QScopedPointer<QPixmap> pmMine;         ///< Pixmap for mine cells
    QScopedPointer<QPixmap> pmZero;         ///< Pixmap for cells with 0 adjacent mines
    QScopedPointer<QPixmap> pmOne;          ///< Pixmap for cells with 1 adjacent mine
    QScopedPointer<QPixmap> pmTwo;          ///< Pixmap for cells with 2 adjacent mines
    QScopedPointer<QPixmap> pmThree;        ///< Pixmap for cells with 3 adjacent mines
    QScopedPointer<QPixmap> pmFour;         ///< Pixmap for cells with 4 adjacent mines
    QScopedPointer<QPixmap> pmFive;         ///< Pixmap for cells with 5 adjacent mines
    QScopedPointer<QPixmap> pmSix;          ///< Pixmap for cells with 6 adjacent mines
    QScopedPointer<QPixmap> pmSeven;        ///< Pixmap for cells with 7 adjacent mines
    QScopedPointer<QPixmap> pmEight;        ///< Pixmap for cells with 8 adjacent mines

    QScopedPointer<QPixmap> pmFlag;         ///< Pixmap for flagged cells
    QScopedPointer<QPixmap> pmQuestionFlag; ///< Pixmap for question mark cells
    QScopedPointer<QPixmap> pmNoFlag;       ///< Pixmap for unflagged hidden cells
    QScopedPointer<QPixmap> pmBoom;         ///< Pixmap for exploded mine
    QScopedPointer<QPixmap> pmWrongFlag;    ///< Pixmap for incorrectly flagged cells
};

#endif // MINEFIELDTHEME_H
