#include "libreminessolver.h"

#include "gameboardrandomgenerator.h"
#include <QMutexLocker>
#include <QQueue>
#include "libreminesconstraintenumerator.hpp"

#define LIBREMINES_SOLVER_DEBUG 0

QString ToString(LibreMinesSolver::SolverStrategy strategy)
{
    switch (strategy)
    {
        case LibreMinesSolver::SolverStrategy::None: return "None";
        case LibreMinesSolver::SolverStrategy::Pattern12X: return "Pattern12X";
        case LibreMinesSolver::SolverStrategy::Pattern11X: return "Pattern11X";
        case LibreMinesSolver::SolverStrategy::Pattern13X: return "Pattern13X";
        case LibreMinesSolver::SolverStrategy::Pattern14X: return "Pattern14X";
        case LibreMinesSolver::SolverStrategy::Pattern23X: return "Pattern23X";
        case LibreMinesSolver::SolverStrategy::Pattern24X: return "Pattern24X";
        case LibreMinesSolver::SolverStrategy::Pattern34X: return "Pattern34X";
        case LibreMinesSolver::SolverStrategy::Pattern22X: return "Pattern22X";
        case LibreMinesSolver::SolverStrategy::Pattern33X: return "Pattern33X";
        case LibreMinesSolver::SolverStrategy::Pattern1X1: return "Pattern1X1";
        case LibreMinesSolver::SolverStrategy::Pattern2X2: return "Pattern2X2";
        case LibreMinesSolver::SolverStrategy::Pattern222Corner: return "Pattern222Corner";
        case LibreMinesSolver::SolverStrategy::Pattern131Corner: return "Pattern131Corner";
        case LibreMinesSolver::SolverStrategy::PairRule: return "PairRule";
        case LibreMinesSolver::SolverStrategy::Enumeration: return "Enumeration";
        case LibreMinesSolver::SolverStrategy::MineCounting: return "MineCounting";
    }

    return QString("Unknown Strategy (%1)").arg(static_cast<quint64>(strategy));
}

#if LIBREMINES_SOLVER_DEBUG
void printSolverBoard(const std::vector<std::vector<LibreMinesSolver::SolverCellState>>& solverBoard,
                      const std::vector<std::vector<LibreMinesGameEngine::CellGameEngine>>& board)
{
    QString str;
    str += '\n';

    for (uchar j = 0; j < solverBoard[0].size(); ++j)
    {
        for (uchar i = 0; i < solverBoard.size(); ++i)
        {
            QChar c;
            switch (solverBoard[i][j])
            {
                case LibreMinesSolver::SolverCellState::Unknown: c = '='; break;
                case LibreMinesSolver::SolverCellState::Safe:    c = QChar((char)board[i][j].value + '0'); break;
                case LibreMinesSolver::SolverCellState::Mine:    c = '*'; break;
                default:                                         c = ' '; break;
            }
            str += c + ' ';
        }
        str += '\n';
    }

    qDebug() << qPrintable(str) << "\n\n";
}
#endif

LibreMinesSolver::LibreMinesSolver(QObject *parent) : 
    QThread{parent},
    state(SolverState::NotStarted),
    cmd(SolverCommand::CmdNone),
    strategies(SolverStrategy::None),
    maxEnumerationClues(DefaultMaxEnumerationClues)
{}

void LibreMinesSolver::run()
{
    // Set state to running at the beginning
    setInternalState(SolverState::Running);
    
    bool solvable = false;

    int triesCount = 0;
    while (!solvable)
    {
        if (checkForInterrupt())
        {
            setInternalState(SolverState::Interrupted);
            break;
        }

        board = GameBoardRandomGenerator::vGenerate(cols, rows, nMines, safeX, safeY);
        solverBoard = std::vector<std::vector<SolverCellState>>(cols, std::vector<SolverCellState>(rows, SolverCellState::Unknown));
        cellsNeedingTreatment.clear();
        nCellsMarkedSafe = 0;
        nCellsMarkedMine = 0;
        solvable = solve();

#if LIBREMINES_SOLVER_DEBUG
        qDebug() << "Tried boards: " << ++triesCount;
#endif
    }
    
    // Set state to finished upon successful completion
    if (solvable)
    {
        setInternalState(SolverState::Finished);
    }
}

bool LibreMinesSolver::solve()
{
    // Implement the solving logic here

    // First clear all neighbor cells of the safe cell
    cleanZero(safeX, safeY);

#if LIBREMINES_SOLVER_DEBUG
    qDebug() << "Initial step:";
    printSolverBoard(solverBoard, board);
#endif

    // Treatment loop: keep processing cells until no more changes can be made
    bool globalChanges = true;
    while (globalChanges && !cellsNeedingTreatment.isEmpty())
    {
        if (checkForInterrupt())
        {
            setInternalState(SolverState::Interrupted);
            return false;
        }

        globalChanges = false;

#if LIBREMINES_SOLVER_DEBUG
        qDebug() << "Applying basic cleaning...";
#endif

        // Process all cells currently in the treatment set
        QSet<QPair<uchar, uchar>> currentTreatmentSet = cellsNeedingTreatment;

        for (const QPair<uchar, uchar>& cellCoord : currentTreatmentSet)
        {
            // Apply basic cleaning logic to this cell
            if (cleanBasic(cellCoord.first, cellCoord.second))
            {
                globalChanges = true;
            }
        }
        
        // If basic rules didn't make progress, try pattern recognition
        if (!globalChanges)
        {
#if LIBREMINES_SOLVER_DEBUG
            qDebug() << "Applying basic pattern cleaning...";
#endif
            QSet<QPair<uchar, uchar>> patternTreatmentSet = cellsNeedingTreatment;
            
            for (const QPair<uchar, uchar>& cellCoord : patternTreatmentSet)
            {   
                // Apply pattern recognition to this cell
                if (cleanBasicPatterns(cellCoord.first, cellCoord.second))
                {
                    globalChanges = true;
                    // Keep the cell in treatment set as patterns might reveal more info
                }
            }
        }

        // If basic and pattern cleaning didn't make progress, try advanced patterns
        if (!globalChanges)
        {
#if LIBREMINES_SOLVER_DEBUG
            qDebug() << "Applying advanced pattern cleaning...";
#endif
            QSet<QPair<uchar, uchar>> advancedPatternTreatmentSet = cellsNeedingTreatment;

            for (const QPair<uchar, uchar>& cellCoord : advancedPatternTreatmentSet)
            {   
                // Apply advanced pattern recognition to this cell
                if (cleanAdvancedPatterns(cellCoord.first, cellCoord.second))
                {
                    globalChanges = true;
                    // Keep the cell in treatment set as patterns might reveal more info
                }
            }

        }

        if (!globalChanges)
        {
#if LIBREMINES_SOLVER_DEBUG
            qDebug() << "Applying general pair rule cleaning...";
#endif
            QSet<QPair<uchar, uchar>> pairRuleTreatmentSet = cellsNeedingTreatment;

            for (const QPair<uchar, uchar>& cellCoord : pairRuleTreatmentSet)
            {

                // Apply general pair rule to this cell
                if (cleanGeneralPairRule(cellCoord.first, cellCoord.second))
                {
                    globalChanges = true;
                    // Keep the cell in treatment set as patterns might reveal more info
                }
            }
        }

        // If no pattern made progress either, try exhaustive enumeration of the frontier.
        // This is the most expensive tier, so it only runs when everything cheaper has stalled.
        if (!globalChanges)
        {
#if LIBREMINES_SOLVER_DEBUG
            qDebug() << "Applying enumeration...";
#endif
            if (cleanEnumeration())
            {
                globalChanges = true;
            }
        }

        // Last resort: use the global mine counter to discard impossible layouts.
        // It enumerates every group completely, so it only runs when enumeration stalled too.
        if (!globalChanges)
        {
#if LIBREMINES_SOLVER_DEBUG
            qDebug() << "Applying mine counting...";
#endif
            if (cleanMineCounting())
            {
                globalChanges = true;
            }
        }


#if LIBREMINES_SOLVER_DEBUG
        // Debug output for each iteration
        if (globalChanges)
        {
            qDebug() << "Treatment iteration completed with changes:";
            printSolverBoard(solverBoard, board);
        }
        else
        {
            qDebug() << "Treatment iteration completed with no changes.";
        }
#endif
    }

    const quint32 totalCells = static_cast<quint32>(cols) * static_cast<quint32>(rows);
    // If it has released all safe cells or marks all mines, the board is considered solved
    const bool result = (nCellsMarkedSafe == totalCells - nMines) || (nCellsMarkedMine == nMines);

#if LIBREMINES_SOLVER_DEBUG
    if (result)
    {
        qDebug() << "Solver successfully solved the board.";
    }
    else
    {
        qDebug() << "Solver could not solve the board with current logic.";
    }

    qDebug() << "Final solver board state:";
    printSolverBoard(solverBoard, board);

    qDebug() << "Cells marked safe:" << nCellsMarkedSafe << "/" << (totalCells - nMines);
    qDebug() << "Cells marked as mines:" << nCellsMarkedMine << "/" << nMines;
    qDebug() << "Solved " << ((nCellsMarkedSafe + nCellsMarkedMine) * 100.0 / totalCells) << "\% of the board.";
    qDebug() << "Final cellsNeedingTreatment: " << cellsNeedingTreatment;
    for (QMap<SolverStrategy, quint64>::const_iterator it = strategyCounters.constBegin(); it != strategyCounters.constEnd(); ++it)
    {
        qDebug() << "Strategy" << ToString(it.key()) << "was applied" << it.value() << "times.";
    }
#endif

    return result;
}

