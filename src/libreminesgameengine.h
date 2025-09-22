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
 * @file libreminesgameengine.h
 * @brief Core game engine for LibreMines minesweeper logic
 * @author Bruno Bollos Correa
 * @date 2020-12-12
 * 
 * This file contains the main game engine that handles all minesweeper game logic
 * including mine placement, cell clearing, flag management, timing, and score
 * generation. It manages the game state and communicates with the GUI through
 * Qt signals and slots.
 */

#ifndef LIBREMINESGAMEENGINE_H
#define LIBREMINESGAMEENGINE_H

#include <QPointer>
#include <QElapsedTimer>

#include "common.h"
#include "libreminesscore.h"

/**
 * @brief Core minesweeper game engine
 * 
 * LibreMinesGameEngine manages all aspects of minesweeper game logic including:
 * - Mine field generation and management
 * - Cell state tracking (hidden/revealed, flagged/unflagged)
 * - Game timing and scoring
 * - Win/loss condition detection
 * - User interaction processing
 * 
 * The engine communicates with the GUI layer through Qt signals and slots,
 * maintaining separation between game logic and presentation. It tracks the
 * complete game state and provides methods for querying game status and
 * statistics.
 * 
 * @since v1.0
 */
class LibreMinesGameEngine: public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Represents a single cell in the minesweeper grid
     * 
     * CellGameEngine encapsulates the state of an individual cell in the
     * game grid, including its value (mine or number), visibility state,
     * and flag status. This nested class provides a clean interface for
     * managing cell properties within the game engine.
     * 
     * @since v1.0
     */
    class CellGameEngine
    {
    public:
        /**
         * @brief Default constructor
         * 
         * Initializes a cell with default values: ZERO value, hidden state,
         * and no flag.
         */
        CellGameEngine();

        CellValue value;      ///< Cell content (mine or number of adjacent mines)
        bool isHidden;        ///< Whether the cell is still hidden from the player
        FlagState flagState;  ///< Current flag state of the cell
    };

public:
    /**
     * @brief Constructor for the game engine
     * 
     * Initializes the game engine with default settings. Sets up internal
     * timers and prepares the engine for a new game.
     */
    LibreMinesGameEngine();
    
    /**
     * @brief Start a new minesweeper game
     * 
     * Creates a new game with the specified dimensions and mine count.
     * Optionally ensures that a specific cell is kept clean (mine-free)
     * for the first click guarantee.
     * 
     * @param _X Width of the game board (number of columns)
     * @param _Y Height of the game board (number of rows)
     * @param i_nMines_ Total number of mines to place on the board
     * @param i_X_Clean X coordinate of cell to keep mine-free (255 = none)
     * @param i_Y_Clean Y coordinate of cell to keep mine-free (255 = none)
     * 
     * @note If i_X_Clean and i_Y_Clean are provided, that cell and its
     *       neighbors will be guaranteed to be mine-free for safe first click
     */
    void vNewGame(const uchar _X,
                  const uchar _Y,
                  short i_nMines_,
                  const uchar i_X_Clean = 255,
                  const uchar i_Y_Clean = 255);

    /**
     * @brief Get read-only access to the game matrix
     * 
     * Provides const access to the internal game matrix for querying
     * cell states and values. Used by the GUI to render the current
     * game state.
     * 
     * @return Const reference to the 2D vector containing all game cells
     */
    const std::vector< std::vector<CellGameEngine> >& getPrincipalMatrix()const;
    
    /**
     * @brief Get the number of rows in the game board
     * 
     * @return Number of rows (height) of the current game board
     */
    uchar rows()const;
    
    /**
     * @brief Get the number of columns in the game board
     * 
     * @return Number of columns (width) of the current game board
     */
    uchar lines()const;
    
    /**
     * @brief Get the total number of mines in the game
     * 
     * @return Total number of mines placed on the board
     */
    short mines()const;
    
    /**
     * @brief Get the number of cells that need to be unlocked to win
     * 
     * This represents the number of non-mine cells that must be revealed
     * to complete the game successfully.
     * 
     * @return Number of non-mine cells that must be revealed to win
     */
    ushort cellsToUnlock()const;
    
    /**
     * @brief Get the current number of hidden cells
     * 
     * @return Number of cells that are still hidden from the player
     */
    ushort hiddenCells()const;
    
    /**
     * @brief Check if the game is currently active
     * 
     * A game is active when it has been started but not yet won or lost.
     * 
     * @return true if the game is in progress, false if not started or ended
     */
    bool isGameActive()const;

    /**
     * @brief Set whether the first cell clicked should be guaranteed safe
     * 
     * When enabled, the first cell clicked will automatically be kept
     * mine-free to provide a fair start to the game.
     * 
     * @param x true to enable first cell safety guarantee, false to disable
     */
    void setFirstCellClean(const bool x);
    
    /**
     * @brief Set whether question mark flags are enabled
     * 
     * When enabled, players can cycle through no flag -> flag -> question mark
     * states. When disabled, only no flag -> flag states are available.
     * 
     * @param x true to enable question marks, false to disable
     */
    void setUseQuestionMark(const bool x);

    /**
     * @brief Set whether no-guess mode is enabled
     * 
     * When enabled, no-guess mode ensures that the generated minesweeper
     * puzzle can always be solved through logical deduction without requiring
     * guessing. This mode may adjust mine placement algorithms to guarantee
     * that every move can be determined through analysis of revealed numbers
     * and their relationships to neighboring cells.
     * 
     * @param x true to enable no-guess mode, false to allow traditional puzzles
     * @since v3.0
     */
    void setNoGuessMode(const bool x);

