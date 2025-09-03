/*****************************************************************************
 * LibreMines                                                                *
 * Copyright (C) 2020-2025  Bruno Bollos Correa                              *
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
 * @file qkeyinput.h
 * @brief Key input widget for keyboard shortcut configuration
 * @author Bruno Bollos Correa
 * @date 2020-2025
 * 
 * This file defines QKeyInput, a specialized QLineEdit widget that allows
 * users to configure keyboard shortcuts by capturing key presses and
 * displaying them in a user-friendly format. Used primarily in preferences
 * dialogs for customizing keyboard controls.
 */

#ifndef QKEYINPUT_H
#define QKEYINPUT_H

#include <QLineEdit>
#include <QObject>

/**
 * @brief Specialized line edit widget for capturing keyboard input
 * 
 * QKeyInput extends QLineEdit to create an input field specifically designed
 * for capturing and configuring keyboard shortcuts. When the widget has focus,
 * it captures key press events and displays the pressed key in a human-readable
 * format, making it ideal for keyboard shortcut configuration in preferences.
 * 
 * The widget provides validation to ensure that captured keys are suitable
 * for use as shortcuts and offers utility functions for converting between
 * key codes and display text.
 * 
 * @since 1.0
 */
class QKeyInput : public QLineEdit
{
    Q_OBJECT
public:
    /**
     * @brief Constructor for the key input widget
     * 
     * Creates a QKeyInput widget with the specified parent. The widget
     * is initialized with no key assigned and ready to capture input.
     * 
     * @param parent Parent widget for Qt's parent-child memory management
     */
    QKeyInput(QWidget* parent = nullptr);

    /**
     * @brief Get the currently assigned key
     * 
     * Returns the Qt key code of the key currently assigned to this
     * input widget.
     * 
     * @return Qt key code of the current key, or 0 if no key is assigned
     */
    int currentKey()const;

    /**
     * @brief Set the key for this input widget
     * 
     * Programmatically assigns a key to this widget and updates the
     * display text accordingly. Performs validation to ensure the
     * key is suitable for use as a shortcut.
     * 
     * @param k Qt key code to assign to this widget
     * @return true if the key was successfully set, false if invalid
     */
    bool setKey(const int k);

    /**
     * @brief Check if the current key assignment is valid
     * 
     * Validates that the currently assigned key is suitable for use
     * as a keyboard shortcut (not a modifier-only key, etc.).
     * 
     * @return true if the current key is valid, false otherwise
     */
    bool isValid()const;

    /**
     * @brief Convert a key code to human-readable text
     * 
     * Static utility function that converts Qt key codes into
     * user-friendly text representations for display purposes.
     * 
     * @param k Qt key code to convert
     * @return QString containing the human-readable key name
     */
    static QString getTextFromKey(const int k);

protected:
    /**
     * @brief Handle key press events for shortcut capture
     * 
     * Overrides the default key press handling to capture keyboard
     * input for shortcut assignment. Updates the widget's display
     * and stored key value when a valid key is pressed.
     * 
     * @param e Key event containing the pressed key information
     */
    void keyPressEvent(QKeyEvent* e);

private:
    int key; ///< Currently assigned Qt key code
};

#endif // QKEYINPUT_H