void LibreMinesSolver::initialStep()
{
    cleanZero(safeX, safeY);
}

void LibreMinesSolver::markCell(const QPair<uchar, uchar>& cell, SolverCellState newState)
{
    if (newState == SolverCellState::Unknown)
    {
        return;
    }

    if (newState == SolverCellState::Safe && cleanZero(cell.first, cell.second))
    {
        return;
    }

    updateCellState(cell, newState);
}

bool LibreMinesSolver::markAllCells(const QSet<QPair<uchar, uchar>> &cells, SolverCellState newState)
{
    bool changed = false;
    for (const QPair<uchar, uchar>& cell : cells)
    {
        if (solverBoard[cell.first][cell.second] != SolverCellState::Unknown)
        {
            continue;
        }
        markCell(cell, newState);
        changed = true;
    }
    return changed;
}

void LibreMinesSolver::updateCellState(const QPair<uchar, uchar>& cell, SolverCellState newState)
{
    if (newState == SolverCellState::Unknown)
    {
        return;
    }

    SolverCellState& currentState = solverBoard[cell.first][cell.second];
    if (currentState != newState)
    {
        if (currentState == SolverCellState::Safe && nCellsMarkedSafe > 0)
        {
            --nCellsMarkedSafe;
        }
        else if (currentState == SolverCellState::Mine && nCellsMarkedMine > 0)
        {
            --nCellsMarkedMine;
        }

        if (newState == SolverCellState::Safe)
        {
            ++nCellsMarkedSafe;
        }
        else
        {
            ++nCellsMarkedMine;
        }

        currentState = newState;
    }

    if (newState == SolverCellState::Safe)
    {
        cellsNeedingTreatment.insert(cell);
    }
    else
    {
        cellsNeedingTreatment.remove(cell);
    }
}

bool LibreMinesSolver::cleanZero(uchar x, uchar y)
{
    if (solverBoard[x][y] != SolverCellState::Unknown || board[x][y].value != CellValue::ZERO)
    {
        return false;
    }

    // Record the zero cell without re-entering markCell's zero-expansion check.
    updateCellState({x, y}, SolverCellState::Safe);

    // Flag all neighboring cells as safe, since there is no neighbor mine
    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            if (dx == 0 && dy == 0) continue;  // Skip the center cell
            uchar nx = x + dx;
            uchar ny = y + dy;
            if (nx < cols && ny < rows)
            {
                markCell({nx, ny}, SolverCellState::Safe);
            }
        }
    }
    cellsNeedingTreatment.remove({x, y});

    return true;
}

bool LibreMinesSolver::cleanBasic(uchar x, uchar y)
{
    if (solverBoard[x][y] != SolverCellState::Safe)
    {
        return false;
    }

    if (board[x][y].value == CellValue::ZERO)
    {
        return cleanZero(x, y);
    }

    const qint8 mines = countRemainingNeighbors(x, y, SolverCellState::Mine);
    QSet<QPair<uchar, uchar>> unknownNeighbors;
    const qint8 unknown = countRemainingNeighbors(x, y, SolverCellState::Unknown, &unknownNeighbors);

    // Get the expected number of mines for this cell
    const qint8 expectedMines = static_cast<qint8>(board[x][y].value);

    // If we've found all the mines, mark remaining unknowns as safe
    if (mines == expectedMines)
    {
        for (const QPair<uchar, uchar>& neighbor : unknownNeighbors)
        {
            if (solverBoard[neighbor.first][neighbor.second] != SolverCellState::Unknown)
            {
                continue;
            }

            markCell(neighbor, SolverCellState::Safe);
        }
        
        cellsNeedingTreatment.remove({x, y});
        return true;
    }

    // If the number of unknown neighbors equals remaining mines needed, mark them all as mines
    if ((mines + unknown) == expectedMines)
    {
        for (const QPair<uchar, uchar>& neighbor : unknownNeighbors)
        {
            markCell(neighbor, SolverCellState::Mine);
        }

        cellsNeedingTreatment.remove({x, y});
        return true;
    }

    return false;
}

bool LibreMinesSolver::cleanBasicPatterns(uchar x, uchar y)
{
    if (solverBoard[x][y] != SolverCellState::Safe)
    {
        return false;
    }

    // Try different pattern recognition algorithms
    
    // 1. Try 1-2-X pattern detection
    if (detect12XPattern(x, y))
    {
        strategyCounters[SolverStrategy::Pattern12X]++;
        return true;
    }
    
    // 2. Try 1-1-X pattern detection
    if (detect11XPattern(x, y))
    {
        strategyCounters[SolverStrategy::Pattern11X]++;
        return true;
    }

    // 3. Try 1-3-X pattern detection
    if (detect13XPattern(x, y))
    {
        strategyCounters[SolverStrategy::Pattern13X]++;
        return true;
    }

    // 4. Try 1-4-X pattern detection
    if (detect14XPattern(x, y))
    {
        strategyCounters[SolverStrategy::Pattern14X]++;
        return true;
    }

    return false;
}

