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
 * @file gameplaythemechooser.h
 * @brief Dialog for selecting and previewing minefield and faces reaction themes
 * @author Bruno Bollos Correa
 * @date 2025-12-31
 * 
 * This file implements GameplayThemeChooser, a dialog window that allows users
 * to browse and select both minefield themes and faces reaction themes for their
 * gameplay.
 */

#ifndef GAMEPLAYTHEMECHOOSER_H
#define GAMEPLAYTHEMECHOOSER_H

#include "facesreactionpreview.h"
#include "minefieldthemepreview.h"

#include <QDialog>
#include <QRadioButton>
#include <QButtonGroup>

namespace Ui {
class GameplayThemeChooser;
}

/**
 * @brief Dialog for selecting and previewing gameplay themes
 * 
 * GameplayThemeChooser provides a comprehensive interface for users to select
 * and preview both minefield and faces reaction themes. It displays all available
 * themes with radio button selectors and live preview widgets. The dialog manages
 * two separate theme categories: minefield themes (which control the appearance
 * of the game board cells) and faces reaction themes (which control the UI faces
 * that react to game events). User selections can be retrieved via getter methods
 * and set programmatically via setter methods. The dialog emits visibility change
 * signals to notify the main application when the theme chooser is shown or hidden.
 * 
 * @see FacesReactionPreview
 * @see MinefieldThemePreview
 * @see MinefieldTheme
 * @see FacesReaction
 * @since v2.3.0
 */
class GameplayThemeChooser : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Construct the GameplayThemeChooser dialog
     * 
     * Initializes the theme chooser dialog with UI elements and populates
     * it with all available minefield and faces reaction themes. Each theme
     * option includes a radio button for selection and a preview widget for
     * visual inspection before applying the theme.
     * 
     * @param parent Parent widget that owns this dialog (optional, defaults to nullptr)
     */
    explicit GameplayThemeChooser(QWidget *parent = nullptr);
    
    /**
     * @brief Destructor
     * 
     * Cleans up UI resources and destroys all allocated preview widgets
     * and button group child widgets.
     */
    ~GameplayThemeChooser();

    /**
     * @brief Get the currently selected minefield theme option
     * 
     * Returns the name of the minefield theme that is currently selected
     * in the dialog. This represents the user's theme choice but does not
     * apply it until the user confirms or closes the dialog.
     * 
     * @return Name of the selected minefield theme
     * 
     * @see setOptionMinefieldTheme()
     */
    QString optionMinefieldTheme() const;
    
    /**
     * @brief Get the currently selected faces reaction theme option
     * 
     * Returns the name of the faces reaction theme that is currently selected
     * in the dialog. This represents the user's theme choice but does not
     * apply it until the user confirms or closes the dialog.
     * 
     * @return Name of the selected faces reaction theme
     * 
     * @see setOptionFacesReaction()
     */
    QString optionFacesReaction() const;

    /**
     * @brief Set the selected minefield theme option
     * 
     * Programmatically selects a minefield theme by name. Updates the
     * radio button state to reflect the specified theme option.
     * 
     * @param option Name of the minefield theme to select
     * 
     * @see optionMinefieldTheme()
     */
    void setOptionMinefieldTheme(const QString& option);
    
    /**
     * @brief Set the selected faces reaction theme option
     * 
     * Programmatically selects a faces reaction theme by name. Updates the
     * radio button state to reflect the specified theme option.
     * 
     * @param option Name of the faces reaction theme to select
     * 
     * @see optionFacesReaction()
     */
    void setOptionFacesReaction(const QString& option);

protected:
    /**
     * @brief Handle dialog close event
     * 
     * Called when the user attempts to close the dialog window.
     * Emits SIGNAL_visibilityChanged to notify the main application
     * that the dialog is being hidden.
     * 
     * @param e Pointer to the close event
     */
    void closeEvent(QCloseEvent* e);
    
    /**
     * @brief Handle dialog hide event
     * 
     * Called when the dialog is hidden. Emits SIGNAL_visibilityChanged
     * signal to notify the main application.
     * 
     * @param e Pointer to the hide event
     */
    void hideEvent(QHideEvent* e);
    
    /**
     * @brief Handle dialog show event
     * 
     * Called when the dialog is shown. Emits SIGNAL_visibilityChanged
     * signal to notify the main application.
     * 
     * @param e Pointer to the show event
     */
    void showEvent(QShowEvent* e);

private:
    /**
     * @brief Add a minefield theme to the dialog with preview
     * 
     * Creates a radio button and preview widget for a specific minefield theme,
     * adds them to the dialog layout, and registers them with the minefield
     * button group for selection management.
     * 
     * @param themeName Name of the minefield theme to add
     * 
     * @see vAddFacesReactionTheme()
     */
    void vAddMinefieldTheme(const QString& themeName);
    
    /**
     * @brief Add a faces reaction theme to the dialog with preview
     * 
     * Creates a radio button and preview widget for a specific faces reaction
     * theme, adds them to the dialog layout, and registers them with the
     * faces reaction button group for selection management.
     * 
     * @param themeName Name of the faces reaction theme to add
     * 
     * @see vAddMinefieldTheme()
     */
    void vAddFacesReactionTheme(const QString& themeName);
    
    /**
     * @brief Add a visual separator line to a form layout
     * 
     * Adds a horizontal line separator to improve visual organization of
     * theme categories within the dialog layout.
     * 
     * @param layout Pointer to the form layout where separator will be added
     */
    static void vAddLineSeparatorOnLayout(QFormLayout* layout);

    Ui::GameplayThemeChooser *ui;                                           ///< UI definition for the theme chooser dialog
    QList<QPair<QRadioButton*, MinefieldThemePreview*>> listMinefieldPreview; ///< Pairs of radio buttons and minefield theme previews
    QButtonGroup* btngMinefield;                                            ///< Button group managing minefield theme selection

    QList<QPair<QRadioButton*, FacesReactionPreview*>> listFacesReactionPreview; ///< Pairs of radio buttons and faces reaction previews
    QButtonGroup* btngFacesReaction;                                        ///< Button group managing faces reaction selection

    int iFacesReactionPreviewLength;                                        ///< Size in pixels for faces reaction preview displays
    int iMinefieldPreviewLength;                                            ///< Size in pixels for minefield theme preview displays

Q_SIGNALS:
    /**
     * @brief Emitted when the dialog's visibility state changes
     * 
     * This signal is emitted whenever the dialog is shown or hidden,
     * allowing the main application to respond to visibility changes
     * (e.g., to pause/resume gameplay or update UI accordingly).
     * 
     * @param visible true if dialog is being shown, false if being hidden
     */
    void SIGNAL_visibilityChanged(const bool visible);

};

#endif // GAMEPLAYTHEMECHOOSER_H
