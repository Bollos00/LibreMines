#include "libreminessolver.h"

#include "gameboardrandomgenerator.h"
#include <QMutexLocker>

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

LibreMinesSolver::LibreMinesSolver(QObject *parent)
    : QThread{parent}, state(SolverState::NotStarted)
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
            uchar x = cellCoord.first;
            uchar y = cellCoord.second;
            
            // Apply basic cleaning logic to this cell
            if (cleanBasic(x, y))
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
                uchar x = cellCoord.first;
                uchar y = cellCoord.second;
                
                // Apply pattern recognition to this cell
                if (cleanBasicPatterns(x, y))
                {
                    globalChanges = true;
                    // Keep the cell in treatment set as patterns might reveal more info
                }
            }
        }

        // If bsic and pattern cleaning didn't make progress, try advanced patterns
        if (!globalChanges)
        {
#if LIBREMINES_SOLVER_DEBUG
            qDebug() << "Applying advanced pattern cleaning...";
#endif
            QSet<QPair<uchar, uchar>> advancedPatternTreatmentSet = cellsNeedingTreatment;

            for (const QPair<uchar, uchar>& cellCoord : advancedPatternTreatmentSet)
            {
                uchar x = cellCoord.first;
                uchar y = cellCoord.second;
                
                // Apply advanced pattern recognition to this cell
                if (cleanAdvancedPatterns(x, y))
                {
                    globalChanges = true;
                    // Keep the cell in treatment set as patterns might reveal more info
                }
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
    const bool result = nCellsMarkedSafe + nCellsMarkedMine == totalCells;

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
    bool changes = false;
    
    // 1. Try 1-2-X pattern detection
    if (detect12XPattern(x, y))
    {
        changes = true;
    }
    
    // 2. Try 1-1-X pattern detection
    if (detect11XPattern(x, y))
    {
        changes = true;
    }

    // 3. Try 1-3-X pattern detection
    if (detect13XPattern(x, y))
    {
        changes = true;
    }

    // 4. Try 1-4-X pattern detection
    if (detect14XPattern(x, y))
    {
        changes = true;
    }

    return changes;
}

bool LibreMinesSolver::cleanAdvancedPatterns(uchar x, uchar y)
{
    if (solverBoard[x][y] != SolverCellState::Safe)
    {
        return false;
    }

    // Try different pattern recognition algorithms
    bool changes = false;

    // 1. Try 2-3-X pattern detection
    if (detect23XPattern(x, y))
    {
        changes = true;
    }

    // 2. Try 2-4-X pattern detection
    if (detect24XPattern(x, y))
    {
        changes = true;
    }

    // 3. Try 3-4-X pattern detection
    if (detect34XPattern(x, y))
    {
        changes = true;
    }

    // 4. Try 2-2-X pattern detection
    if (detect22XPattern(x, y))
    {
        changes = true;
    }

    // 5. Try 3-3-X pattern detection
    if (detect33XPattern(x, y))
    {
        changes = true;
    }

    // 6. Try the 1-X-1 pattern detection
    if (detect1X1Pattern(x, y))
    {
        changes = true;
    }

    // 7. Try the 2-X-2 pattern detection
    if (detect2X2Pattern(x, y))
    {
        changes = true;
    }

    // 8. Try the generic 2-2-2 corner pattern
    if (detect222CornerPattern(x, y))
    {
        changes = true;
    }

    // 9. Try the 1-3-1 corner pattern
    if (detect131CornerPattern(x, y))
    {
        changes = true;
    }

    return changes;
}

bool LibreMinesSolver::detect12XPattern(uchar x, uchar y)
{
    return detectMNXPattern(x, y, 1, 2);
}

bool LibreMinesSolver::detect11XPattern(uchar x, uchar y)
{
    return detectNNXPattern(x, y, 1);
}

bool LibreMinesSolver::detect22XPattern(uchar x, uchar y)
{
    return detectNNXPattern(x, y, 2);
}

bool LibreMinesSolver::detect33XPattern(uchar x, uchar y)
{
    return detectNNXPattern(x, y, 3);
}

bool LibreMinesSolver::detect13XPattern(uchar x, uchar y)
{
    return detectMNXPattern(x, y, 1, 3);
}

bool LibreMinesSolver::detect14XPattern(uchar x, uchar y)
{
    return detectMNXPattern(x, y, 1, 4);
}

bool LibreMinesSolver::detect23XPattern(uchar x, uchar y)
{
    return detectMNXPattern(x, y, 2, 3);
}

bool LibreMinesSolver::detect24XPattern(uchar x, uchar y)
{
    return detectMNXPattern(x, y, 2, 4);
}

bool LibreMinesSolver::detect34XPattern(uchar x, uchar y)
{
    return detectMNXPattern(x, y, 3, 4);
}

bool LibreMinesSolver::detect1X1Pattern(uchar x, uchar y)
{
    return detectNXNPattern(x, y, 1);
}

bool LibreMinesSolver::detect2X2Pattern(uchar x, uchar y)
{
    return detectNXNPattern(x, y, 2);
}

bool LibreMinesSolver::detect131CornerPattern(uchar x, uchar y)
{
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

LibreMinesSolver::SolverState LibreMinesSolver::getSolverState() const
{
    QMutexLocker locker(&stateMutex);
    return state;
}

void LibreMinesSolver::interrupt()
{
    QMutexLocker locker(&cmdMutex);
    cmd = SolverCommand::CmdInterrupt;
}