bool LibreMinesSolver::cleanAdvancedPatterns(uchar x, uchar y)
{
    if (solverBoard[x][y] != SolverCellState::Safe)
    {
        return false;
    }

    // Try different pattern recognition algorithms

    // 1. Try 2-3-X pattern detection
    if (detect23XPattern(x, y))
    {
        strategyCounters[SolverStrategy::Pattern23X]++;
        return true;
    }

    // 2. Try 2-4-X pattern detection
    if (detect24XPattern(x, y))
    {
        strategyCounters[SolverStrategy::Pattern24X]++;
        return true;
    }

    // 3. Try 3-4-X pattern detection
    if (detect34XPattern(x, y))
    {
        strategyCounters[SolverStrategy::Pattern34X]++;
        return true;
    }

    // 4. Try 2-2-X pattern detection
    if (detect22XPattern(x, y))
    {
        strategyCounters[SolverStrategy::Pattern22X]++;
        return true;
    }

    // 5. Try 3-3-X pattern detection
    if (detect33XPattern(x, y))
    {
        strategyCounters[SolverStrategy::Pattern33X]++;
        return true;
    }

    // 6. Try the 1-X-1 pattern detection
    if (detect1X1Pattern(x, y))
    {
        strategyCounters[SolverStrategy::Pattern1X1]++;
        return true;
    }

    // 7. Try the 2-X-2 pattern detection
    if (detect2X2Pattern(x, y))
    {
        strategyCounters[SolverStrategy::Pattern2X2]++;
        return true;
    }

    // 8. Try the generic 2-2-2 corner pattern
    if (detect222CornerPattern(x, y))
    {
        strategyCounters[SolverStrategy::Pattern222Corner]++;
        return true;
    }

    // 9. Try the 1-3-1 corner pattern
    if (detect131CornerPattern(x, y))
    {
        strategyCounters[SolverStrategy::Pattern131Corner]++;
        return true;
    }

    return false;
}

bool LibreMinesSolver::cleanGeneralPairRule(uchar x, uchar y)
{
    if (solverBoard[x][y] != SolverCellState::Safe)
    {
        return false;
    }

    if (detectPairRule(x, y))
    {
        strategyCounters[SolverStrategy::PairRule]++;
        return true;
    }

    return false;
}

bool LibreMinesSolver::cleanEnumeration()
{
    if (!isStrategyEnabled(SolverStrategy::Enumeration)) return false;

    // The cheaper tiers look at patterns made of two or three clues. This tier
    // treats the whole frontier as a constraint problem instead:
    //   - variables:   unknown cells that touch at least one revealed number
    //   - constraints: each revealed number ("exactly N of my unknown neighbours are mines")
    // Every mine layout satisfying all constraints is enumerated. A cell that is a
    // mine (or safe) in ALL layouts is a certain deduction.

    // --- Steps 1 and 2: gather the clues and index them by unknown cell -----
    QVector<EnumerationClue> clues;
    QHash<int, QVector<int>> cluesByCell;
    collectEnumerationClues(clues, cluesByCell);

    if (clues.isEmpty())
    {
        return false;
    }

    // --- Step 3: solve each group of connected clues ------------------------
    // Groups that share no unknown cell cannot influence each other, so they are
    // solved separately. This keeps every search small.
    bool changed = false;
    QVector<bool> alreadyGrouped(clues.size(), false);

    for (int seed = 0; seed < clues.size(); ++seed)
    {
        if (alreadyGrouped[seed])
        {
            continue;
        }

        if (checkForInterrupt())
        {
            return changed;   // solve() notices the interruption on its next iteration
        }

        // limit = 0: collect the complete group of connected clues
        const QVector<int> group = collectConnectedClues(seed, 0, clues, cluesByCell);
        for (int clueId : group)
        {
            alreadyGrouped[clueId] = true;
        }

        if (maxEnumerationClues <= 0 || group.size() <= maxEnumerationClues)
        {
            // Small enough: solve the whole group at once
            if (enumerateClueSubset(clues, group))
            {
                strategyCounters[SolverStrategy::Enumeration]++;
                changed = true;
            }
        }
        else
        {
            // Too large: solve one window per clue, each made of that clue and its nearest
            // neighbours. Windows ignore the clues outside them, which can only lose
            // deductions, never produce wrong ones.
            for (int windowSeed : group)
            {
                const QVector<int> window = collectConnectedClues(windowSeed, maxEnumerationClues, clues, cluesByCell);
                if (enumerateClueSubset(clues, window))
                {
                    strategyCounters[SolverStrategy::Enumeration]++;
                    changed = true;
                }
            }
        }
    }

    return changed;
}


