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
 * @file facesreactionpreview.h
 * @brief Preview widget for displaying faces reaction themes
 * @author Bruno Bollos Correa
 * @date 2025-12-31
 * 
 * This file defines FacesReactionPreview, an UI component for previewing
 * faces reaction themes in the game. Users can see how various face reactions
 * appear before applying a specific faces theme to the game.
 */

#ifndef FACESREACTIONPREVIEW_H
#define FACESREACTIONPREVIEW_H

#include "facesreaction.h"

#include <QWidget>
#include <QLabel>
#include <QList>

namespace Ui {
class FacesReactionPreview;
}

/**
 * @brief Preview widget for faces reaction themes
 * 
 * FacesReactionPreview is a UI component that displays all available
 * face reactions for a given theme, allowing users to preview how
 * the faces will appear before selecting a theme. It follows the same
 * design pattern as MinefieldThemePreview for consistency across the
 * application.
 * 
 * @since v2.3.0
 */
class FacesReactionPreview : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Construct a FacesReactionPreview widget
     * 
     * @param parent Parent widget (optional)
     */
    explicit FacesReactionPreview(QWidget *parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~FacesReactionPreview();

    /**
     * @brief Set and preview a faces reaction theme
     * 
     * Loads the specified theme and displays all face reactions
     * (lost, won, pressed, etc.) in the preview widget.
     * 
     * @param theme Name of the theme to preview
     */
    void vSetFacesReactionTheme(const QString& theme, int length);

private:
    Ui::FacesReactionPreview *ui; ///< UI definition for the preview widget
    FacesReaction facesReaction;  ///< Manages loading and accessing face reaction pixmaps
    int iFaceLength;              ///< Size in pixels for each face reaction display
    QList<QLabel*> listLbFaces;   ///< List of label widgets displaying each face reaction
};

#endif // FACESREACTIONPREVIEW_H
