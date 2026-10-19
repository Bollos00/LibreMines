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

#include "solversolveharness.h"

SolverSolveHarness::SolverSolveHarness() :
    solver()
{
}

bool SolverSolveHarness::solve(const std::vector<std::vector<LibreMinesGameEngine::CellGameEngine>>& board,
                               uchar cols,
                               uchar rows,
                               short nMines,
                               uchar safeX,
                               uchar safeY,
                               LibreMinesSolver::SolverStrategy strategies,
                               int maxEnumerationClues)
{
    // Mirror the state reset performed by LibreMinesSolver::run() before solve().
    solver.board = board;
    solver.cols = cols;
    solver.rows = rows;
    solver.nMines = nMines;
    solver.safeX = safeX;
    solver.safeY = safeY;
    solver.strategies = strategies;
    solver.setMaxEnumerationClues(maxEnumerationClues);

    solver.solverBoard.assign(
        cols,
        std::vector<LibreMinesSolver::SolverCellState>(
            rows, LibreMinesSolver::SolverCellState::Unknown));
    solver.cellsNeedingTreatment.clear();
    solver.nCellsMarkedSafe = 0;
    solver.nCellsMarkedMine = 0;
    solver.strategyCounters.clear();

    return solver.solve();
}

QMap<LibreMinesSolver::SolverStrategy, quint64> SolverSolveHarness::strategyCounters() const
{
    return solver.strategyCounters;
}