bool LibreMinesSolver::cleanMineCounting()
{
    if (!isStrategyEnabled(SolverStrategy::MineCounting)) return false;

    // The mine counter tells how many mines are still hidden among ALL unknown cells.
    // Two ideas use it:
    //  1. No mines left -> every unknown cell is safe. As many mines as unknown cells -> all mines.
    //  2. Every valid layout of a group of connected clues has a certain number of mines. A
    //     combination of layouts whose totals cannot add up to the mines left is impossible.
    //     What is the same in all the remaining combinations is certain.

    // --- Step 1: the unknown cells and the mines still hidden ---------------
    QVector<int> unknownCells;
    for (int x = 0; x < cols; ++x)
    {
        for (int y = 0; y < rows; ++y)
        {
            if (solverBoard[x][y] == SolverCellState::Unknown)
            {
                unknownCells.append(cellIndex(x, y));
            }
        }
    }

    const int unknownTotal = unknownCells.size();
    const int minesLeft = static_cast<int>(nMines) - static_cast<int>(nCellsMarkedMine);

    if (unknownTotal == 0)
    {
        return false;
    }

    // A real board never has fewer unknown cells than hidden mines (or a negative count)
    if (minesLeft < 0 || minesLeft > unknownTotal)
    {
        qWarning() << "LibreMinesSolver: mine count is inconsistent with the solver board";
        return false;
    }

    // --- Step 2: the simple cases -------------------------------------------
    if (minesLeft == 0 || minesLeft == unknownTotal)
    {
        const SolverCellState state = (minesLeft == 0) ? SolverCellState::Safe : SolverCellState::Mine;
        bool changed = false;
        for (int cellId : unknownCells)
        {
            const QPair<uchar, uchar> cell = cellFromIndex(cellId);

            // Marking a safe zero cell can reveal other cells as a side effect
            if (solverBoard[cell.first][cell.second] != SolverCellState::Unknown)
            {
                continue;
            }

#if LIBREMINES_SOLVER_DEBUG
            qDebug() << "Mine counting: cell (" << cell.first << "," << cell.second << ") is"
                     << (state == SolverCellState::Safe ? "safe" : "a mine") << "(no other option left).";
#endif
            strategyCounters[SolverStrategy::MineCounting]++;
            markCell(cell, state);
            changed = true;
        }
        return changed;
    }

    // --- Step 3: enumerate every group of connected clues -------------------
    // Unlike cleanEnumeration(), each group is enumerated completely (no clue limit) and
    // the number of mines of every layout is recorded.
    QVector<EnumerationClue> clues;
    QHash<int, QVector<int>> cluesByCell;
    collectEnumerationClues(clues, cluesByCell);

    struct Group
    {
        QVector<int> cellIds;                              // local number -> board cell id
        LibreMinesConstraintEnumerator::Result layouts;    // every layout, filed by number of mines
    };

    QVector<Group> groups;
    QSet<int> groupedCells;                  // cells that belong to an enumerated group
    QVector<bool> alreadyGrouped(clues.size(), false);

    for (int seed = 0; seed < clues.size(); ++seed)
    {
        if (alreadyGrouped[seed])
        {
            continue;
        }

        if (checkForInterrupt())
        {
            return false;   // solve() notices the interruption on its next iteration
        }

        const QVector<int> clueGroup = collectConnectedClues(seed, 0, clues, cluesByCell);
        for (int clueId : clueGroup)
        {
            alreadyGrouped[clueId] = true;
        }

        Group group;
        LibreMinesConstraintEnumerator::Input input;
        buildEnumerationInput(clues, clueGroup, input.clueCells, input.clueMines, group.cellIds);
        input.nCells = group.cellIds.size();
        input.maxMines = minesLeft;   // a layout with more mines than are left is impossible: skip it

        group.layouts = LibreMinesConstraintEnumerator().run(input, EnumerationNodeBudget, true);

        // Too large for the search budget. Its cells simply stay among the "free" cells
        // below (their clues are ignored), which can only lose deductions.
        if (group.layouts.budgetExceeded)
        {
            continue;
        }

        if (group.layouts.solutions == 0)
        {
            qWarning() << "LibreMinesSolver: mine counting found a group with no valid layout; the solver board is inconsistent";
            return false;
        }

        for (int cellId : group.cellIds)
        {
            groupedCells.insert(cellId);
        }
        groups.append(group);
    }

    if (groups.isEmpty())
    {
        return false;   // no frontier: the mine counter alone says nothing more
    }

    // --- Step 4: which combinations of group totals fit the mine counter? ---
    // "Free" cells are the unknown cells outside the enumerated groups. Nothing constrains
    // them, so together they can hold any number of mines from 0 to freeCells. The mines
    // left must equal (sum of the groups' totals) + (mines among the free cells).
    const int freeCells = unknownTotal - groupedCells.size();
    const int nGroups = groups.size();

    // Adds one group to a set of reachable sums: sums[s] -> result[s + t] for every
    // total t the group can have. Sums above minesLeft are useless and dropped.
    auto addGroup = [minesLeft](const QVector<bool>& sums, const QVector<bool>& totals)
    {
        QVector<bool> result(minesLeft + 1, false);
        for (int sum = 0; sum <= minesLeft; ++sum)
        {
            if (!sums[sum])
            {
                continue;
            }
            for (int total = 0; total < totals.size() && sum + total <= minesLeft; ++total)
            {
                if (totals[total])
                {
                    result[sum + total] = true;
                }
            }
        }
        return result;
    };

    // prefix[i]: sums reachable by groups 0..i-1.  suffix[i]: sums reachable by groups i..end.
    QVector<QVector<bool>> prefix(nGroups + 1);
    QVector<QVector<bool>> suffix(nGroups + 1);
    prefix[0] = QVector<bool>(minesLeft + 1, false);
    prefix[0][0] = true;
    suffix[nGroups] = prefix[0];
    for (int i = 0; i < nGroups; ++i)
    {
        prefix[i + 1] = addGroup(prefix[i], groups[i].layouts.mineTotalPossible);
    }
    for (int i = nGroups - 1; i >= 0; --i)
    {
        suffix[i] = addGroup(suffix[i + 1], groups[i].layouts.mineTotalPossible);
    }

    // The free cells can absorb whatever the groups do not use, as long as it fits
    auto freeCellsCanHold = [minesLeft, freeCells](int groupsSum)
    {
        const int rest = minesLeft - groupsSum;
        return rest >= 0 && rest <= freeCells;
    };

    // How many mines the free cells can hold in the combinations that fit the counter
    int minFreeMines = freeCells + 1;
    int maxFreeMines = -1;
    for (int sum = 0; sum <= minesLeft; ++sum)
    {
        if (prefix[nGroups][sum] && freeCellsCanHold(sum))
        {
            minFreeMines = qMin(minFreeMines, minesLeft - sum);
            maxFreeMines = qMax(maxFreeMines, minesLeft - sum);
        }
    }

    // A real board always fits (the true layout does). If nothing fits, a previous deduction was wrong.
    if (maxFreeMines < 0)
    {
        qWarning() << "LibreMinesSolver: no combination of layouts fits the mine counter; the solver board is inconsistent";
        return false;
    }

    // --- Step 5: deduce ------------------------------------------------------
    bool changed = false;

    auto markIfUnknown = [this, &changed](int cellId, SolverCellState state)
    {
        const QPair<uchar, uchar> cell = cellFromIndex(cellId);

        // Marking a safe zero cell can reveal other cells as a side effect
        if (solverBoard[cell.first][cell.second] != SolverCellState::Unknown)
        {
            return;
        }

#if LIBREMINES_SOLVER_DEBUG
        qDebug() << "Mine counting: cell (" << cell.first << "," << cell.second << ") is"
                 << (state == SolverCellState::Safe ? "safe" : "a mine") << "in every layout that fits the mine counter.";
#endif
        strategyCounters[SolverStrategy::MineCounting]++;
        markCell(cell, state);
        changed = true;
    };

    // 5a. Cells inside the groups: keep only the totals that fit with the other groups
    for (int i = 0; i < nGroups; ++i)
    {
        const LibreMinesConstraintEnumerator::Result& layouts = groups[i].layouts;

        // Sums reachable by all the OTHER groups together
        QVector<bool> others(minesLeft + 1, false);
        for (int a = 0; a <= minesLeft; ++a)
        {
            if (!prefix[i][a])
            {
                continue;
            }
            for (int b = 0; a + b <= minesLeft; ++b)
            {
                if (suffix[i + 1][b])
                {
                    others[a + b] = true;
                }
            }
        }

        // Totals of this group that fit with at least one combination of the others
        QVector<bool> totalFits(layouts.mineTotalPossible.size(), false);
        for (int total = 0; total < totalFits.size(); ++total)
        {
            if (!layouts.mineTotalPossible[total])
            {
                continue;
            }
            for (int othersSum = 0; othersSum + total <= minesLeft; ++othersSum)
            {
                if (others[othersSum] && freeCellsCanHold(othersSum + total))
                {
                    totalFits[total] = true;
                    break;
                }
            }
        }

        // A cell is certain if it has the same value in every layout with a total that fits
        for (int local = 0; local < groups[i].cellIds.size(); ++local)
        {
            bool canBeMine = false;
            bool canBeSafe = false;
            for (int total = 0; total < totalFits.size(); ++total)
            {
                if (!totalFits[total])
                {
                    continue;
                }
                canBeMine = canBeMine || layouts.canBeMineWithTotal(total, local);
                canBeSafe = canBeSafe || layouts.canBeSafeWithTotal(total, local);
            }

            if (canBeMine && !canBeSafe)
            {
                markIfUnknown(groups[i].cellIds[local], SolverCellState::Mine);
            }
            else if (canBeSafe && !canBeMine)
            {
                markIfUnknown(groups[i].cellIds[local], SolverCellState::Safe);
            }
        }
    }

    // 5b. Free cells: if they must hold no mines they are all safe; if they must be full they are all mines
    if (freeCells > 0 && (maxFreeMines == 0 || minFreeMines == freeCells))
    {
        const SolverCellState state = (maxFreeMines == 0) ? SolverCellState::Safe : SolverCellState::Mine;
        for (int cellId : unknownCells)
        {
            if (!groupedCells.contains(cellId))
            {
                markIfUnknown(cellId, state);
            }
        }
    }

    return changed;
}

