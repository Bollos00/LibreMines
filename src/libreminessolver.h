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
        Failed      = 4   ///< Solver could not complete because of an error
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
     * @brief Bit flags identifying the deduction strategies used by the solver
     *
     * Each concrete detection strategy has its own bit so strategies can be
     * combined in a single value.
     *
     * @since v3.0.0
     */
    enum class SolverStrategy : quint64 {
        None             = 0,              ///< No strategy
        Pattern12X       = 1ULL << 0,      ///< 1-2-X pattern
        Pattern11X       = 1ULL << 1,      ///< 1-1-X pattern
        Pattern13X       = 1ULL << 2,      ///< 1-3-X pattern
        Pattern14X       = 1ULL << 3,      ///< 1-4-X pattern
        Pattern23X       = 1ULL << 4,      ///< 2-3-X pattern
        Pattern24X       = 1ULL << 5,      ///< 2-4-X pattern
        Pattern34X       = 1ULL << 6,      ///< 3-4-X pattern
        Pattern22X       = 1ULL << 7,      ///< 2-2-X pattern
        Pattern33X       = 1ULL << 8,      ///< 3-3-X pattern
        Pattern1X1       = 1ULL << 9,      ///< 1-X-1 pattern
        Pattern2X2       = 1ULL << 10,     ///< 2-X-2 pattern
        Pattern222Corner = 1ULL << 11,    ///< 2-2-2 corner pattern
        Pattern131Corner = 1ULL << 12,    ///< 1-3-1 corner pattern
        PairRule         = 1ULL << 13,      ///< General pair rule
        Enumeration      = 1ULL << 14,     ///< Constraint enumeration
        MineCounting     = 1ULL << 15,     ///< Global mine counter combined with the enumerated layouts

        BasicPatterns     = Pattern12X | Pattern11X | Pattern13X | Pattern14X,  ///< Basic patterns only
        AdvancedPatterns  = Pattern23X | Pattern24X | Pattern34X | Pattern22X
            | Pattern33X | Pattern1X1 | Pattern2X2 | Pattern222Corner | Pattern131Corner,  ///< Advanced patterns only
        BasicAndAdvancedPatterns = BasicPatterns | AdvancedPatterns,  ///< Basic and advanced patterns combined
        AllStrategies     = BasicAndAdvancedPatterns | PairRule | Enumeration | MineCounting  ///< All strategies enabled
    };

    /**
     * @brief Get the enabled solver strategies
     * @return Bitwise combination of enabled strategies
     */
    SolverStrategy getStrategies() const;

    /**
     * @brief Set the enabled solver strategies
     * @param newStrategies Bitwise combination of strategies to enable
     */
    void setStrategies(SolverStrategy newStrategies);

    /**
     * @brief Check whether a solver strategy is enabled
     * @param strategy Strategy flag to check
     * @return true if the strategy flag is enabled
     */
    bool isStrategyEnabled(SolverStrategy strategy) const;

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
    * @brief Default value of the enumeration clue limit
    * @see setMaxEnumerationClues()
    */
    static constexpr int DefaultMaxEnumerationClues = 8;

    /**
     * @brief Get the maximum number of clues considered by one enumeration
     * @return Clue limit; 0 means "no limit"
     * @see setMaxEnumerationClues()
     */
    int getMaxEnumerationClues() const;
   
    /**
    * @brief Set the maximum number of clues considered by one enumeration
    *
    * The enumeration tier (see cleanEnumeration()) splits the frontier into
    * groups of connected clues. A group with more clues than this limit is
    * not solved as a whole; instead, windows of at most this many neighbouring
    * clues are solved one by one. A smaller limit is faster but may miss
    * deductions that need many clues at once; a larger limit finds more
    * deductions but searches more. Ignoring clues can only lose deductions,
    * never produce wrong ones.
    *
    * Must be called before the solver thread is started.
    *
    * @param newMaxEnumerationClues Clue limit; 0 disables the limit, negative values are treated as 0
    */
    void setMaxEnumerationClues(int newMaxEnumerationClues);


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
    friend class TestLibreMinesSolver;
    friend class SolverSolveHarness;

    std::vector<std::vector<LibreMinesGameEngine::CellGameEngine>> board;  ///< Generated game board with actual mine locations
    std::vector<std::vector<SolverCellState>> solverBoard;  ///< Parallel board tracking solver's knowledge of each cell
    QSet<QPair<uchar, uchar>> cellsNeedingTreatment;  ///< Set of safe cells requiring further constraint analysis
    
    uchar cols;     ///< Number of columns in the board
    uchar rows;     ///< Number of rows in the board
    short nMines;   ///< Total number of mines to place

    uchar safeX;    ///< X coordinate of guaranteed safe starting cell
    uchar safeY;    ///< Y coordinate of guaranteed safe starting cell

    quint32 nCellsMarkedSafe;  ///< Number of cells currently marked safe on the solver board
    quint32 nCellsMarkedMine;  ///< Number of cells currently marked as mines on the solver board

    SolverState state;              ///< Current state of the solver thread
    mutable QMutex stateMutex;      ///< Mutex for thread-safe state access

    SolverStrategy strategies;  ///< Enabled deduction strategies

    int maxEnumerationClues;  ///< Clue limit of one enumeration window (0 = unlimited)
    
    /**
     * @brief Search node limit for one enumeration
     *
     * Protects the solver against rare, very large constraint problems. When
     * the limit is exceeded the enumeration is discarded, which can only lose
     * deductions and never produce a wrong one.
     */
    static constexpr quint64 EnumerationNodeBudget = 200000;

    QMap<SolverStrategy, quint64> strategyCounters;  ///< Counters for how many times each strategy was applied

    SolverCommand cmd;              ///< Current command for controlling solver execution
    mutable QMutex cmdMutex;        ///< Mutex for thread-safe command access

    /**
    * @brief A revealed number that still has unknown neighbours
    *
    * Describes the constraint "exactly remainingMines of unknownCells are mines".
    */
   struct EnumerationClue
   {
       QVector<int> unknownCells;  ///< Ids (see cellIndex()) of the unknown neighbours
       int remainingMines = 0;     ///< Cell value minus the mines already found around it
   };


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
     * @brief Mark a cell as safe or as a mine
     *
     * For safe cells, first attempts zero expansion; otherwise updates the
     * solver board, treatment set, and marked-cell counters through
     * updateCellState(). Marking a cell already in the requested state does
     * not change counters.
     *
     * @param cell Coordinates of the cell to mark
     * @param newState New solver state; must be Safe or Mine
     */
    void markCell(const QPair<uchar, uchar>& cell, SolverCellState newState);

    /**
     * @brief Mark every still-undetermined cell of a set as safe or as a mine
     *
     * Cells that already have a decided state are skipped, so the call is safe
     * to repeat with overlapping sets. Each remaining cell is handed to
     * markCell(), which also applies zero expansion for safe cells.
     *
     * @param cells Coordinates of the cells to mark
     * @param newState New solver state; must be Safe or Mine
     * @return true if at least one cell changed state
     * @see markCell()
     */
    bool markAllCells(const QSet<QPair<uchar, uchar>>& cells, SolverCellState newState);

    /**
     * @brief Update a cell's solver state, treatment membership, and counter
     *
     * Applies the state update without attempting zero expansion. This is
     * used by cleanZero() for the zero cell currently being expanded.
     *
     * @param cell Coordinates of the cell to update
     * @param newState New solver state; must be Safe or Mine
     */
    void updateCellState(const QPair<uchar, uchar>& cell, SolverCellState newState);

    /**
     * @brief Apply the initial zero-cell expansion
     * 
     * Invokes cleanZero() at the configured safe starting position. If the
     * starting cell has no adjacent mines, it and the connected safe area are
     * marked safe; a numbered starting cell is not changed by this method.
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

    /**
     * @brief Apply basic pattern recognition rules to a cell
     * 
     * Applies the basic pattern deductions supported by the solver, in
     * addition to the direct constraint rules handled by cleanBasic().
     * 
     * Currently supports:
     * - 1-2-X pattern detection and resolution
     * - 1-1-X pattern detection and resolution
     * - 1-3-X pattern detection and resolution
     * - 1-4-X pattern detection and resolution
     * 
     * @param x X coordinate of the cell to analyze
     * @param y Y coordinate of the cell to analyze
     * @return true if any pattern-based deductions were made
     * @see detect12XPattern()
     * @see detect13XPattern()
     * @see detect14XPattern()
     * @see detect11XPattern()
     */
    bool cleanBasicPatterns(uchar x, uchar y);
    
    /**
     * @brief Apply advanced pattern recognition rules to a cell
     * 
     * Tries the 2-3-X, 2-4-X, 3-4-X, 2-2-X, and 3-3-X deductions, the
     * 1-X-1 and 2-X-2 deductions, and the 2-2-2 and 1-3-1 corner deductions.
     * 
     * @param x X coordinate of the cell to analyze
     * @param y Y coordinate of the cell to analyze
     * @return true if any pattern-based deductions were made
     * @see detect23XPattern()
     * @see detect24XPattern()
     * @see detect34XPattern()
     * @see detect22XPattern()
     * @see detect33XPattern()
     * @see detect1X1Pattern()
     * @see detect2X2Pattern()
     * @see detect222CornerPattern()
     * @see detect131CornerPattern()
     */
    bool cleanAdvancedPatterns(uchar x, uchar y);

    /**
     * @brief Apply the general pair rule to a revealed clue
     *
     * Entry point of the SolverStrategy::PairRule tier. The cell must already
     * be marked Safe and the strategy flag must be enabled; the actual search
     * for a partner clue is done by detectPairRule().
     *
     * @param x X coordinate of the clue to analyze
     * @param y Y coordinate of the clue to analyze
     * @return true if the pair rule marked at least one cell
     * @see detectPairRule()
     * @see applyPairRule()
     */
    bool cleanGeneralPairRule(uchar x, uchar y);

    /**
     * @brief Deduce cells by exhaustively enumerating the layouts allowed by the clues
     *
     * Last tier of solve(), used when basic rules and patterns make no progress.
     * Gathers every clue on the board, groups clues that share unknown cells,
     * and for each group enumerates all mine layouts that satisfy every clue
     * (see LibreMinesConstraintEnumerator). A cell that is a mine in all layouts
     * is marked as a mine; a cell that is safe in all layouts is marked safe.
     * Groups with more than maxEnumerationClues clues are solved in windows.
     *
     * This subsumes every pattern based on the numbers around the frontier
     * (1-2-1, 1-3-1, 2-2-2, T-pattern, longer chains, ...).
     *
     * @return true if at least one cell was marked
     * @see setMaxEnumerationClues()
     */
    bool cleanEnumeration();

    /**
     * @brief Deduce cells by combining the enumerated layouts with the global mine counter
     *
     * Last tier of solve(), used when everything else (including enumeration) has
     * stalled. The mine counter says how many mines are still hidden among all
     * unknown cells. Every layout of a group of connected clues has a certain
     * number of mines; layouts whose totals cannot add up to the mines left are
     * impossible, and whatever is the same in all remaining layouts is certain.
     * The unknown cells that touch no clue ("free" cells) may hold any number of
     * mines, so they are what absorbs the difference. Also covers the simple cases
     * (no mines left, or as many mines as unknown cells).
     *
     * Needs every layout of a group, so it enumerates whole groups: the
     * maxEnumerationClues limit does not apply. A group too large for the search
     * budget is skipped and treated as free cells, which only loses deductions.
     * Independent of the Enumeration strategy flag.
     *
     * @return true if at least one cell was marked
     * @see SolverStrategy::MineCounting
     */
    bool cleanMineCounting();

    /**
     * @brief Detect and apply the 1-3-1 corner deduction
     *
     * Searches for a safe cell with three remaining mines and two
     * perpendicular safe neighbors with one remaining mine each. Applies the
     * corner deduction when their unknown-neighbor sets support it.
     *
     * @param x X coordinate of the cell to analyze
     * @param y Y coordinate of the cell to analyze
     * @return true if the corner deduction was applied
     * @see apply131CornerPatternLogic()
     */
    bool detect131CornerPattern(uchar x, uchar y);

    /**
     * @brief Detect and apply the 2-2-2 corner deduction
     *
     * Searches for a safe cell and two perpendicular safe neighbors, each
     * with two remaining mines, and applies the corner deduction when their
     * unknown-neighbor sets support it.
     *
     * @param x X coordinate of the cell to analyze
     * @param y Y coordinate of the cell to analyze
     * @return true if the corner deduction was applied
     * @see apply222CornerPatternLogic()
     */
    bool detect222CornerPattern(uchar x, uchar y);

    /**
     * @brief Detect 1-2-X mine pattern in the vicinity of a cell
     * 
     * Identifies an adjacent cell with 1 remaining mine when this cell has
     * exactly 2 remaining mines, then applies the 1-2-X deduction.
     *
     * The pattern works as follows:
     * - The middle cell has N remaining mines and the adjacent cell has M, where N > M
     * - If the middle-only unknown neighbors number N-M, they are all mines
     * - Unknown neighbors unique to the adjacent cell are safe
     *
     * Searches in all four directions (horizontal and vertical) from the
     * given cell position to find this pattern.
     *
     * @param x X coordinate of the cell to analyze
     * @param y Y coordinate of the cell to analyze
     * @return true if an N-(N-1)-X pattern was detected and processed
     * @see detectMNXPattern()
     */
    bool detect12XPattern(uchar x, uchar y);

    /**
     * @brief Detect 1-1-X mine pattern in the vicinity of a cell
     * 
     * Identifies two adjacent cells each with exactly 1 remaining mine
     * and applies the 1-1-X deduction when they share unknown neighbors.
     *
     * The pattern works as follows:
     * - Cell A has 1 remaining mine among its unknown neighbors
     * - Cell B has 1 remaining mine among its unknown neighbors
     * - If all unknown neighbors of cell B are also neighbors of cell A
     * - Then the unique neighbors of cell A (not shared with B) are safe
     *
     * Searches in all four directions from the given cell position to find this pattern.
     *
     * @param x X coordinate of the cell to analyze
     * @param y Y coordinate of the cell to analyze
     * @return true if a 1-1-X pattern was detected and processed
     * @see applyNNXPatternLogic()
     */
    bool detect11XPattern(uchar x, uchar y);

    /**
     * @brief Detect 2-2-X mine pattern in the vicinity of a cell
     * 
     * Identifies two adjacent cells each with exactly 2 remaining mines
     * and applies the 2-2-X deduction when they share unknown neighbors.
     *
     * The pattern works as follows:
     * - Cell A has 2 remaining mines among its unknown neighbors
     * - Cell B has 2 remaining mines among its unknown neighbors
     * - If all unknown neighbors of cell B are also neighbors of cell A
     * - Then the unique neighbors of cell A (not shared with B) are safe
     *
     * Searches in all four directions from the given cell position to find this pattern.
     *
     * @param x X coordinate of the cell to analyze
     * @param y Y coordinate of the cell to analyze
     * @return true if a 2-2-X pattern was detected and processed
     * @see applyNNXPatternLogic()
     */
    bool detect22XPattern(uchar x, uchar y);

    /**
     * @brief Detect 3-3-X mine pattern in the vicinity of a cell
     * 
     * Identifies two adjacent cells each with exactly 3 remaining mines
     * and applies the 3-3-X deduction when they share unknown neighbors.
     *
     * The pattern works as follows:
     * - Cell A has 3 remaining mines among its unknown neighbors
     * - Cell B has 3 remaining mines among its unknown neighbors
     * - If all unknown neighbors of cell B are also neighbors of cell A
     * - Then the unique neighbors of cell A (not shared with B) are safe
     *
     * Searches in all four directions from the given cell position to find this pattern.
     *
     * @param x X coordinate of the cell to analyze
     * @param y Y coordinate of the cell to analyze
     * @return true if a 3-3-X pattern was detected and processed
     * @see applyNNXPatternLogic()
     */
    bool detect33XPattern(uchar x, uchar y);

    /**
     * @brief Detect the 1-3-X mine pattern in the vicinity of a cell
     *
     * Finds an adjacent cell with 1 remaining mine and applies the deduction
     * when this cell has 3 remaining mines and exactly 2 unique unknown
     * neighbors, with all unknown neighbors of the adjacent cell shared.
     *
     * @param x X coordinate of the cell to analyze
     * @param y Y coordinate of the cell to analyze
     * @return true if the pattern was detected and processed
     */
    bool detect13XPattern(uchar x, uchar y);

    /**
     * @brief Detect the 1-4-X mine pattern in the vicinity of a cell
     *
     * Finds an adjacent cell with 1 remaining mine and applies the deduction
     * when this cell has 4 remaining mines and exactly 3 unique unknown
     * neighbors, with all unknown neighbors of the adjacent cell shared.
     *
     * @param x X coordinate of the cell to analyze
     * @param y Y coordinate of the cell to analyze
     * @return true if the pattern was detected and processed
     */
    bool detect14XPattern(uchar x, uchar y);

    /**
     * @brief Detect the 2-3-X mine pattern in the vicinity of a cell
     *
     * Finds an adjacent cell with 2 remaining mines and applies the deduction
     * when this cell has 3 remaining mines and exactly 2 unique unknown
     * neighbors, with all unknown neighbors of the adjacent cell shared.
     *
     * @param x X coordinate of the cell to analyze
     * @param y Y coordinate of the cell to analyze
     * @return true if the pattern was detected and processed
     */
    bool detect23XPattern(uchar x, uchar y);

    /**
     * @brief Detect the 2-4-X mine pattern in the vicinity of a cell
     *
     * Finds an adjacent cell with 2 remaining mines and applies the deduction
     * when this cell has 4 remaining mines and exactly 3 unique unknown
     * neighbors, with all unknown neighbors of the adjacent cell shared.
     *
     * @param x X coordinate of the cell to analyze
     * @param y Y coordinate of the cell to analyze
     * @return true if the pattern was detected and processed
     */
    bool detect24XPattern(uchar x, uchar y);

    /**
     * @brief Detect the 3-4-X mine pattern in the vicinity of a cell
     *
     * Finds an adjacent cell with 3 remaining mines and applies the deduction
     * when this cell has 4 remaining mines and exactly 3 unique unknown
     * neighbors, with all unknown neighbors of the adjacent cell shared.
     *
     * @param x X coordinate of the cell to analyze
     * @param y Y coordinate of the cell to analyze
     * @return true if the pattern was detected and processed
     */
    bool detect34XPattern(uchar x, uchar y);

    /**
     * @brief Detect the 1-X-1 mine pattern in the vicinity of a cell
     *
     * @param x X coordinate of the cell to analyze
     * @param y Y coordinate of the cell to analyze
     * @return true if the pattern was detected and processed
     */
    bool detect1X1Pattern(uchar x, uchar y);

    /**
     * @brief Detect the 2-X-2 mine pattern in the vicinity of a cell
     *
     * @param x X coordinate of the cell to analyze
     * @param y Y coordinate of the cell to analyze
     * @return true if the pattern was detected and processed
     */
    bool detect2X2Pattern(uchar x, uchar y);

    /**
     * @brief Search for a second clue that can be paired with the given one
     *
     * Two clues can only share unknown neighbours when they are at most two
     * cells apart in both axes, so a 5x5 window around the given clue covers
     * every possible partner. applyPairRule() is tried for each other safe
     * clue; the search stops at the first successful deduction because the
     * clue data is stale afterwards.
     *
     * @param x X coordinate of the clue to analyze
     * @param y Y coordinate of the clue to analyze
     * @return true if a pair rule deduction was applied
     * @see applyPairRule()
     */
    bool detectPairRule(uchar x, uchar y);

    /**
     * @brief Detect a M-N-X mine pattern in the vicinity of a cell
     *
     * Checks the four orthogonally adjacent cells for one with M remaining
     * mines, then applies the M-N-X deduction for N in the range 2 through 4.
     * 
     * This method is a generalized implementation of the 1-2-X, 1-3-X, 1-4-X, 2-3-X, 2-4-X, and 3-4-X patterns.
     *
     * @param x X coordinate of the cell to analyze
     * @param y Y coordinate of the cell to analyze
     * @param adjacentMines Remaining mines required by the adjacent cell (M)
     * @param remainingMines Remaining mines required by the analyzed cell (N)
     * @return true if the pattern was detected and processed
     */
    bool detectMNXPattern(uchar x, uchar y, int adjacentMines, int remainingMines);

    /**
     * @brief Detect N-N-X mine pattern in the vicinity of a cell
     * 
     * Identifies the N-N-X minesweeper pattern where two adjacent cells
     * each have exactly N remaining mines among their unknown neighbors.
     * This pattern allows for logical deduction of safe cell locations.
     * 
     * The pattern works as follows:
     * - Cell A has N remaining mines among its unknown neighbors
     * - Cell B has N remaining mines among its unknown neighbors
     * - If all unknown neighbors of cell B are also neighbors of cell A
     * - Then the unique neighbors of cell A (not shared with B) are safe
     * 
     * This is a powerful pattern that can reveal safe cells that would
     * otherwise require guessing. Searches in all four directions from
     * the given cell position to find this pattern.
     * 
     * This can be used to detect 1-1-X, 2-2-X, and 3-3-X patterns.
     * 
     * @param x X coordinate of the cell to analyze
     * @param y Y coordinate of the cell to analyze
     * @param remainingMines Remaining mines required by both cells (N)
     * @return true if a N-N-X pattern was detected and processed
     * @see applyNNXPatternLogic()
     */
    bool detectNNXPattern(uchar x, uchar y, int remainingMines);

    /**
     * @brief Detect an N-X-N mine pattern in the vicinity of a cell
     *
     * Checks cells two spaces apart horizontally and vertically for a second
     * clue with the same remaining-mine count. If the second clue's unknown
     * neighbors are a subset of this cell's unknown neighbors, the existing
     * N-N-X deduction marks this cell's unique neighbors safe.
     *
     * @param x X coordinate of the cell to analyze
     * @param y Y coordinate of the cell to analyze
     * @param remainingMines Remaining mines required by both clues (N)
     * @return true if an N-X-N pattern was detected and processed
     * @see detectNNXPattern()
     * @see applyNNXPatternLogic()
     */
    bool detectNXNPattern(uchar x, uchar y, int remainingMines);

    /**
     * @brief Count neighboring cells in a given solver state
     * 
     * Counts the neighbors of a cell whose solver state matches the requested
     * state, optionally collecting their coordinates.
     * 
     * @param x X coordinate of the cell to analyze
     * @param y Y coordinate of the cell to analyze
     * @param cellState Solver state of the neighbors to count
     * @param outSet Optional output set to receive the matching neighbor coordinates
     * @return Number of neighboring cells in the requested state
     */
    int countRemainingNeighbors(uchar x, uchar y, SolverCellState cellState, QSet<QPair<uchar, uchar>>* outSet = nullptr) const;


    /**
     * @brief Count the number of remaining mines adjacent to a cell
     * 
     * Calculates how many mines are still needed to satisfy a numbered cell's
     * constraint. This is determined by subtracting the number of already
     * identified mines from the cell's value.
     * 
     * @param x X coordinate of the cell to analyze
     * @param y Y coordinate of the cell to analyze
     * @return Number of remaining mines needed (0 if all mines found, negative if over-flagged)
     */
    int countRemainingMines(uchar x, uchar y) const;

    /**
     * @brief Apply M-N-X pattern deduction logic to identified cells
     *
     * If the analyzed cell has N remaining mines, the adjacent cell has M,
     * and the analyzed cell has exactly N-M unknown neighbors not shared with
     * the adjacent cell, those unique neighbors must all be mines.
     *
     * @param middleX X coordinate of the cell with N remaining mines
     * @param middleY Y coordinate of the cell with N remaining mines
     * @param adjacentX X coordinate of the adjacent cell
     * @param adjacentY Y coordinate of the adjacent cell
     * @param adjacentMines M, the remaining-mine count of the adjacent cell
     * @param remainingMines N, the remaining-mine count of the analyzed cell
     * @return true if the pattern logic was successfully applied
     */
    bool applyMNXPatternLogic(uchar middleX, uchar middleY, uchar adjacentX, uchar adjacentY, int adjacentMines, int remainingMines);

    /**
     * @brief Apply N-N-X pattern deduction logic to identified cells
     * 
     * Implements the logical deduction for the N-N-X pattern where:
     * - Cell 1 has N remaining mine among its unknown neighbors
     * - Cell 2 has N remaining mine among its unknown neighbors
     * - All unknown neighbors of cell 2 are also neighbors of cell 1
     * 
     * The deduction rule states that the unique neighbors of cell 1
     * (not shared with cell 2) must be safe, since cell 2's mine
     * must be among its own neighbors, and if all of cell 2's neighbors
     * are also neighbors of cell 1, then cell 1's mine must be among
     * the shared neighbors, making cell 1's unique neighbors safe.
     * 
     * @param middleX X coordinate of the first cell with N remaining mines
     * @param middleY Y coordinate of the first cell with N remaining mines
     * @param adjX X coordinate of the second cell with N remaining mines
     * @param adjY Y coordinate of the second cell with N remaining mines
     * @param remainingMines N, the remaining-mine count of both cells
     * @return true if the pattern logic was successfully applied
     */
    bool applyNNXPatternLogic(uchar middleX, uchar middleY, uchar adjX, uchar adjY, int remainingMines);

    /**
     * @brief Apply the 1-3-1 corner deduction to three safe clues
     *
     * When the middle clue has three remaining mines and each perpendicular
     * outer clue has one, marks the sole unknown neighbor unique to the middle
     * clue as a mine and the outer clues' unique unknown neighbors as safe.
     * The outer clues' unknown-neighbor sets must be disjoint.
     *
     * @param middleX X coordinate of the middle clue
     * @param middleY Y coordinate of the middle clue
     * @param firstX X coordinate of the first outer clue
     * @param firstY Y coordinate of the first outer clue
     * @param secondX X coordinate of the second outer clue
     * @param secondY Y coordinate of the second outer clue
     * @return true if the deduction was applied
     */
    bool apply131CornerPatternLogic(uchar middleX,
                                   uchar middleY,
                                   uchar firstX,
                                   uchar firstY,
                                   uchar secondX,
                                   uchar secondY);

    /**
     * @brief Apply the 2-2-2 corner deduction to three safe clues
     *
     * When all three clues have two remaining mines and the two outer clues'
     * unknown-neighbor sets are disjoint, marks the sole unknown neighbor
     * unique to the middle clue safe. An outer clue's sole unique unknown
     * neighbor is marked as a mine.
     *
     * @param middleX X coordinate of the middle clue
     * @param middleY Y coordinate of the middle clue
     * @param firstX X coordinate of the first outer clue
     * @param firstY Y coordinate of the first outer clue
     * @param secondX X coordinate of the second outer clue
     * @param secondY Y coordinate of the second outer clue
     * @return true if the deduction was applied
     */
    bool apply222CornerPatternLogic(uchar middleX,
                                   uchar middleY,
                                   uchar firstX,
                                   uchar firstY,
                                   uchar secondX,
                                   uchar secondY);

    /**
     * @brief Apply the general pair rule to two revealed clues
     *
     * Let onlyA and onlyB be the unknown neighbours exclusive to each clue and
     * d = kA - kB the difference between their remaining mine counts. Then
     * mines(onlyA) - mines(onlyB) = d, so the number of mines in onlyA lies
     * between max(0, d) and min(|onlyA|, d + |onlyB|), and symmetrically for
     * onlyB. When a bound forces a region to contain no mines it is marked
     * safe; when it forces the region to be completely filled it is marked as
     * mines.
     *
     * Both clues must already be marked Safe and share at least one unknown
     * neighbour, and their exclusive neighbour sets must not both be empty.
     *
     * @param ax X coordinate of the first clue
     * @param ay Y coordinate of the first clue
     * @param bx X coordinate of the second clue
     * @param by Y coordinate of the second clue
     * @return true if any cell was marked
     * @see cleanGeneralPairRule()
     */
    bool applyPairRule(uchar ax, uchar ay, uchar bx, uchar by);


    /**
     * @brief Enumerate the layouts allowed by a subset of the clues and mark the certain cells
     *
     * Using only some of the clues is safe: dropping a clue removes a
     * restriction, so every deduction that holds without it still holds with it.
     *
     * @param clues  All clues collected by cleanEnumeration()
     * @param subset Indices (into clues) of the clues to use
     * @return true if at least one cell was marked
     */
    bool enumerateClueSubset(const QVector<EnumerationClue>& clues, const QVector<int>& subset);

    /**
     * @brief Collect clues connected to a seed clue, nearest first
     *
     * Two clues are connected when they share an unknown cell. The search is
     * breadth-first from the seed, so the result is the seed followed by the
     * closest connected clues.
     *
     * @param seed        Index of the first clue
     * @param limit       Maximum number of clues to return; 0 or less means all connected clues
     * @param clues       All clues collected by cleanEnumeration()
     * @param cluesByCell For every unknown cell id, the indices of the clues that contain it
     * @return Indices (into clues) of the collected clues
     */
    QVector<int> collectConnectedClues(int seed,
                                       int limit,
                                       const QVector<EnumerationClue>& clues,
                                       const QHash<int, QVector<int>>& cluesByCell) const;

    /**
     * @brief Convert board coordinates to a single integer id (used as hash key)
     */
    int cellIndex(uchar x, uchar y) const;

    /**
     * @brief Convert an id created by cellIndex() back to board coordinates
     */
    QPair<uchar, uchar> cellFromIndex(int index) const;

    /**
     * @brief Collect every clue on the board (shared by the enumeration and mine counting tiers)
     *
     * A clue is a revealed number that still has unknown neighbours.
     *
     * @param[out] clues       The clues found, in board order (deterministic)
     * @param[out] cluesByCell For every unknown cell id, the indices of the clues that contain it
     */
    void collectEnumerationClues(QVector<EnumerationClue>& clues,
                                 QHash<int, QVector<int>>& cluesByCell) const;

    /**
     * @brief Translate a subset of the clues into the enumerator's input format
     *
     * The board cells used by the clues are renumbered 0..n-1, which is what the
     * enumerator works with.
     *
     * @param clues            All clues collected by collectEnumerationClues()
     * @param subset           Indices (into clues) of the clues to translate
     * @param[out] clueCells   For each clue, the local numbers of its cells
     * @param[out] clueMines   For each clue, the mines it still needs
     * @param[out] cellIds     Local number -> board cell id (see cellIndex())
     */
    void buildEnumerationInput(const QVector<EnumerationClue>& clues,
                               const QVector<int>& subset,
                               std::vector<std::vector<int>>& clueCells,
                               QVector<int>& clueMines,
                               QVector<int>& cellIds) const;

    /**
     * @brief Check whether an interruption has been requested
     *
     * Reads the pending solver command while holding the command mutex.
     *
     * @return true if the solver has received an interrupt command
     */
    bool checkForInterrupt();

    /**
     * @brief Update the solver state while holding its mutex
     *
     * @param newState State to assign to the solver
     */
    void setInternalState(SolverState newState);
};

#endif // LIBREMINESSOLVER_H
