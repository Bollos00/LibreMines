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

/**
 * @file minefieldthemepreview.h
 * @brief Preview widget for displaying minefield themes
 * @author Bruno Bollos Correa
 * @date 2025-12-31
 * 
 * This file implements MinefieldThemePreview, a UI component for previewing
 * minefield themes in the game preferences dialog. It displays a grid of cells
 * representing various game cell states (empty, mines, numbers, etc.) allowing
 * users to visually inspect how a theme will look before applying it to the game.
 */

 #ifndef MINEFIELDTHEMEPREVIEW_H
#define MINEFIELDTHEMEPREVIEW_H

#include "minefieldtheme.h"
#include "qlabel_adapted.h"

#include <QWidget>

namespace Ui {
class MinefieldThemePreview;
}

/**
 * @brief Preview widget for minefield themes
 * 
 * MinefieldThemePreview is a UI component that displays a representative grid
 * of minefield cells showcasing different cell states (unrevealed cells, mines,
 * flagged cells, cells with numbers, etc.) for a given minefield theme. This
 * allows users to preview the visual appearance of a theme before applying it
 * to the actual game. The widget dynamically creates and manages QLabel_adapted
 * widgets that display theme pixmaps for each cell state. It is typically
 * embedded in a GameplayThemeChooser dialog alongside a radio button for theme
 * selection.
 * 
 * @see MinefieldTheme
 * @see GameplayThemeChooser
 * @see FacesReactionPreview
 * @see QLabel_adapted
 * @since v2.3.0
 */
class MinefieldThemePreview : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Construct a MinefieldThemePreview widget
     * 
     * Initializes the preview widget with the UI definition and prepares
     * it for displaying minefield theme cell previews. The widget is not
     * yet populated with any theme data until vSetMinefieldTheme() is called.
     * 
     * @param parent Parent widget that owns this preview widget (optional, defaults to nullptr)
     */
    explicit MinefieldThemePreview(QWidget *parent = nullptr);
    
    /**
     * @brief Destructor
     * 
     * Cleans up UI resources and destroys all allocated QLabel_adapted
     * widgets used for displaying minefield theme cells.
     */
    ~MinefieldThemePreview();
    
    /**
     * @brief Set and preview a minefield theme
     * 
     * Loads the specified minefield theme and displays a representative grid
     * of cells showing different cell states for the theme. If a theme was
     * previously loaded, it is replaced with the new theme. The length
     * parameter controls the size of each cell display in pixels.
     * 
     * @param theme Name of the minefield theme to preview (e.g., "default", "custom", "neon")
     * @param lenght Size in pixels for each cell display (note: parameter name has typo in original code)
     * 
     * @see MinefieldTheme::setMinefieldTheme()
     */
    void vSetMinefieldTheme(const QString& theme, int lenght);

private:
    Ui::MinefieldThemePreview *ui;                  ///< UI definition for the preview widget
    MinefieldTheme minefieldTheme;                  ///< Manages loading and accessing minefield theme pixmaps
    int iCellLength;                                ///< Size in pixels for each cell display
    QList<QLabel_adapted*> listLbCells;             ///< List of adapted label widgets displaying each minefield cell state
};

#endif // MINEFIELDTHEMEPREVIEW_H