bool LibreMinesSolver::detect12XPattern(uchar x, uchar y)
{
    if (!isStrategyEnabled(SolverStrategy::Pattern12X)) return false;
    return detectMNXPattern(x, y, 1, 2);
}

bool LibreMinesSolver::detect11XPattern(uchar x, uchar y)
{
    if (!isStrategyEnabled(SolverStrategy::Pattern11X)) return false;
    return detectNNXPattern(x, y, 1);
}

bool LibreMinesSolver::detect22XPattern(uchar x, uchar y)
{
    if (!isStrategyEnabled(SolverStrategy::Pattern22X)) return false;
    return detectNNXPattern(x, y, 2);
}

bool LibreMinesSolver::detect33XPattern(uchar x, uchar y)
{
    if (!isStrategyEnabled(SolverStrategy::Pattern33X)) return false;
    return detectNNXPattern(x, y, 3);
}

bool LibreMinesSolver::detect13XPattern(uchar x, uchar y)
{
    if (!isStrategyEnabled(SolverStrategy::Pattern13X)) return false;
    return detectMNXPattern(x, y, 1, 3);
}

bool LibreMinesSolver::detect14XPattern(uchar x, uchar y)
{
    if (!isStrategyEnabled(SolverStrategy::Pattern14X)) return false;
    return detectMNXPattern(x, y, 1, 4);
}

bool LibreMinesSolver::detect23XPattern(uchar x, uchar y)
{
    if (!isStrategyEnabled(SolverStrategy::Pattern23X)) return false;
    return detectMNXPattern(x, y, 2, 3);
}

bool LibreMinesSolver::detect24XPattern(uchar x, uchar y)
{
    if (!isStrategyEnabled(SolverStrategy::Pattern24X)) return false;
    return detectMNXPattern(x, y, 2, 4);
}

bool LibreMinesSolver::detect34XPattern(uchar x, uchar y)
{
    if (!isStrategyEnabled(SolverStrategy::Pattern34X)) return false;
    return detectMNXPattern(x, y, 3, 4);
}

bool LibreMinesSolver::detect1X1Pattern(uchar x, uchar y)
{
    if (!isStrategyEnabled(SolverStrategy::Pattern1X1)) return false;
    return detectNXNPattern(x, y, 1);
}

bool LibreMinesSolver::detect2X2Pattern(uchar x, uchar y)
{
    if (!isStrategyEnabled(SolverStrategy::Pattern2X2)) return false;
    return detectNXNPattern(x, y, 2);
}

bool LibreMinesSolver::detectPairRule(uchar x, uchar y)
{
    if (!isStrategyEnabled(SolverStrategy::PairRule)) return false;

    if (solverBoard[x][y] != SolverCellState::Safe)
    {
        return false;
    }

    // Clues share unknown cells only if they are within 2 cells of each other
    // in both axes, so a 5x5 window covers every useful partner.
    for (int dy = -2; dy <= 2; ++dy)
    {
        for (int dx = -2; dx <= 2; ++dx)
        {
            if (dx == 0 && dy == 0) continue;

            const uchar nx = x + dx;
            const uchar ny = y + dy;
            if (nx >= cols || ny >= rows) continue;

            if (applyPairRule(x, y, nx, ny))
            {
                return true;   // state changed: clue data is stale, let the outer loop re-run
            }
        }
    }
    return false;
}

bool LibreMinesSolver::detect131CornerPattern(uchar x, uchar y)
{
    if (!isStrategyEnabled(SolverStrategy::Pattern131Corner)) return false;

    if (solverBoard[x][y] != SolverCellState::Safe ||
        countRemainingMines(x, y) != 3)
    {
        return false;
    }

    // Check the four perpendicular pairs of neighboring clues.
    if (x >= 1 && y >= 1 &&
        apply131CornerPatternLogic(x, y, x - 1, y, x, y - 1))
    {
        return true;
    }

    if (x < cols - 1 && y >= 1 &&
        apply131CornerPatternLogic(x, y, x + 1, y, x, y - 1))
    {
        return true;
    }

    if (x >= 1 && y < rows - 1 &&
        apply131CornerPatternLogic(x, y, x - 1, y, x, y + 1))
    {
        return true;
    }

    if (x < cols - 1 && y < rows - 1 &&
        apply131CornerPatternLogic(x, y, x + 1, y, x, y + 1))
    {
        return true;
    }

    return false;
}


bool LibreMinesSolver::detect222CornerPattern(uchar x, uchar y)
{
    if (!isStrategyEnabled(SolverStrategy::Pattern222Corner)) return false;

    if (solverBoard[x][y] != SolverCellState::Safe ||
        countRemainingMines(x, y) != 2)
    {
        return false;
    }

    // Check the four perpendicular pairs of neighboring clues.
    if (x >= 1 && y >= 1 &&
        apply222CornerPatternLogic(x, y, x - 1, y, x, y - 1))
    {
        return true;
    }

    if (x < cols - 1 && y >= 1 &&
        apply222CornerPatternLogic(x, y, x + 1, y, x, y - 1))
    {
        return true;
    }

    if (x >= 1 && y < rows - 1 &&
        apply222CornerPatternLogic(x, y, x - 1, y, x, y + 1))
    {
        return true;
    }

    if (x < cols - 1 && y < rows - 1 &&
        apply222CornerPatternLogic(x, y, x + 1, y, x, y + 1))
    {
        return true;
    }

    return false;
}

bool LibreMinesSolver::detectMNXPattern(uchar x, uchar y, int adjacentMines, int remainingMines)
{
    // Current cell must have exactly `remainingMines` remaining unsolved neighbors
    if (countRemainingMines(x, y) != remainingMines)
    {
        return false;
    }

    // Look for M-N-X pattern in horizontal direction (left)
    if (x >= 1)
    {
        if (applyMNXPatternLogic(x, y, x-1, y, adjacentMines, remainingMines))
        {
            return true;
        }
    }
    
    // Look for X-N-1 pattern in horizontal direction (right)
    if (x < cols - 1)
    {
        if (applyMNXPatternLogic(x, y, x+1, y, adjacentMines, remainingMines))
        {
            return true;
        }
    }
    
    // Look for M-N-X pattern in vertical direction (top)
    if (y >= 1)
    {
        if (applyMNXPatternLogic(x, y, x, y-1, adjacentMines, remainingMines))
        {
            return true;
        }
    }
    
    // Look for X-N-M pattern in vertical direction (bottom)
    if (y < rows - 1)
    {
        if (applyMNXPatternLogic(x, y, x, y+1, adjacentMines, remainingMines))
        {
            return true;
        }
    }
    
    return false;
}

