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
 *****************************************************************************/

/**
 * @file deterministicboardgenerator.h
 * @brief Reproducible board generation for the solver statistics tools
 * @author Bruno Bollos Correa
 * @date 2026-10-18
 *
 * GameBoardRandomGenerator::vGenerate() relies on the process-wide
 * QRandomGenerator, which cannot be reseeded and is therefore unsuitable for
 * reproducible, per-task generation across threads. This helper mirrors the
 * exact algorithm of vGenerate() (uniform mine placement with rejection of
 * the clean area and of already placed mines, followed by adjacent-mine
 * counting) but draws its randomness from a caller-owned std::mt19937.
 *
 * @since v3.0.0
 */

#ifndef DETERMINISTICBOARDGENERATOR_H
#define DETERMINISTICBOARDGENERATOR_H

#include "libreminesgameengine.h"

#include <random>
#include <vector>

/**
 * @brief Deterministic minesweeper board generator
 *
 * Every worker thread keeps its own std::mt19937 seeded from a value derived
 * from the tool's master seed, so a run with a fixed seed always produces the
 * same numbers no matter how the work is scheduled.
 *
 * @since v3.0.0
 * @see GameBoardRandomGenerator::vGenerate()
 */
class DeterministicBoardGenerator
{
public:
    /**
     * @brief Generate one board, mirroring GameBoardRandomGenerator::vGenerate()
     *
     * @param rng Random engine owned by the caller (per worker / per task)
     * @param cols Width of the game board (number of columns)
     * @param rows Height of the game board (number of rows)
     * @param nMines Total number of mines to place
     * @param xClean X coordinate of the cell kept mine-free (255 = none)
     * @param yClean Y coordinate of the cell kept mine-free (255 = none)
     * @return Matrix indexed first by column and then by row
     */
    static std::vector<std::vector<LibreMinesGameEngine::CellGameEngine>>
    generate(std::mt19937& rng,
             uchar cols,
             uchar rows,
             short nMines,
             uchar xClean = 255,
             uchar yClean = 255);

private:
    /**
     * @brief Count the mines around a cell
     *
     * @param matrix Board to inspect
     * @param x X coordinate of the cell
     * @param y Y coordinate of the cell
     * @param maxX Highest valid X coordinate
     * @param maxY Highest valid Y coordinate
     * @return Number of neighbouring mines (0-8)
     */
    static uchar countAdjacentMines(
        const std::vector<std::vector<LibreMinesGameEngine::CellGameEngine>>& matrix,
        uchar x, uchar y, uchar maxX, uchar maxY);
};

#endif // DETERMINISTICBOARDGENERATOR_H
