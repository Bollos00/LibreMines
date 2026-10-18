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
 * @file libreminesconstraintenumerator.hpp
 * @brief Exhaustive search over the mine layouts allowed by a set of clues
 *
 * The enumerator is a small constraint-satisfaction engine. It knows nothing
 * about the minesweeper board: it only receives "clues" (a list of unknown
 * cells plus how many of them are mines) and reports, for every cell, whether
 * some valid layout makes it a mine and whether some valid layout makes it
 * safe. A cell that can only be one of the two is a certain deduction.
 *
 * Keeping it independent from LibreMinesSolver makes it easy to unit test
 * (see the example in the class documentation).
 */

#ifndef LIBREMINESCONSTRAINTENUMERATOR_HPP
#define LIBREMINESCONSTRAINTENUMERATOR_HPP

#include <QtGlobal>
#include <QVector>

/**
 * @brief Backtracking enumerator of all mine layouts consistent with a set of clues
 *
 * Variables are cells numbered 0..nCells-1 (0 = safe, 1 = mine). Each clue says
 * "exactly N of these cells are mines". The search places cells one by one and
 * abandons a branch as soon as a clue becomes impossible to satisfy.
 *
 * Example (clues A: {a,b}=1, B: {b,c}=1, C: {a,c,d}=1, cells a..d = 0..3):
 * @code
 * LibreMinesConstraintEnumerator::Input in;
 * in.nCells    = 4;
 * in.clueCells = { {0,1}, {1,2}, {0,2,3} };
 * in.clueMines = { 1, 1, 1 };
 * auto r = LibreMinesConstraintEnumerator().run(in, 100000);
 * // exactly one layout survives: b and d are mines, a and c are safe
 * @endcode
 *
 * @see LibreMinesSolver::cleanEnumeration()
 * @since v3.0.0
 */
class LibreMinesConstraintEnumerator
{
public:
    /**
     * @brief Problem description handed to the enumerator
     *
     * @since v3.0.0
     */
    struct Input
    {
        int nCells = 0;                    ///< Number of cells (variables) in the problem
        std::vector<std::vector<int>> clueCells;   ///< For each clue: the cells it covers (indices 0..nCells-1)
        QVector<int> clueMines;            ///< For each clue: how many of its cells are mines

        /// Optional cap on the number of mines in a layout; layouts with more mines are not
        /// enumerated (-1 = no cap). Only use it when such layouts are known to be impossible
        /// anyway, e.g. because the board's mine counter does not allow them.
        int maxMines = -1;
    };

    /**
     * @brief Outcome of one enumeration
     *
     * Only trust canBeMine/canBeSafe when budgetExceeded is false: a search that
     * was cut short has not seen every layout, so "always a mine" proves nothing.
     *
     * @since v3.0.0
     */
    struct Result
    {
        bool budgetExceeded = false;   ///< The node budget ran out; flags below are incomplete and must be ignored
        bool saturated = false;        ///< Every cell was seen as both mine and safe, so nothing can be deduced
                                       ///< (the search stops early; `solutions` is then only a partial count)
        quint64 solutions = 0;         ///< Layouts found (0 on a finished search means the clues contradict each other)
        quint64 nodes = 0;             ///< Search nodes visited (for budget tuning and debugging)
        QVector<bool> canBeMine;       ///< canBeMine[i]: at least one valid layout has cell i as a mine
        QVector<bool> canBeSafe;       ///< canBeSafe[i]: at least one valid layout has cell i as safe

        // --- Only filled when run() is called with recordMineCounts = true ---------------
        // They describe the layouts grouped by how many mines each one has, which is what
        // the global mine counter needs (see LibreMinesSolver::cleanMineCounting()).
        int nCells = 0;                    ///< Number of cells of the problem (row length of the tables below)
        QVector<bool> mineTotalPossible;   ///< mineTotalPossible[t]: some valid layout has exactly t mines
        QVector<bool> mineAtTotal;         ///< flat table [t * nCells + i]: cell i is a mine in some t-mine layout
        QVector<bool> safeAtTotal;         ///< flat table [t * nCells + i]: cell i is safe in some t-mine layout

        /**
         * @brief Query mine possibility for a given mine total
         *
         * @param total Number of mines the layout must have
         * @param i Cell index
         * @return true if some valid layout with exactly @p total mines has
         *         cell @p i as a mine
         * @note Only meaningful when run() was called with
         *       recordMineCounts = true and budgetExceeded is false
         */
        bool canBeMineWithTotal(int total, int i) const { return mineAtTotal[total * nCells + i]; }