bool LibreMinesSolver::detectNNXPattern(uchar x, uchar y, int remainingMines)
{
    // Current cell must have exactly `remainingMines` remaining unsolved neighbor
    if (countRemainingMines(x, y) != remainingMines)
    {
        return false;
    }
    
    // Look for N-N-X pattern in horizontal direction (left)
    if (x >= 1)
    {
        if (applyNNXPatternLogic(x, y, x-1, y, remainingMines))
        {
            return true;
        }
    }
    
    // Look for N-N-X pattern in horizontal direction (right)
    if (x < cols - 1)
    {
        if (applyNNXPatternLogic(x, y, x+1, y, remainingMines))
        {
            return true;
        }
    }
    
    // Look for N-N-X pattern in vertical direction (top)
    if (y >= 1)
    {
        if (applyNNXPatternLogic(x, y, x, y-1, remainingMines))
        {
            return true;
        }
    }
    
    // Look for N-N-X pattern in vertical direction (bottom)
    if (y < rows - 1)
    {
        if (applyNNXPatternLogic(x, y, x, y+1, remainingMines))
        {
            return true;
        }
    }
    
    return false;
}

bool LibreMinesSolver::detectNXNPattern(uchar x, uchar y, int remainingMines)
{
    if (countRemainingMines(x, y) != remainingMines)
    {
        return false;
    }

    // Look for N-X-N pattern in horizontal direction (left).
    if (x >= 2)
    {
        if (applyNNXPatternLogic(x, y, x - 2, y, remainingMines))
        {
            return true;
        }
    }

    // Look for N-X-N pattern in horizontal direction (right).
    if (x + 2 < cols)
    {
        if (applyNNXPatternLogic(x, y, x + 2, y, remainingMines))
        {
            return true;
        }
    }

    // Look for N-X-N pattern in vertical direction (top).
    if (y >= 2)
    {
        if (applyNNXPatternLogic(x, y, x, y - 2, remainingMines))
        {
            return true;
        }
    }

    // Look for N-X-N pattern in vertical direction (bottom).
    if (y + 2 < rows)
    {
        if (applyNNXPatternLogic(x, y, x, y + 2, remainingMines))
        {
            return true;
        }
    }

    return false;
}

int LibreMinesSolver::countRemainingNeighbors(uchar x, uchar y, SolverCellState cellState, QSet<QPair<uchar, uchar>>* outSet) const
{
    int matchingNeighbors = 0;
    
    // Count neighbors around the cell
    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            if (dx == 0 && dy == 0) continue;
            uchar nx = x + dx;
            uchar ny = y + dy;
            if (nx < cols && ny < rows && solverBoard[nx][ny] == cellState)
            {
                matchingNeighbors++;
                if (outSet)
                {
                    outSet->insert({nx, ny});
                }
            }
        }
    }
    
    return matchingNeighbors;
}

int LibreMinesSolver::countRemainingMines(uchar x, uchar y) const
{
    // Count the number of remaining mines needed for the cell at (x, y)
    int currentMines = countRemainingNeighbors(x, y, SolverCellState::Mine, nullptr);
    return static_cast<int>(board[x][y].value) - currentMines;
}

bool LibreMinesSolver::applyMNXPatternLogic(uchar middleX, uchar middleY, uchar adjacentX, uchar adjacentY, int adjacentMines, int remainingMines)
{
    if (solverBoard[middleX][middleY] != SolverCellState::Safe ||
        solverBoard[adjacentX][adjacentY] != SolverCellState::Safe ||
        remainingMines <= adjacentMines || adjacentMines < 0 ||
        countRemainingMines(middleX, middleY) != remainingMines ||
        countRemainingMines(adjacentX, adjacentY) != adjacentMines)
    {
        return false;
    }

    // Find neighbors of middle and adjacent cells
    QSet<QPair<uchar, uchar>> middleNeighbors;
    QSet<QPair<uchar, uchar>> adjacentNeighbors;
    countRemainingNeighbors(middleX, middleY, SolverCellState::Unknown, &middleNeighbors);
    countRemainingNeighbors(adjacentX, adjacentY, SolverCellState::Unknown, &adjacentNeighbors);
    
    // Find shared and unique neighbors
    QSet<QPair<uchar, uchar>> sharedNeighbors = middleNeighbors & adjacentNeighbors;
    QSet<QPair<uchar, uchar>> middleUniqueNeighbors = middleNeighbors - sharedNeighbors;
    QSet<QPair<uchar, uchar>> adjacentUniqueNeighbors = adjacentNeighbors - sharedNeighbors;
    
    // Apply M-N-X pattern logic:
    // If the middle-only candidates equal the difference in remaining mines,
    // they must all be mines; candidates unique to the adjacent cell are safe.
    if (sharedNeighbors.size() >= adjacentMines &&
        middleUniqueNeighbors.size() == remainingMines - adjacentMines)
    {
        for (const QPair<uchar, uchar>& uniqueCell : middleUniqueNeighbors)
        {
            markCell(uniqueCell, SolverCellState::Mine);
        }
        for (const QPair<uchar, uchar>& uniqueCell : adjacentUniqueNeighbors)
        {
            markCell(uniqueCell, SolverCellState::Safe);
        }
        return true;
    }
    
    return false;
}

bool LibreMinesSolver::applyNNXPatternLogic(uchar middleX, uchar middleY, uchar adjX, uchar adjY, int remainingMines)
{
    if (solverBoard[middleX][middleY] != SolverCellState::Safe ||
        solverBoard[adjX][adjY] != SolverCellState::Safe ||
        remainingMines < 0 ||
        countRemainingMines(middleX, middleY) != remainingMines ||
        countRemainingMines(adjX, adjY) != remainingMines)
    {
        return false;
    }

    // Find neighbors of first and second cell
    QSet<QPair<uchar, uchar>> adjNeighbors;
    QSet<QPair<uchar, uchar>> midNeighbors;
    countRemainingNeighbors(middleX, middleY, SolverCellState::Unknown, &midNeighbors);
    countRemainingNeighbors(adjX, adjY, SolverCellState::Unknown, &adjNeighbors);
    
    // Find shared and unique neighbors
    QSet<QPair<uchar, uchar>> sharedNeighbors = midNeighbors & adjNeighbors;
    QSet<QPair<uchar, uchar>> midUniqueNeighbors = midNeighbors - sharedNeighbors;
    
    // Apply N-N-X pattern logic:
    // If sharedNeighbors == adjNeighbors -> All midUniqueNeighbors are safe
    if (sharedNeighbors == adjNeighbors && !midUniqueNeighbors.isEmpty())
    {
        for (const QPair<uchar, uchar>& cell : midUniqueNeighbors)
        {
            markCell(cell, SolverCellState::Safe);
        }
        return true;
    }

    return false;
}