private:
    /**
     * @brief Reset the game matrix to initial state
     * 
     * Clears all cells and resets the game matrix to default values.
     * Called internally when starting a new game.
     */
    void vResetPrincipalMatrix();

    /**
     * @brief Clear a cell at the specified position
     * 
     * Reveals the cell at the given coordinates. If the cell contains a mine,
     * triggers game loss. If the cell has value ZERO, automatically clears
     * all neighboring cells (recursive clearing).
     *
     * @param _X X coordinate of the cell to clear
     * @param _Y Y coordinate of the cell to clear
     * @param recursive Indicates this cell is being cleared as part of a chain reaction
     * @return true if cell was successfully cleared, false otherwise
     */
    bool bCleanCell(const uchar _X, const uchar _Y, const bool recursive=true);
    
    /**
     * @brief Handle game loss condition
     * 
     * Called when a mine is clicked. Sets the game state to lost and
     * emits the appropriate signal with the coordinates of the triggered mine.
     * 
     * @param _X X coordinate of the mine that was clicked
     * @param _Y Y coordinate of the mine that was clicked
     */
    void vGameLost(const uchar _X, const uchar _Y);
    
    /**
     * @brief Handle game won condition
     * 
     * Called when all non-mine cells have been revealed. Sets the game
     * state to won and emits the appropriate signal.
     */
    void vGameWon();

    /**
     * @brief Generate end-of-game score and statistics
     * 
     * Calculates the final score including time taken, accuracy statistics,
     * and other performance metrics. Emits the score signal for the GUI
     * to display or save.
     * 
     * @param iTimeInNs Custom time in nanoseconds (-1 to use actual elapsed time)
     * @param ignorePreferences Whether to ignore user preferences for score saving
     */
    void vGenerateEndGameScore(qint64 iTimeInNs = -1, bool ignorePreferences=false);

    std::vector< std::vector<CellGameEngine> > principalMatrix; ///< 2D grid containing all game cells

    uchar iX;               ///< Current game board width (number of columns)
    uchar iY;               ///< Current game board height (number of rows)

    short nMines;           ///< Total number of mines in the current game
    short iMinesLeft;       ///< Number of mines not yet flagged by the player
    ushort iHiddenCells;    ///< Number of cells still hidden from the player
    ushort iCellsToUnlock;  ///< Number of non-mine cells that need to be revealed to win

    QScopedPointer<QTimer> timerTimeInGame; ///< Timer for updating game duration display
    QElapsedTimer elapsedPreciseTimeInGame;  ///< High-precision timer for scoring

    bool bFirst;            ///< Whether this is the first cell click of the game
    bool bFirstCellClean;   ///< Whether first clicked cell should be guaranteed safe

    bool bUseQuestionMark;  ///< Whether question mark flags are enabled
    bool bNoGuessMode;      ///< Whether no-guess mode is enabled for logical-only puzzles

    bool bGameActive;       ///< Whether the game is currently in progress