        /**
         * @brief Query safe possibility for a given mine total
         *
         * @param total Number of mines the layout must have
         * @param i Cell index
         * @return true if some valid layout with exactly @p total mines has
         *         cell @p i as safe
         * @note Only meaningful when run() was called with
         *       recordMineCounts = true and budgetExceeded is false
         */
        bool canBeSafeWithTotal(int total, int i) const { return safeAtTotal[total * nCells + i]; }

        /**
         * @brief Check whether a cell is a mine in every valid layout
         *
         * @param i Cell index
         * @return true if cell @p i is a mine in every valid layout
         * @note Only trustworthy when budgetExceeded is false
         */
        bool isMineInAllSolutions(int i) const { return canBeMine[i] && !canBeSafe[i]; }

        /**
         * @brief Check whether a cell is safe in every valid layout
         *
         * @param i Cell index
         * @return true if cell @p i is safe in every valid layout
         * @note Only trustworthy when budgetExceeded is false
         */
        bool isSafeInAllSolutions(int i) const { return canBeSafe[i] && !canBeMine[i]; }
    };

    /**
     * @brief Enumerate the layouts allowed by the clues
     *
     * @param input       Cells and clues to solve
     * @param nodeBudget  Maximum number of search nodes; protects against rare, very large problems
     * @param recordMineCounts  Also record, for every possible number of mines, what the layouts
     *                    with that many mines say about each cell. This needs every layout, so the
     *                    early stop (see Result::saturated) is disabled.
     * @return Per-cell mine/safe possibilities (see Result for when they can be trusted)
     * @since v3.0.0
     */
    Result run(const Input& input, quint64 nodeBudget, bool recordMineCounts = false)
    {
        m_input = &input;
        m_nodeBudget = nodeBudget;
        m_stop = false;
        m_undetermined = input.nCells;

        m_result = Result();
        m_result.canBeMine.fill(false, input.nCells);
        m_result.canBeSafe.fill(false, input.nCells);

        m_recordMineCounts = recordMineCounts;
        m_minesPlaced = 0;
        m_result.nCells = input.nCells;
        if (recordMineCounts)
        {
            // One row per possible number of mines (0..nCells)
            m_result.mineTotalPossible.fill(false, input.nCells + 1);
            m_result.mineAtTotal.fill(false, (input.nCells + 1) * input.nCells);
            m_result.safeAtTotal.fill(false, (input.nCells + 1) * input.nCells);
        }

        const int nClues = input.clueCells.size();

        // Reverse index: which clues are affected when a given cell is assigned
        m_cluesOfCell = std::vector<std::vector<int>>(input.nCells);
        for (int clue = 0; clue < nClues; ++clue)
        {
            for (int cell : input.clueCells[clue])
            {
                m_cluesOfCell[cell].push_back(clue);
            }
        }

        // Per-clue bookkeeping, updated incrementally while the search runs:
        //  - m_missingMines: mines the clue still needs
        //  - m_unassigned:   cells of the clue that have not been decided yet
        m_missingMines = input.clueMines;
        m_unassigned.resize(nClues);
        for (int clue = 0; clue < nClues; ++clue)
        {
            m_unassigned[clue] = input.clueCells[clue].size();
        }

        m_assignment.fill(-1, input.nCells);   // -1 = undecided, 0 = safe, 1 = mine

        buildSearchOrder();
        search(0);

        // Saturation means "stopped early because nothing more can be learned"; when recording
        // mine counts every layout is visited, so it never applies.
        m_result.saturated = (m_undetermined == 0) && !m_recordMineCounts;
        return m_result;
    }

private:
    const Input* m_input = nullptr;     ///< Problem being solved (owned by the caller)
    quint64 m_nodeBudget = 0;           ///< Search node limit
    bool m_stop = false;                ///< Set to abort the whole search (budget hit or nothing left to learn)
    int m_undetermined = 0;             ///< Cells not yet seen as both mine and safe
    bool m_recordMineCounts = false;    ///< Also record the mine total of every layout (disables the early stop)
    int m_minesPlaced = 0;              ///< Mines among the cells assigned so far
    Result m_result;                    ///< Result being built

    std::vector<std::vector<int>> m_cluesOfCell;   ///< For each cell: clues that contain it
    QVector<int> m_missingMines;           ///< For each clue: mines still needed
    QVector<int> m_unassigned;             ///< For each clue: cells still undecided
    QVector<int> m_assignment;             ///< For each cell: -1 undecided, 0 safe, 1 mine
    QVector<int> m_searchOrder;            ///< Order in which cells are decided