bool LibreMinesSolver::apply131CornerPatternLogic(uchar middleX,
                                                  uchar middleY,
                                                  uchar firstX,
                                                  uchar firstY,
                                                  uchar secondX,
                                                  uchar secondY)
{
    if (solverBoard[middleX][middleY] != SolverCellState::Safe ||
        solverBoard[firstX][firstY] != SolverCellState::Safe ||
        solverBoard[secondX][secondY] != SolverCellState::Safe ||
        countRemainingMines(middleX, middleY) != 3 ||
        countRemainingMines(firstX, firstY) != 1 ||
        countRemainingMines(secondX, secondY) != 1)
    {
        return false;
    }

    QSet<QPair<uchar, uchar>> middleNeighbors;
    QSet<QPair<uchar, uchar>> firstNeighbors;
    QSet<QPair<uchar, uchar>> secondNeighbors;
    countRemainingNeighbors(middleX, middleY, SolverCellState::Unknown, &middleNeighbors);
    countRemainingNeighbors(firstX, firstY, SolverCellState::Unknown, &firstNeighbors);
    countRemainingNeighbors(secondX, secondY, SolverCellState::Unknown, &secondNeighbors);

    if (firstNeighbors.isEmpty() || secondNeighbors.isEmpty() ||
        !(firstNeighbors & secondNeighbors).isEmpty())
    {
        return false;
    }

    const QSet<QPair<uchar, uchar>> middleUniqueNeighbors = middleNeighbors - firstNeighbors - secondNeighbors;

    if (middleUniqueNeighbors.size() != 1)
    {
        return false;
    }

    const QPair<uchar, uchar>& cell = *middleUniqueNeighbors.cbegin();

    // Mark the unique middle neighbor as a mine
    markCell(cell, SolverCellState::Mine);

    const QSet<QPair<uchar, uchar>> firstUniqueNeighbors = firstNeighbors - middleNeighbors;
    const QSet<QPair<uchar, uchar>> secondUniqueNeighbors = secondNeighbors - middleNeighbors;

    // Mark the unique neighbors of the first and second cells as safe
    for (const QPair<uchar, uchar>& uniqueCell : firstUniqueNeighbors)
    {
        markCell(uniqueCell, SolverCellState::Safe);
    }

    // Mark the unique neighbors of the second cell as safe
    for (const QPair<uchar, uchar>& uniqueCell : secondUniqueNeighbors)
    {
        markCell(uniqueCell, SolverCellState::Safe);
    }

    return true;
}

bool LibreMinesSolver::apply222CornerPatternLogic(uchar middleX,
                                                  uchar middleY,
                                                  uchar firstX,
                                                  uchar firstY,
                                                  uchar secondX,
                                                  uchar secondY)
{
    if (solverBoard[middleX][middleY] != SolverCellState::Safe ||
        solverBoard[firstX][firstY] != SolverCellState::Safe ||
        solverBoard[secondX][secondY] != SolverCellState::Safe ||
        countRemainingMines(middleX, middleY) != 2 ||
        countRemainingMines(firstX, firstY) != 2 ||
        countRemainingMines(secondX, secondY) != 2)
    {
        return false;
    }

    QSet<QPair<uchar, uchar>> middleNeighbors;
    QSet<QPair<uchar, uchar>> firstNeighbors;
    QSet<QPair<uchar, uchar>> secondNeighbors;
    countRemainingNeighbors(middleX, middleY, SolverCellState::Unknown, &middleNeighbors);
    countRemainingNeighbors(firstX, firstY, SolverCellState::Unknown, &firstNeighbors);
    countRemainingNeighbors(secondX, secondY, SolverCellState::Unknown, &secondNeighbors);

    if (firstNeighbors.isEmpty() || secondNeighbors.isEmpty() ||
        !(firstNeighbors & secondNeighbors).isEmpty())
    {
        return false;
    }

    const QSet<QPair<uchar, uchar>> middleUniqueNeighbors = middleNeighbors - firstNeighbors - secondNeighbors;

    if (middleUniqueNeighbors.size() != 1)
    {
        return false;
    }

    const QPair<uchar, uchar>& middleCell = *middleUniqueNeighbors.cbegin();

    const QSet<QPair<uchar, uchar>> firstUniqueNeighbors = firstNeighbors - middleNeighbors;
    const QSet<QPair<uchar, uchar>> secondUniqueNeighbors = secondNeighbors - middleNeighbors;
    if (firstUniqueNeighbors.size() != 1 || secondUniqueNeighbors.size() != 1)
    {
        return false;
    }

    // Mark the unique middle neighbor as a safe
    markCell(middleCell, SolverCellState::Safe);

    // With two remaining mines in each arm and one unique unknown per arm,
    // each arm needs at least one mine from the shared neighbors. The middle
    // clue has exactly two mines, so both shared groups contribute one mine,
    // forcing the unique middle cell safe and both arm-unique cells mined.
    const QPair<uchar, uchar>& firstCell = *firstUniqueNeighbors.cbegin();
    markCell(firstCell, SolverCellState::Mine);

    const QPair<uchar, uchar>& secondCell = *secondUniqueNeighbors.cbegin();
    markCell(secondCell, SolverCellState::Mine);

    return true;
}

bool LibreMinesSolver::applyPairRule(uchar ax, uchar ay, uchar bx, uchar by)
{
    if (solverBoard[ax][ay] != SolverCellState::Safe ||
        solverBoard[bx][by] != SolverCellState::Safe)
    {
        return false;
    }

    QSet<QPair<uchar, uchar>> unknownA;
    QSet<QPair<uchar, uchar>> unknownB;
    countRemainingNeighbors(ax, ay, SolverCellState::Unknown, &unknownA);
    countRemainingNeighbors(bx, by, SolverCellState::Unknown, &unknownB);

    const QSet<QPair<uchar, uchar>> common = unknownA & unknownB;
    if (common.isEmpty())
    {
        return false;   // no shared cells: a pair can't add anything beyond the basic rules
    }

    const QSet<QPair<uchar, uchar>> onlyA = unknownA - common;
    const QSet<QPair<uchar, uchar>> onlyB = unknownB - common;
    if (onlyA.isEmpty() && onlyB.isEmpty())
    {
        return false;   // identical unknown sets: nothing to separate
    }

    const int kA = countRemainingMines(ax, ay);
    const int kB = countRemainingMines(bx, by);
    const int d  = kA - kB;                       // mines(onlyA) - mines(onlyB)
    const int nA = static_cast<int>(onlyA.size());
    const int nB = static_cast<int>(onlyB.size());

    const int aMin = std::max(0, d);
    const int aMax = std::min(nA, d + nB);
    const int bMin = std::max(0, -d);
    const int bMax = std::min(nB, nA - d);

    bool changed = false;

    if (nA > 0)
    {
        if (aMax == 0)         changed |= markAllCells(onlyA, SolverCellState::Safe);
        else if (aMin == nA)   changed |= markAllCells(onlyA, SolverCellState::Mine);
    }

    if (nB > 0)
    {
        if (bMax == 0)         changed |= markAllCells(onlyB, SolverCellState::Safe);
        else if (bMin == nB)   changed |= markAllCells(onlyB, SolverCellState::Mine);
    }

#if LIBREMINES_SOLVER_DEBUG
    if (changed)
    {
        qDebug() << "Pair rule applied between (" << ax << "," << ay << ") and (" << bx << "," << by << ").";
    }
#endif

    return changed;
}