Q_SIGNALS:
    /**
     * @brief Signal emitted when a cell should be revealed in the GUI
     * 
     * Emitted when the game engine determines that a cell should be shown
     * to the player, either from direct clicking or recursive clearing.
     * 
     * @param _X X coordinate of the cell to show
     * @param _Y Y coordinate of the cell to show
     * @param recursive true if this cell is being shown as part of auto-clearing
     */
    void SIGNAL_showCell(const uchar _X, const uchar _Y, const bool recursive);
    
    /**
     * @brief Signal emitted when the game ends with complete statistics
     * 
     * Provides comprehensive end-game statistics including score, timing,
     * and accuracy metrics for display or saving by the GUI.
     * 
     * @param score The final LibreMinesScore object with game results
     * @param iCorrectFlags Number of correctly placed flags
     * @param iWrongFlags Number of incorrectly placed flags
     * @param iUnlockedCells Number of cells successfully revealed
     * @param dFlagsPerSecond Rate of flag placement during the game
     * @param dCellsPerSecond Rate of cell clearing during the game
     * @param ignorePreferences Whether to ignore user preferences for score handling
     */
    void SIGNAL_endGameScore(LibreMinesScore score,
                             int iCorrectFlags,
                             int iWrongFlags,
                             int iUnlockedCells,
                             double dFlagsPerSecond,
                             double dCellsPerSecond,
                             bool ignorePreferences=false);
    
    /**
     * @brief Signal emitted to update the game timer display
     * 
     * @param time Current game time in seconds
     */
    void SIGNAL_currentTime(const ushort);
    
    /**
     * @brief Signal emitted when the mine count display should be updated
     * 
     * @param count Number of mines remaining (total mines minus flagged cells)
     */
    void SIGNAL_minesLeft(const short);
    
    /**
     * @brief Signal emitted when a cell should be flagged in the GUI
     * 
     * @param _X X coordinate of the cell to flag
     * @param _Y Y coordinate of the cell to flag
     */
    void SIGNAL_flagCell(const uchar _X, const uchar _Y);
    
    /**
     * @brief Signal emitted when a cell should show a question mark in the GUI
     * 
     * @param _X X coordinate of the cell to mark with question
     * @param _Y Y coordinate of the cell to mark with question
     */
    void SIGNAL_questionCell(const uchar _X, const uchar _Y);
    
    /**
     * @brief Signal emitted when a cell flag should be removed in the GUI
     * 
     * @param _X X coordinate of the cell to unflag
     * @param _Y Y coordinate of the cell to unflag
     */
    void SIGNAL_unflagCell(const uchar _X, const uchar _Y);
    
    /**
     * @brief Signal emitted when the player has won the game
     * 
     * Emitted when all non-mine cells have been successfully revealed.
     */
    void SIGNAL_gameWon();
    
    /**
     * @brief Signal emitted when the player has lost the game
     * 
     * @param _X X coordinate of the mine that was clicked
     * @param _Y Y coordinate of the mine that was clicked
     */
    void SIGNAL_gameLost(const uchar _X, const uchar _Y);
    
    /**
     * @brief Signal emitted to request a game restart
     * 
     * Emitted when the game should be restarted with the same parameters.
     */
    void SIGNAL_remakeGame();

public Q_SLOTS:
    /**
     * @brief Slot to handle cell clearing requests from the GUI
     * 
     * Processes a request to clear (reveal) a cell at the specified coordinates.
     * If the cell contains a mine, the game will end in loss. If the cell
     * is empty (value ZERO), neighboring cells will be automatically cleared.
     * 
     * @param _X X coordinate of the cell to clear
     * @param _Y Y coordinate of the cell to clear
     */
    void SLOT_cleanCell(const uchar _X, const uchar _Y);
    
    /**
     * @brief Slot to handle flag state changes from the GUI
     * 
     * Cycles through the available flag states for a cell:
     * no flag -> flag -> question mark (if enabled) -> no flag
     * 
     * @param _X X coordinate of the cell to change flag state
     * @param _Y Y coordinate of the cell to change flag state
     */
    void SLOT_changeFlagState(const uchar _X, const uchar _Y);
    
    /**
     * @brief Slot to stop the current game
     * 
     * Stops the game timer and sets the game to inactive state.
     * Used when pausing or ending a game.
     */
    void SLOT_stop();
    
    /**
     * @brief Slot to start the game timer
     * 
     * Begins timing the current game. Usually called after the first
     * cell is clicked.
     */
    void SLOT_startTimer();
    
    /**
     * @brief Slot to handle neighbor cell clearing requests
     * 
     * Attempts to clear all neighboring cells around the specified coordinates.
     * This is typically used for the "chord" action when clicking on a
     * revealed number cell that has the correct number of flags around it.
     * 
     * @param _X X coordinate of the center cell
     * @param _Y Y coordinate of the center cell
     */
    void SLOT_cleanNeighborCells(const uchar _X, const uchar _Y);
    
    /**
     * @brief Slot to regenerate the end game score
     * 
     * Recalculates and re-emits the end game score signal. Used when
     * score calculation needs to be updated or repeated.
     */
    void SLOT_generateEndGameScoreAgain();

private Q_SLOTS:
    /**
     * @brief Internal slot to update the game timer display
     * 
     * Called periodically by the internal timer to update the elapsed
     * time display. Emits SIGNAL_currentTime with the current game duration.
     */
    void SLOT_UpdateTime();

};

#endif // LIBREMINESGAMEENGINE_H
