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
 * @file gameboardrandomgenerator.h
 * @brief Random game board generation utilities for LibreMines
 * @author Bruno Bollos Correa
 * @date 2025-09-27
 * 
 * This file contains the GameBoardRandomGenerator class responsible for
 * generating random minesweeper game boards with proper mine placement
 * and neighbor counting. It handles both standard random generation and
 * regeneration scenarios where specific cells need to remain mine-free.
 */

#ifndef GAMEBOARDRANDOMGENERATOR_H
#define GAMEBOARDRANDOMGENERATOR_H

#include <vector>
#include "common.h"
#include "libreminesgameengine.h"

/**
 * @brief Utility class for generating random minesweeper game boards
 * 
 * GameBoardRandomGenerator provides static methods for creating randomized
 * minesweeper game boards with proper mine placement and adjacent mine
 * counting. It supports both initial generation and regeneration scenarios
 * where certain cells must be kept mine-free (e.g., for first-click safety).
 * 
 * The generator handles all aspects of board creation including:
 * - Random mine placement with collision avoidance
 * - Clean cell protection (keeping specified areas mine-free)
 * - Adjacent mine counting for numbered cells
 * - Edge case handling for board boundaries
 * 
 * @since v3.0
 */
class GameBoardRandomGenerator
{
public:
    /**
     * @brief Generate a random minesweeper game board
     * 
     * Creates a new random minesweeper board with the specified dimensions
     * and mine count. Optionally ensures that a specific cell and its
     * neighbors remain mine-free for safe first-click scenarios.
     * 
     * The generation process:
     * 1. Creates an empty board matrix
     * 2. Randomly places mines while avoiding clean areas
     * 3. Calculates adjacent mine counts for all non-mine cells
     * 4. Returns the completed board with statistics
     * 
     * @param lines Height of the game board (number of rows)
     * @param rows Width of the game board (number of columns)
     * @param nMines Total number of mines to place on the board
     * @param xClean X coordinate of cell to keep mine-free (255 = none)
     * @param yClean Y coordinate of cell to keep mine-free (255 = none)
     * @return GeneratedBoard containing the complete game board and metadata
     * 
     * @note If xClean and yClean are provided, that cell and its 8 neighbors
     *       will be guaranteed to be mine-free for safe first click
     * @note The function handles edge cases where the clean area overlaps
     *       board boundaries appropriately
     */
    static std::vector<std::vector<LibreMinesGameEngine::CellGameEngine>> vGenerate(
        const uchar lines,
        const uchar rows,
        short nMines,
        const uchar xClean = 255,
        const uchar yClean = 255);

private:
    /**
     * @brief Calculate adjacent mine count for a specific cell
     * 
     * Counts the number of mines in the 8 cells surrounding the specified
     * position. Handles boundary conditions appropriately for cells on
     * edges and corners of the game board.
     * 
     * @param matrix The game board matrix to examine
     * @param x X coordinate of the cell to calculate for
     * @param y Y coordinate of the cell to calculate for
     * @param maxX Maximum X coordinate (board width - 1)
     * @param maxY Maximum Y coordinate (board height - 1)
     * @return Number of adjacent mines (0-8)
     */
    static uchar countAdjacentMines(const std::vector<std::vector<LibreMinesGameEngine::CellGameEngine>>& matrix,
                                    uchar x, uchar y, uchar maxX, uchar maxY);
};

#endif // GAMEBOARDRANDOMGENERATOR_H