    /**
     * @brief Decide the order in which cells are assigned
     *
     * Breadth-first over the "cell -> clue -> cell" graph, so neighbouring cells
     * are decided one after another. This makes clues become fully assigned
     * early, which is when wrong branches get detected and discarded.
     */
    void buildSearchOrder()
    {
        const int nCells = m_input->nCells;
        const int nClues = m_input->clueCells.size();

        m_searchOrder.clear();
        QVector<bool> cellQueued(nCells, false);
        QVector<bool> clueExpanded(nClues, false);

        for (int start = 0; start < nCells; ++start)
        {
            if (cellQueued[start])
            {
                continue;
            }

            // `queue` is a plain vector walked with an index: cheaper than dequeuing
            QVector<int> queue;
            queue.append(start);
            cellQueued[start] = true;

            for (int head = 0; head < queue.size(); ++head)
            {
                const int cell = queue[head];
                m_searchOrder.append(cell);

                for (int clue : m_cluesOfCell[cell])
                {
                    if (clueExpanded[clue])
                    {
                        continue;
                    }
                    clueExpanded[clue] = true;

                    for (int neighbour : m_input->clueCells[clue])
                    {
                        if (!cellQueued[neighbour])
                        {
                            cellQueued[neighbour] = true;
                            queue.append(neighbour);
                        }
                    }
                }
            }
        }
    }

    /**
     * @brief Assign a value to a cell and update every clue that contains it
     *
     * @param cell  Cell to assign
     * @param value 0 for safe, 1 for mine
     * @return false if some clue can no longer be satisfied. The assignment is
     *         still applied, so the caller must always call unplace().
     */
    bool place(int cell, int value)
    {
        bool consistent = true;

        for (int clue : m_cluesOfCell[cell])
        {
            --m_unassigned[clue];
            m_missingMines[clue] -= value;

            // Too many mines placed, or not enough cells left to supply the missing ones
            if (m_missingMines[clue] < 0 || m_missingMines[clue] > m_unassigned[clue])
            {
                consistent = false;
            }
        }

        m_assignment[cell] = value;
        return consistent;
    }

    /**
     * @brief Undo place()
     */
    void unplace(int cell, int value)
    {
        for (int clue : m_cluesOfCell[cell])
        {
            ++m_unassigned[clue];
            m_missingMines[clue] += value;
        }

        m_assignment[cell] = -1;
    }

    /**
     * @brief Recursive backtracking search
     *
     * @param index Position in m_searchOrder of the next cell to decide
     */
    void search(int index)
    {
        if (m_stop)
        {
            return;
        }

        // Safety valve: give up on problems that are too large
        if (++m_result.nodes > m_nodeBudget)
        {
            m_result.budgetExceeded = true;
            m_stop = true;
            return;
        }

        // Every cell is decided and no clue was violated: this is a valid layout
        if (index == m_input->nCells)
        {
            recordSolution();
            return;
        }

        const int cell = m_searchOrder[index];
        for (int value = 0; value <= 1 && !m_stop; ++value)   // try "safe", then "mine"
        {
            // Mine cap: placing one more mine would exceed the allowed number of mines
            if (value == 1 && m_input->maxMines >= 0 && m_minesPlaced >= m_input->maxMines)
            {
                continue;
            }

            m_minesPlaced += value;
            if (place(cell, value))
            {
                search(index + 1);
            }
            unplace(cell, value);
            m_minesPlaced -= value;
        }
    }

    /**
     * @brief Remember what the current (valid) layout says about each cell
     *
     * When mine counts are recorded, the layout is also filed under its number of
     * mines (m_minesPlaced), so the caller can later keep only the layouts whose
     * total is compatible with the board's mine counter.
     */
    void recordSolution()
    {
        ++m_result.solutions;

        // Row of the per-total tables that this layout belongs to
        const int tableRow = m_minesPlaced * m_input->nCells;
        if (m_recordMineCounts)
        {
            m_result.mineTotalPossible[m_minesPlaced] = true;
        }

        for (int cell = 0; cell < m_input->nCells; ++cell)
        {
            const bool alreadySeenBoth = m_result.canBeMine[cell] && m_result.canBeSafe[cell];

            if (m_assignment[cell] == 1)
            {
                m_result.canBeMine[cell] = true;
            }
            else
            {
                m_result.canBeSafe[cell] = true;
            }

            if (m_recordMineCounts)
            {
                if (m_assignment[cell] == 1)
                {
                    m_result.mineAtTotal[tableRow + cell] = true;
                }
                else
                {
                    m_result.safeAtTotal[tableRow + cell] = true;
                }
            }

            if (!alreadySeenBoth && m_result.canBeMine[cell] && m_result.canBeSafe[cell])
            {
                --m_undetermined;
            }
        }

        // If every cell has been seen both ways, more layouts cannot add any
        // deduction, so stop searching (unless the mine totals are being recorded:
        // a layout with a different total could still matter to the mine counter).
        if (m_undetermined == 0 && !m_recordMineCounts)
        {
            m_stop = true;
        }
    }
};

#endif // LIBREMINESCONSTRAINTENUMERATOR_HPP