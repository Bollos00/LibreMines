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
 *****************************************************************************/

/**
 * @file libreminessolver.h
 * @brief Intelligent minesweeper board solver for generating solvable puzzles
 * @author Bruno Bollos Correa
 * @date 2025-09-27
 * 
 * This file contains the LibreMinesSolver class which implements an automated
 * minesweeper solving engine. The solver generates random boards and validates
 * their solvability using logical deduction algorithms. It employs basic
 * minesweeper solving strategies including zero-clearing and constraint
 * satisfaction to ensure generated puzzles are solvable without guessing.
 * 
 * The solver operates in a separate thread to avoid blocking the main
 * application and provides thread-safe state management and interruption
 * capabilities. It is primarily used for the "No Guess" game mode to
 * guarantee puzzle solvability.
 */

#ifndef LIBREMINESSOLVER_H
#define LIBREMINESSOLVER_H

#include "libreminesgameengine.h"

#include <QThread>
#include <QMutex>
#include <QSet>

/**
 * @brief Intelligent minesweeper board solver and generator
 * 
 * LibreMinesSolver is a threaded minesweeper solving engine that generates
 * random boards and validates their solvability using logical deduction.
 * It implements fundamental minesweeper solving algorithms to ensure that
 * generated puzzles can be solved without guessing.
 * 
 * The solver operates through multiple phases:
 * 1. Initial clearing: Reveals safe cells around the starting position
 * 2. Basic constraint satisfaction: Applies logical rules to identify mines and safe cells
 * 3. Iterative processing: Continues until no more progress can be made
 * 
 * Key features:
 * - Thread-safe operation with state management
 * - Interruptible solving process
 * - Debug visualization of solving progress
 * - Support for custom board dimensions and mine counts
 * - Guaranteed solvable board generation for "No Guess" mode
 * 
 * The solver maintains two parallel boards:
 * - Original game board with actual mine locations
 * - Solver state board tracking discovered safe cells and mines
 * 
 * @since v3.0.0
 */
class LibreMinesSolver : public QThread
{
    Q_OBJECT
public:

    /**
     * @brief Enumeration of possible solver states during execution
     * 
     * Represents the current lifecycle state of the solver thread,
     * from initialization through completion or termination.
     * 
     * @since v3.0.0
     */
    enum class SolverState : quint8 {
        NotStarted  = 0,  ///< Solver has been created but not yet started
        Running     = 1,  ///< Solver is currently executing and generating boards
        Finished    = 2,  ///< Solver has completed successfully and found a solvable board
        Interrupted = 3,  ///< Solver was interrupted before completion
        Failed      = 4   ///< Solver encountered an error or was terminated
    };

    /**
     * @brief Commands for controlling solver execution
     * 
     * Provides a thread-safe mechanism for external control of the solver
     * thread, allowing graceful interruption of long-running operations.
     * 
     * @since v3.0.0
     */
    enum class SolverCommand : quint8 {
        CmdNone        = 0,  ///< No command issued - solver continues normal operation
        CmdInterrupt   = 1   ///< Command to interrupt the solver and terminate gracefully
    };

    /**
     * @brief Cell states during solving process
     * 
     * Represents the solver's knowledge about each cell in the board.
     * These states are determined through logical deduction and updated
     * iteratively as more information becomes available.
     * 
     * @since v3.0.0
     */
    enum class SolverCellState : quint8 {
        Unknown = 0,    ///< Cell state is undetermined - requires further analysis
        Safe    = 1,    ///< Cell is confirmed safe through logical deduction
        Mine    = 2     ///< Cell is confirmed to contain a mine through logical deduction
    };

    /**
     * @brief Constructor for the minesweeper solver
     * 
     * Initializes the solver with default state and prepares thread
     * infrastructure for solving operations.
     * 
     * @param parent Parent QObject for memory management
     */
    explicit LibreMinesSolver(QObject *parent = nullptr);

    /**
     * @brief Get the number of rows in the board
     * @return Number of rows
     */
    uchar getRows() const;
    
    /**
     * @brief Set the number of rows for board generation
     * @param newRows Number of rows (must be > 0)
     */
    void setRows(uchar newRows);

    /**
     * @brief Get the number of columns in the board
     * @return Number of columns
     */
    uchar getCols() const;
    
    /**
     * @brief Set the number of columns for board generation
     * @param newCols Number of columns (must be > 0)
     */
    void setCols(uchar newCols);