void LibreMinesSolver::collectEnumerationClues(QVector<EnumerationClue>& clues,
                                               QHash<int, QVector<int>>& cluesByCell) const
{
    // The board is scanned instead of cellsNeedingTreatment so the order is deterministic.
    clues.clear();
    cluesByCell.clear();

    for (int x = 0; x < cols; ++x)
    {
        for (int y = 0; y < rows; ++y)
        {
            if (solverBoard[x][y] != SolverCellState::Safe)
            {
                continue;
            }

            QSet<QPair<uchar, uchar>> unknownNeighbors;
            if (countRemainingNeighbors(x, y, SolverCellState::Unknown, &unknownNeighbors) == 0)
            {
                continue;   // nothing left to learn from this number
            }

            EnumerationClue clue;
            clue.remainingMines = countRemainingMines(x, y);   // the number minus mines already found around it
            for (const QPair<uchar, uchar>& neighbor : unknownNeighbors)
            {
                clue.unknownCells.append(cellIndex(neighbor.first, neighbor.second));
            }
            clues.append(clue);
        }
    }

    // Index which clues touch each unknown cell
    for (int clueId = 0; clueId < clues.size(); ++clueId)
    {
        for (int cellId : clues[clueId].unknownCells)
        {
            cluesByCell[cellId].append(clueId);
        }
    }
}

void LibreMinesSolver::buildEnumerationInput(const QVector<EnumerationClue>& clues,
                                             const QVector<int>& subset,
                                             std::vector<std::vector<int>>& clueCells,
                                             QVector<int>& clueMines,
                                             QVector<int>& cellIds) const
{
    // Translate the board cells used by these clues into compact local numbers
    // 0..n-1, which is what the enumerator works with.
    QHash<int, int> localIndex;   // board cell id -> local number
    cellIds.clear();

    for (int clueId : subset)
    {
        std::vector<int> localCells;
        for (int cellId : clues[clueId].unknownCells)
        {
            if (!localIndex.contains(cellId))
            {
                localIndex.insert(cellId, cellIds.size());
                cellIds.append(cellId);
            }
            localCells.push_back(localIndex.value(cellId));
        }
        clueCells.push_back(localCells);
        clueMines.append(clues[clueId].remainingMines);
    }
}

bool LibreMinesSolver::enumerateClueSubset(const QVector<EnumerationClue>& clues, const QVector<int>& subset)
{
    // Translate the clues into the enumerator's input (cells renumbered 0..n-1)
    LibreMinesConstraintEnumerator::Input input;
    QVector<int> cellIds;         // local number -> board cell id
    buildEnumerationInput(clues, subset, input.clueCells, input.clueMines, cellIds);
    input.nCells = cellIds.size();

    const LibreMinesConstraintEnumerator::Result result = LibreMinesConstraintEnumerator().run(input, EnumerationNodeBudget);

    // A search cut short has not seen every layout, so "always a mine / always safe"
    // proves nothing. Discard it entirely.
    if (result.budgetExceeded)
    {
        return false;
    }

    // A real board always has at least one valid layout (the true one). Finding none
    // means a previous deduction was wrong: report it instead of marking anything.
    if (result.solutions == 0 && !result.saturated)
    {
        qWarning() << "LibreMinesSolver: enumeration found no valid layout; the solver board is inconsistent";
        return false;
    }

    bool changed = false;
    for (int local = 0; local < cellIds.size(); ++local)
    {
        const QPair<uchar, uchar> cell = cellFromIndex(cellIds[local]);

        // Marking a safe zero cell can reveal other cells of this subset as a side
        // effect, so skip cells that are no longer unknown.
        if (solverBoard[cell.first][cell.second] != SolverCellState::Unknown)
        {
            continue;
        }

        if (result.isMineInAllSolutions(local))
        {
#if LIBREMINES_SOLVER_DEBUG
            qDebug() << "Enumeration: cell (" << cell.first << "," << cell.second << ") is a mine in all solutions.";
#endif
            markCell(cell, SolverCellState::Mine);
            changed = true;
        }
        else if (result.isSafeInAllSolutions(local))
        {
#if LIBREMINES_SOLVER_DEBUG
            qDebug() << "Enumeration: cell (" << cell.first << "," << cell.second << ") is safe in all solutions.";
#endif
            markCell(cell, SolverCellState::Safe);
            changed = true;
        }
    }

    return changed;
}

QVector<int> LibreMinesSolver::collectConnectedClues(int seed,
                                                     int limit,
                                                     const QVector<EnumerationClue>& clues,
                                                     const QHash<int, QVector<int>>& cluesByCell) const
{
    // Breadth-first walk over clues: two clues are neighbours when they share an
    // unknown cell. Visiting order = distance from the seed, so a limited result
    // is the group of clues closest to it.
    QVector<int> collected;
    QSet<int> seen;
    QQueue<int> pending;

    seen.insert(seed);
    pending.enqueue(seed);

    while (!pending.isEmpty() && (limit <= 0 || collected.size() < limit))
    {
        const int current = pending.dequeue();
        collected.append(current);

        for (int cellId : clues[current].unknownCells)
        {
            for (int other : cluesByCell.value(cellId))
            {
                if (!seen.contains(other))
                {
                    seen.insert(other);
                    pending.enqueue(other);
                }
            }
        }
    }

    return collected;
}

int LibreMinesSolver::cellIndex(uchar x, uchar y) const
{
    return static_cast<int>(x) * rows + y;
}

QPair<uchar, uchar> LibreMinesSolver::cellFromIndex(int index) const
{
    return QPair<uchar, uchar>(static_cast<uchar>(index / rows), static_cast<uchar>(index % rows));
}

bool LibreMinesSolver::checkForInterrupt()
{
    QMutexLocker locker(&cmdMutex);
    return cmd == SolverCommand::CmdInterrupt;
}

void  LibreMinesSolver::setInternalState(SolverState newState) 
{
    QMutexLocker locker(&stateMutex);
    state = newState;
}

std::vector<std::vector<LibreMinesGameEngine::CellGameEngine> > LibreMinesSolver::getBoard() const
{
    return board;
}

short LibreMinesSolver::getNMines() const
{
    return nMines;
}

void LibreMinesSolver::setNMines(short newNMines)
{
    nMines = newNMines;
}

uchar LibreMinesSolver::getSafeY() const
{
    return safeY;
}

void LibreMinesSolver::setSafeY(uchar newSafeY)
{
    safeY = newSafeY;
}

uchar LibreMinesSolver::getSafeX() const
{
    return safeX;
}

void LibreMinesSolver::setSafeX(uchar newSafeX)
{
    safeX = newSafeX;
}

uchar LibreMinesSolver::getCols() const
{
    return cols;
}

void LibreMinesSolver::setCols(uchar newCols)
{
    cols = newCols;
}

uchar LibreMinesSolver::getRows() const
{
    return rows;
}

void LibreMinesSolver::setRows(uchar newRows)
{
    rows = newRows;
}

LibreMinesSolver::SolverStrategy LibreMinesSolver::getStrategies() const
{
    return strategies;
}

void LibreMinesSolver::setStrategies(SolverStrategy newStrategies)
{
    strategies = newStrategies;
}

bool LibreMinesSolver::isStrategyEnabled(SolverStrategy strategy) const
{
    return (static_cast<quint64>(strategies) & static_cast<quint64>(strategy)) != 0;
}

LibreMinesSolver::SolverState LibreMinesSolver::getSolverState() const
{
    QMutexLocker locker(&stateMutex);
    return state;
}

int LibreMinesSolver::getMaxEnumerationClues() const
{
    return maxEnumerationClues;
}

void LibreMinesSolver::setMaxEnumerationClues(int newMaxEnumerationClues)
{
    maxEnumerationClues = qMax(0, newMaxEnumerationClues);
}

void LibreMinesSolver::interrupt()
{
    QMutexLocker locker(&cmdMutex);
    cmd = SolverCommand::CmdInterrupt;
}
