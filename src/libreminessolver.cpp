#include "libreminessolver.h"

#include "gameboardrandomgenerator.h"
#include <QMutexLocker>
#include "libreminesconstraintenumerator.hpp"

#define LIBREMINES_SOLVER_DEBUG 1

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
        return true;
    }
    
    // 2. Try 1-1-X pattern detection
    if (detect11XPattern(x, y))
    {
        return true;
    }

    // 3. Try 1-3-X pattern detection
    if (detect13XPattern(x, y))
    {
        return true;
    }

    // 4. Try 1-4-X pattern detection
    if (detect14XPattern(x, y))
    {
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
        return true;
    }

    // 2. Try 2-4-X pattern detection
    if (detect24XPattern(x, y))
    {
        return true;
    }

    // 3. Try 3-4-X pattern detection
    if (detect34XPattern(x, y))
    {
        return true;
    }

    // 4. Try 2-2-X pattern detection
    if (detect22XPattern(x, y))
    {
        return true;
    }

    // 5. Try 3-3-X pattern detection
    if (detect33XPattern(x, y))
    {
        return true;
    }

    // 6. Try the 1-X-1 pattern detection
    if (detect1X1Pattern(x, y))
    {
        return true;
    }

    // 7. Try the 2-X-2 pattern detection
    if (detect2X2Pattern(x, y))
    {
        return true;
    }

    // 8. Try the generic 2-2-2 corner pattern
    if (detect222CornerPattern(x, y))
    {
        return true;
    }

    // 9. Try the 1-3-1 corner pattern
    if (detect131CornerPattern(x, y))
    {
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

    // --- Step 1: gather the clues -------------------------------------------
    // The board is scanned instead of cellsNeedingTreatment so the order is deterministic.
    QVector<EnumerationClue> clues;
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

    if (clues.isEmpty())
    {
        return false;
    }

    // --- Step 2: index which clues touch each unknown cell ------------------
    QHash<int, QVector<int>> cluesByCell;
    for (int clueId = 0; clueId < clues.size(); ++clueId)
    {
        for (int cellId : clues[clueId].unknownCells)
        {
            cluesByCell[cellId].append(clueId);
        }
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
                    changed = true;
                }
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

bool LibreMinesSolver::enumerateClueSubset(const QVector<EnumerationClue>& clues, const QVector<int>& subset)
{
    // Translate the board cells used by these clues into compact local numbers
    // 0..n-1, which is what the enumerator works with.
    QHash<int, int> localIndex;   // board cell id -> local number
    QVector<int> cellIds;         // local number  -> board cell id

    LibreMinesConstraintEnumerator::Input input;
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
        input.clueCells.push_back(localCells);
        input.clueMines.append(clues[clueId].remainingMines);
    }
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
