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
 * @file solversolveharness.h
 * @brief Solves a single supplied board through the private solver logic
 * @author Bruno Bollos Correa
 * @date 2026-10-18
 *
 * LibreMinesSolver::run() repeatedly generates boards until it finds a
 * solvable one, so it cannot be used to measure how often boards are
 * solvable. This harness is a friend of LibreMinesSolver and therefore can
 * reset the solver state and call the private solve() once per board.
 *
 * One harness owns one LibreMinesSolver and is meant to be used by a single
 * worker thread.
 *
 * @since v3.0.0
 */

#ifndef SOLVERSOLVEHARNESS_H
#define SOLVERSOLVEHARNESS_H

#include "libreminessolver.h"
#include "libreminesgameengine.h"

#include <QMap>
#include <vector>

/**
 * @brief Single-board solving harness used by the statistics tools
 *
 * @since v3.0.0
 */
class SolverSolveHarness
{
public:
    /**
     * @brief Construct a harness with its own solver instance
     */
    SolverSolveHarness();

    /**
     * @brief Solve one externally supplied board
     *
     * Replicates the per-board reset done by LibreMinesSolver::run() before
     * calling solve(): installs the board, recreates the solver board, clears
     * the treatment set and the marked-cell counters, and clears the strategy
     * counters.
     *
     * @param board Game board with actual mine locations
     * @param cols Board width (number of columns)
     * @param rows Board height (number of rows)
     * @param nMines Total number of mines
     * @param safeX X coordinate of the guaranteed safe starting cell
     * @param safeY Y coordinate of the guaranteed safe starting cell
     * @param strategies Strategy flags to enable
     * @param maxEnumerationClues Clue limit of the enumeration strategy (0 = unlimited)
     * @return true if the board was solved without guessing
     */
    bool solve(const std::vector<std::vector<LibreMinesGameEngine::CellGameEngine>>& board,
               uchar cols,
               uchar rows,
               short nMines,
               uchar safeX,
               uchar safeY,
               LibreMinesSolver::SolverStrategy strategies,
               int maxEnumerationClues);

    /**
     * @brief How many times each strategy was applied during the last solve()
     * @return Copy of the solver strategy counters
     */
    QMap<LibreMinesSolver::SolverStrategy, quint64> strategyCounters() const;

private:
    LibreMinesSolver solver;    ///< Solver instance owned by this harness
};

#endif // SOLVERSOLVEHARNESS_H
