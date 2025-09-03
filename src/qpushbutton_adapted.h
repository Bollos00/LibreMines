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
 * @file qpushbutton_adapted.h
 * @brief Custom QPushButton widget for LibreMines minefield cells
 * @author Bruno Bollos Correa
 * @date 2020-2025
 * 
 * This file defines QPushButton_adapted, a specialized QPushButton widget
 * that represents hidden cells in the minesweeper grid. It extends QPushButton
 * with cell position tracking and enhanced mouse event handling for primary
 * game interactions like revealing cells and placing flags.
 */

#ifndef QPUSHBUTTON_ADAPTED_H
#define QPUSHBUTTON_ADAPTED_H

#include <QPushButton>

/**
 * @brief Custom QPushButton widget for hidden minesweeper cells
 * 
 * QPushButton_adapted extends QPushButton to create interactive buttons that
 * represent hidden cells in the minesweeper grid. Each button knows its
 * position in the grid and provides enhanced mouse event handling for the
 * primary game interactions: revealing cells (left click) and placing flags
 * (right click).
 * 
 * The widget maintains its grid coordinates and emits custom signals when
 * mouse events occur, allowing the game engine to respond appropriately
 * to user interactions with hidden cells.
 * 
 * @since 1.0
 */
class QPushButton_adapted : public QPushButton
{
    Q_OBJECT

public:
    /**
     * @brief Constructor for the adapted push button widget
     * 
     * Creates a QPushButton_adapted with the specified parent and grid
     * coordinates. The coordinates are stored and used for game logic
     * when the button is interacted with.
     * 
     * @param parent Parent widget for Qt's parent-child memory management
     * @param x X coordinate (column) of this cell in the game grid (255 = unset)
     * @param y Y coordinate (row) of this cell in the game grid (255 = unset)
     */
    explicit QPushButton_adapted(QWidget *parent = nullptr, const uchar x = 255, const uchar y = 255);
    
    /**
     * @brief Get the X coordinate of this cell
     * 
     * @return X coordinate (column) of this cell in the game grid
     */
    uchar getXCell();
    
    /**
     * @brief Get the Y coordinate of this cell
     * 
     * @return Y coordinate (row) of this cell in the game grid
     */
    uchar getYCell();

protected:
    /**
     * @brief Handle mouse release events
     * 
     * Called when the mouse button is released over this button.
     * Emits the SIGNAL_released signal with the mouse event details.
     * Different mouse buttons trigger different game actions.
     * 
     * @param e Mouse event containing button and position information
     */
    void mouseReleaseEvent(QMouseEvent* e);
    
    /**
     * @brief Handle mouse press events
     * 
     * Called when the mouse button is pressed over this button.
     * Emits the SIGNAL_clicked signal with the mouse event details.
     * Used for providing immediate visual feedback during interaction.
     * 
     * @param e Mouse event containing button and position information
     */
    void mousePressEvent(QMouseEvent* e);

private:
    const uchar xCell; ///< X coordinate (column) of this cell in the game grid
    const uchar yCell; ///< Y coordinate (row) of this cell in the game grid

Q_SIGNALS:
    /**
     * @brief Signal emitted when mouse button is released over this button
     * 
     * Emitted when the user releases a mouse button over this hidden cell.
     * Left button typically reveals the cell, right button places/removes flags,
     * and middle button may trigger special actions.
     * 
     * @param event Const pointer to the mouse event with button and position details
     */
    void SIGNAL_released(const QMouseEvent*const event);
    
    /**
     * @brief Signal emitted when mouse button is pressed over this button
     * 
     * Emitted when the user presses a mouse button over this hidden cell.
     * Used for providing immediate visual feedback during interaction.
     * 
     * @param event Const pointer to the mouse event with button and position details
     */
    void SIGNAL_clicked(const QMouseEvent*const event);

};

#endif // QPUSHBUTTON_ADAPTED_H