    /**
     * @brief Get the X coordinate of the guaranteed safe starting cell
     * @return X coordinate of safe cell
     */
    uchar getSafeX() const;
    
    /**
     * @brief Set the X coordinate of the guaranteed safe starting cell
     * @param newSafeX X coordinate (must be < rows)
     */
    void setSafeX(uchar newSafeX);

    /**
     * @brief Get the Y coordinate of the guaranteed safe starting cell
     * @return Y coordinate of safe cell
     */
    uchar getSafeY() const;
    
    /**
     * @brief Set the Y coordinate of the guaranteed safe starting cell
     * @param newSafeY Y coordinate (must be < cols)
     */
    void setSafeY(uchar newSafeY);

    /**
     * @brief Get the total number of mines in the board
     * @return Number of mines
     */
    short getNMines() const;
    
    /**
     * @brief Set the total number of mines for board generation
     * @param newNMines Number of mines (must be > 0 and < total cells)
     */
    void setNMines(short newNMines);

    /**
     * @brief Get the current state of the solver thread
     * 
     * Thread-safe method to query solver execution status.
     * 
     * @return Current solver state
     * @see SolverState
     */
    SolverState getSolverState() const;
    
    /**
     * @brief Request interruption of the solving process
     * 
     * Thread-safe method to gracefully stop the solver. The solver
     * will check for interruption at safe points and terminate cleanly.
     */
    void interrupt();

    /**
     * @brief Get the generated solvable board
     * 
     * Returns the final generated board that passed solvability validation.
     * Should only be called after solver has reached Finished state.
     * 
     * @return 2D vector containing the complete game board
     */
    std::vector<std::vector<LibreMinesGameEngine::CellGameEngine> > getBoard() const;

protected:
    /**
     * @brief Main thread execution method
     * 
     * Implements the primary solving loop that generates random boards
     * and validates their solvability. Continues until a solvable board
     * is found or the process is interrupted.
     */
    void run() override;

private:
    std::vector<std::vector<LibreMinesGameEngine::CellGameEngine>> board;  ///< Generated game board with actual mine locations
    std::vector<std::vector<SolverCellState>> solverBoard;  ///< Parallel board tracking solver's knowledge of each cell
    QSet<QPair<uchar, uchar>> cellsNeedingTreatment;  ///< Set of safe cells requiring further constraint analysis
    
    uchar rows;     ///< Number of rows in the board
    uchar cols;     ///< Number of columns in the board
    short nMines;   ///< Total number of mines to place

    uchar safeX;    ///< X coordinate of guaranteed safe starting cell
    uchar safeY;    ///< Y coordinate of guaranteed safe starting cell
    
    SolverState state;              ///< Current state of the solver thread
    mutable QMutex stateMutex;      ///< Mutex for thread-safe state access

    SolverCommand cmd;              ///< Current command for controlling solver execution
    mutable QMutex cmdMutex;        ///< Mutex for thread-safe command access

    /**
     * @brief Core solving algorithm implementation
     * 
     * Applies logical deduction rules to determine if the current board
     * is solvable without guessing. Uses basic minesweeper solving
     * strategies including constraint satisfaction.
     * 
     * @return true if board is solvable, false otherwise
     */
    bool solve();

    /**
     * @brief Initialize solving by clearing the safe starting cell
     * 
     * Performs the initial step of revealing the guaranteed safe cell
     * and its neighbors, establishing the foundation for further solving.
     */
    void initialStep();

    /**
     * @brief Clear zero-value cells and their safe neighbors
     * 
     * Recursively clears cells with zero adjacent mines and marks
     * their neighbors as safe. This implements the fundamental
     * minesweeper clearing rule.
     * 
     * @param x X coordinate of the zero cell
     * @param y Y coordinate of the zero cell
     * @return true if any changes were made to the solver board
     */
    bool cleanZero(uchar x, uchar y);
    
    /**
     * @brief Apply basic constraint satisfaction to a cell
     * 
     * Analyzes a numbered cell and its neighbors to identify mines
     * and safe cells using logical deduction. Implements two key rules:
     * 1. If all required mines are found, remaining neighbors are safe
     * 2. If unknown neighbors equal remaining mines, all are mines
     * 
     * @param x X coordinate of the cell to analyze
     * @param y Y coordinate of the cell to analyze
     * @return true if any new deductions were made
     */
    bool cleanBasic(uchar x, uchar y);

signals:
};

#endif // LIBREMINESSOLVER_H
