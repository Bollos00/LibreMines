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
 * @file qlabel_adapted.h
 * @brief Custom QLabel widget for LibreMines minefield cells
 * @author Bruno Bollos Correa
 * @date 2020-12-12
 * 
 * This file defines QLabel_adapted, a specialized QLabel widget that
 * represents revealed cells in the minesweeper grid. It extends QLabel
 * with cell position tracking and enhanced mouse event handling for
 * game interaction.
 */

#ifndef QLABEL_ADAPTED_H
#define QLABEL_ADAPTED_H

#include <QLabel>

/**
 * @brief Custom QLabel widget for revealed minesweeper cells
 * 
 * QLabel_adapted extends QLabel to create interactive labels that represent
 * revealed cells in the minesweeper grid. Each label knows its position in
 * the grid and provides enhanced mouse event handling for user interactions
 * such as chord clicking (revealing neighboring cells).
 * 
 * The widget maintains its grid coordinates and emits custom signals when
 * mouse events occur, allowing the game engine to respond appropriately
 * to user interactions with revealed cells.
 * 
 * @since v1.0
 */
class QLabel_adapted : public QLabel
{
    Q_OBJECT

public:
    /**
     * @brief Constructor for the adapted label widget
     * 
     * Creates a QLabel_adapted with the specified parent and grid coordinates.
     * The coordinates are stored and used for game logic when the label
     * is interacted with.
     * 
     * @param parent Parent widget for Qt's parent-child memory management
     * @param x X coordinate (column) of this cell in the game grid (255 = unset)
     * @param y Y coordinate (row) of this cell in the game grid (255 = unset)
     */
    explicit QLabel_adapted(QWidget* parent = nullptr, const uchar x = 255, const uchar y = 255);
    
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

    /**
     * @brief Set pixmap with pre-cached inverted version for performance
     * 
     * This method pre-computes the inverted version of the pixmap to avoid
     * expensive real-time image processing during mouse events. The inverted
     * version is cached and used for visual feedback during user interactions.
     * 
     * @param pixmap The normal pixmap to display and cache
     */
    void setPixmapCached(const QPixmap& pixmap);

protected:
    /**
     * @brief Handle mouse release events
     * 
     * Called when the mouse button is released over this label.
     * Emits the SIGNAL_released signal with the mouse event details.
     * 
     * @param e Mouse event containing button and position information
     */
    void mouseReleaseEvent(QMouseEvent* e);
    
    /**
     * @brief Handle mouse press events
     * 
     * Called when the mouse button is pressed over this label.
     * Emits the SIGNAL_clicked signal with the mouse event details.
     * 
     * @param e Mouse event containing button and position information
     */
    void mousePressEvent(QMouseEvent* e);

private:
    const uchar xCell; ///< X coordinate (column) of this cell in the game grid
    const uchar yCell; ///< Y coordinate (row) of this cell in the game grid

    QPixmap normalPixmap;    ///< Normal pixmap state for the label
    QPixmap invertedPixmap;  ///< Pre-computed inverted pixmap for visual feedback
    bool isInverted = false; ///< Current state tracking for proper pixmap management

Q_SIGNALS:
    /**
     * @brief Signal emitted when mouse button is released over this label
     * 
     * Emitted when the user releases a mouse button over this revealed cell.
     * This is typically used for chord clicking operations where the user
     * clicks on a revealed number to clear its neighbors.
     * 
     * @param e Const pointer to the mouse event with button and position details
     */
    void SIGNAL_released(const QMouseEvent *const e);
    
    /**
     * @brief Signal emitted when mouse button is pressed over this label
     * 
     * Emitted when the user presses a mouse button over this revealed cell.
     * Used for providing immediate visual feedback during interaction.
     * 
     * @param e Const pointer to the mouse event with button and position details
     */
    void SIGNAL_clicked(const QMouseEvent *const e);
};

#endif // QLABEL_ADAPTED_H
