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
 * @file common.h
 * @brief Common definitions and enumerations for LibreMines
 * @author Bruno Bollos Correa
 * @date 2020-2025
 * 
 * This file contains fundamental data types and enumerations used throughout
 * the LibreMines application. It defines the core game concepts including
 * cell values, game difficulty levels, and flag states that are essential
 * for the minesweeper game logic.
 */

#ifndef COMMON_H
#define COMMON_H

#include <QtWidgets>

/**
 * @brief Cell values in the minesweeper grid
 * 
 * Represents the possible values that a cell can contain in the minesweeper
 * game. Values from ZERO to EIGHT indicate the number of adjacent mines,
 * while MINE indicates the cell contains a mine.
 * 
 * @since 1.0
 */
enum class CellValue: qint8{
    MINE = -1,    ///< Cell contains a mine
    ZERO = 0,     ///< Cell has no adjacent mines
    ONE = 1,      ///< Cell has one adjacent mine
    TWO = 2,      ///< Cell has two adjacent mines
    THREE = 3,    ///< Cell has three adjacent mines
    FOUR = 4,     ///< Cell has four adjacent mines
    FIVE = 5,     ///< Cell has five adjacent mines
    SIX = 6,      ///< Cell has six adjacent mines
    SEVEN = 7,    ///< Cell has seven adjacent mines
    EIGHT = 8     ///< Cell has eight adjacent mines
};

/**
 * @brief Game difficulty levels
 * 
 * Defines the predefined difficulty levels available in LibreMines.
 * Each level corresponds to specific board dimensions and mine counts.
 * CUSTOMIZED allows players to create their own game parameters.
 * 
 * @since 1.0
 */
enum class GameDifficulty: uchar{
    NONE,         ///< No difficulty selected (initial state)
    EASY,         ///< Easy difficulty (9x9 board, 10 mines)
    MEDIUM,       ///< Medium difficulty (16x16 board, 40 mines)
    HARD,         ///< Hard difficulty (30x16 board, 99 mines)
    CUSTOMIZED    ///< Custom difficulty with user-defined parameters
};

/**
 * @brief Flag states for minesweeper cells
 * 
 * Represents the possible flag states that can be applied to a cell
 * by the player. Used for marking suspected mines or uncertain cells.
 * 
 * @since 1.0
 */
enum class FlagState : quint8{
    NoFlag   = 0, ///< Cell has no flag
    HasFlag  = 1, ///< Cell is flagged as a suspected mine
    Question = 2  ///< Cell is marked with a question mark (uncertain)
};


#endif // COMMON_H
