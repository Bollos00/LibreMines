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

    bool result = true;
    // Check if all cells are resolved
    for (uchar i = 0; i < cols; ++i)
    {
        for (uchar j = 0; j < rows; ++j)
        {
            if (solverBoard[i][j] == SolverCellState::Unknown)
            {
                result = false;
                break;
            }
        }
        if (!result) break;
    }

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

    qDebug() << "Final cellsNeedingTreatment: " << cellsNeedingTreatment;
#endif

    return result;
}

void LibreMinesSolver::initialStep()
{
    cleanZero(safeX, safeY);
}

bool LibreMinesSolver::cleanZero(uchar x, uchar y)
{
    if (solverBoard[x][y] != SolverCellState::Unknown || board[x][y].value != CellValue::ZERO)
    {
        return false;
    }

    // Flag the specified cell
    solverBoard[x][y] = SolverCellState::Safe;

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
                // If unable to clean as zero, this cell was not zero, so mark it as safe and add to treatment set
                if (!cleanZero(nx, ny))
                {
                    // Mark as safe if not zero and add to treatment set
                    solverBoard[nx][ny] = SolverCellState::Safe;
                    cellsNeedingTreatment.insert({nx, ny});
                }
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

    // Neighbor cells count
    qint8 safes = 0, mines = 0, unknown = 0;
    
    // Count neighboring cells by their solver state
    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            if (dx == 0 && dy == 0) continue;  // Skip the center cell
            uchar nx = x + dx;
            uchar ny = y + dy;
            if (nx < cols && ny < rows)
            {
                switch (solverBoard[nx][ny])
                {
                    case SolverCellState::Safe:
                        safes++;
                        break;
                    case SolverCellState::Mine:
                        mines++;
                        break;
                    case SolverCellState::Unknown:
                        unknown++;
                        break;
                }
            }
        }
    }

    // Get the expected number of mines for this cell
    const qint8 expectedMines = static_cast<qint8>(board[x][y].value);

    // If we've found all the mines, mark remaining unknowns as safe
    if (mines == expectedMines)
    {
        for (int dx = -1; dx <= 1; ++dx)
        {
            for (int dy = -1; dy <= 1; ++dy)
            {
                if (dx == 0 && dy == 0) continue;  // Skip the center cell
                uchar nx = x + dx;
                uchar ny = y + dy;
                if (nx < cols && ny < rows && solverBoard[nx][ny] == SolverCellState::Unknown)
                {
                    // If unable to clean as zero, this cell was not zero, so mark it as safe and add to treatment set
                    if (!cleanZero(nx, ny))
                    {
                        // Mark as safe if not zero and add to treatment set
                        solverBoard[nx][ny] = SolverCellState::Safe;
                        cellsNeedingTreatment.insert({nx, ny});
                    }
                }
            }
        }
        
        cellsNeedingTreatment.remove({x, y});
        return true;
    }

    // If the number of unknown neighbors equals remaining mines needed, mark them all as mines
    if ((mines + unknown) == expectedMines)
    {
        for (int dx = -1; dx <= 1; ++dx)
        {
            for (int dy = -1; dy <= 1; ++dy)
            {
                if (dx == 0 && dy == 0) continue;  // Skip the center cell
                uchar nx = x + dx;
                uchar ny = y + dy;
                if (nx < cols && ny < rows && solverBoard[nx][ny] == SolverCellState::Unknown)
                {
                    cellsNeedingTreatment.remove({nx, ny});
                    solverBoard[nx][ny] = SolverCellState::Mine;
                }
            }
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
        solverBoard[x - 1][y] == SolverCellState::Safe &&
        solverBoard[x][y - 1] == SolverCellState::Safe &&
        countRemainingMines(x - 1, y) == 1 &&
        countRemainingMines(x, y - 1) == 1 &&
        apply131CornerPatternLogic(x, y, x - 1, y, x, y - 1))
    {
        return true;
    }

    if (x < cols - 1 && y >= 1 &&
        solverBoard[x + 1][y] == SolverCellState::Safe &&
        solverBoard[x][y - 1] == SolverCellState::Safe &&
        countRemainingMines(x + 1, y) == 1 &&
        countRemainingMines(x, y - 1) == 1 &&
        apply131CornerPatternLogic(x, y, x + 1, y, x, y - 1))
    {
        return true;
    }

    if (x >= 1 && y < rows - 1 &&
        solverBoard[x - 1][y] == SolverCellState::Safe &&
        solverBoard[x][y + 1] == SolverCellState::Safe &&
        countRemainingMines(x - 1, y) == 1 &&
        countRemainingMines(x, y + 1) == 1 &&
        apply131CornerPatternLogic(x, y, x - 1, y, x, y + 1))
    {
        return true;
    }

    if (x < cols - 1 && y < rows - 1 &&
        solverBoard[x + 1][y] == SolverCellState::Safe &&
        solverBoard[x][y + 1] == SolverCellState::Safe &&
        countRemainingMines(x + 1, y) == 1 &&
        countRemainingMines(x, y + 1) == 1 &&
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
        solverBoard[x - 1][y] == SolverCellState::Safe &&
        solverBoard[x][y - 1] == SolverCellState::Safe &&
        countRemainingMines(x - 1, y) == 2 &&
        countRemainingMines(x, y - 1) == 2 &&
        apply222CornerPatternLogic(x, y, x - 1, y, x, y - 1))
    {
        return true;
    }

    if (x < cols - 1 && y >= 1 &&
        solverBoard[x + 1][y] == SolverCellState::Safe &&
        solverBoard[x][y - 1] == SolverCellState::Safe &&
        countRemainingMines(x + 1, y) == 2 &&
        countRemainingMines(x, y - 1) == 2 &&
        apply222CornerPatternLogic(x, y, x + 1, y, x, y - 1))
    {
        return true;
    }

    if (x >= 1 && y < rows - 1 &&
        solverBoard[x - 1][y] == SolverCellState::Safe &&
        solverBoard[x][y + 1] == SolverCellState::Safe &&
        countRemainingMines(x - 1, y) == 2 &&
        countRemainingMines(x, y + 1) == 2 &&
        apply222CornerPatternLogic(x, y, x - 1, y, x, y + 1))
    {
        return true;
    }

    if (x < cols - 1 && y < rows - 1 &&
        solverBoard[x + 1][y] == SolverCellState::Safe &&
        solverBoard[x][y + 1] == SolverCellState::Safe &&
        countRemainingMines(x + 1, y) == 2 &&
        countRemainingMines(x, y + 1) == 2 &&
        apply222CornerPatternLogic(x, y, x + 1, y, x, y + 1))
    {
        return true;
    }

    return false;
}

bool LibreMinesSolver::detectMNXPattern(uchar x, uchar y, int adjacentMines, int remainingMines)
{
    const int currentCellRemaining = countRemainingMines(x, y);
    
    // Current cell must have exactly `remainingMines` remaining unsolved neighbors
    if (currentCellRemaining != remainingMines)
    {
        return false;
    }

    // Look for M-N-X pattern in horizontal direction (left)
    if (x >= 1)
    {
        const int leftCellRemaining = countRemainingMines(x-1, y);
        if (leftCellRemaining == adjacentMines)
        {
            // Found M-N-X pattern horizontally (left direction)
            if (applyMNXPatternLogic(x, y, x-1, y, adjacentMines, remainingMines))
            {
                return true;
            }
        }
    }
    
    // Look for X-N-1 pattern in horizontal direction (right)
    if (x < cols - 1)
    {
        const int rightCellRemaining = countRemainingMines(x+1, y);
        if (rightCellRemaining == adjacentMines)
        {
            // Found X-N-M pattern horizontally (right direction)
            if (applyMNXPatternLogic(x, y, x+1, y, adjacentMines, remainingMines))
            {
                return true;
            }
        }
    }
    
    // Look for M-N-X pattern in vertical direction (top)
    if (y >= 1)
    {
        const int topCellRemaining = countRemainingMines(x, y-1);
        if (topCellRemaining == adjacentMines)
        {
            // Found M-N-X pattern vertically (top direction)
            if (applyMNXPatternLogic(x, y, x, y-1, adjacentMines, remainingMines))
            {
                return true;
            }
        }
    }
    
    // Look for X-N-M pattern in vertical direction (bottom)
    if (y < rows - 1)
    {
        const int bottomCellRemaining = countRemainingMines(x, y+1);
        if (bottomCellRemaining == adjacentMines)
        {
            // Found X-N-M pattern vertically (bottom direction)
            if (applyMNXPatternLogic(x, y, x, y+1, adjacentMines, remainingMines))
            {
                return true;
            }
        }
    }
    
    return false;
}

bool LibreMinesSolver::detectNNXPattern(uchar x, uchar y, int remainingMines)
{
    // Check if current cell has exactly `remainingMines` remaining unsolved neighbor
    const int currentCellRemaining = countRemainingMines(x, y);
    
    // Current cell must have exactly `remainingMines` remaining unsolved neighbor
    if (currentCellRemaining != remainingMines)
    {
        return false;
    }
    
    // Look for N-N-X pattern in horizontal direction (left)
    if (x >= 1)
    {
        const int leftCellRemaining = countRemainingMines(x-1, y);
        if (leftCellRemaining == remainingMines)
        {
            // Found N-N-X pattern horizontally (left direction)
            if (applyNNXPatternLogic(x, y, x-1, y, remainingMines))
            {
                return true;
            }
        }
    }
    
    // Look for N-N-X pattern in horizontal direction (right)
    if (x < cols - 1)
    {
        const int rightCellRemaining = countRemainingMines(x+1, y);
        if (rightCellRemaining == remainingMines)
        {
            // Found N-N-X pattern horizontally (right direction)
            if (applyNNXPatternLogic(x, y, x+1, y, remainingMines))
            {
                return true;
            }
        }
    }
    
    // Look for N-N-X pattern in vertical direction (top)
    if (y >= 1)
    {
        const int topCellRemaining = countRemainingMines(x, y-1);
        if (topCellRemaining == remainingMines)
        {
            // Found N-N-X pattern vertically (top direction)
            if (applyNNXPatternLogic(x, y, x, y-1, remainingMines))
            {
                return true;
            }
        }
    }
    
    // Look for N-N-X pattern in vertical direction (bottom)
    if (y < rows - 1)
    {
        const int bottomCellRemaining = countRemainingMines(x, y+1);
        if (bottomCellRemaining == remainingMines)
        {
            // Found N-N-X pattern vertically (bottom direction)
            if (applyNNXPatternLogic(x, y, x, y+1, remainingMines))
            {
                return true;
            }
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
    if (x >= 2 && countRemainingMines(x - 2, y) == remainingMines)
    {
        if (applyNNXPatternLogic(x, y, x - 2, y, remainingMines))
        {
            return true;
        }
    }

    // Look for N-X-N pattern in horizontal direction (right).
    if (x + 2 < cols && countRemainingMines(x + 2, y) == remainingMines)
    {
        if (applyNNXPatternLogic(x, y, x + 2, y, remainingMines))
        {
            return true;
        }
    }

    // Look for N-X-N pattern in vertical direction (top).
    if (y >= 2 && countRemainingMines(x, y - 2) == remainingMines)
    {
        if (applyNNXPatternLogic(x, y, x, y - 2, remainingMines))
        {
            return true;
        }
    }

    // Look for N-X-N pattern in vertical direction (bottom).
    if (y + 2 < rows && countRemainingMines(x, y + 2) == remainingMines)
    {
        if (applyNNXPatternLogic(x, y, x, y + 2, remainingMines))
        {
            return true;
        }
    }

    return false;
}

int LibreMinesSolver::countRemainingMines(uchar x, uchar y) const
{
    const int cellValue = static_cast<int>(board[x][y].value);
    int currentMines = 0;
    
    // Count current mines around the cell
    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            if (dx == 0 && dy == 0) continue;
            uchar nx = x + dx;
            uchar ny = y + dy;
            if (nx < cols && ny < rows && solverBoard[nx][ny] == SolverCellState::Mine)
            {
                currentMines++;
            }
        }
    }
    
    return cellValue - currentMines;
}

bool LibreMinesSolver::applyMNXPatternLogic(uchar middleX, uchar middleY, uchar adjacentX, uchar adjacentY, int adjacentMines, int remainingMines)
{
    // Find neighbors of middle and middle cells
    QSet<QPair<uchar, uchar>> middleNeighbors;
    QSet<QPair<uchar, uchar>> adjacentNeighbors;

    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            if (dx == 0 && dy == 0) continue;

            // middle cell neighbors
            uchar nx = middleX + dx;
            uchar ny = middleY + dy;
            if (nx < cols && ny < rows && solverBoard[nx][ny] == SolverCellState::Unknown)
            {
                middleNeighbors.insert({nx, ny});
            }

            // adjacent cell neighbors
            nx = adjacentX + dx;
            ny = adjacentY + dy;
            if (nx < cols && ny < rows && solverBoard[nx][ny] == SolverCellState::Unknown)
            {
                adjacentNeighbors.insert({nx, ny});
            }

        }
    }
    
    // Find shared and unique neighbors
    QSet<QPair<uchar, uchar>> sharedNeighbors = middleNeighbors & adjacentNeighbors;
    QSet<QPair<uchar, uchar>> middleUniqueNeighbors = middleNeighbors - sharedNeighbors;
    
    // Apply M-N-X pattern logic:
    // If the middle cell has N remaining mines and exactly N-M unique unknown
    // neighbors when compared to the adjacent M-cell, those unique neighbors are
    // forced mines for the pattern to hold.
    if (remainingMines > adjacentMines && middleUniqueNeighbors.size() == remainingMines - adjacentMines)
    {
        for (const QPair<uchar, uchar>& uniqueCell : middleUniqueNeighbors)
        {
            solverBoard[uniqueCell.first][uniqueCell.second] = SolverCellState::Mine;
            cellsNeedingTreatment.remove(uniqueCell);
        }
        return true;
    }
    
    return false;
}

bool LibreMinesSolver::applyNNXPatternLogic(uchar middleX, uchar middleY, uchar adjX, uchar adjY, int remainingMines)
{
    // Find neighbors of first and second cell
    QSet<QPair<uchar, uchar>> adjNeighbors;
    QSet<QPair<uchar, uchar>> midNeighbors;
    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            if (dx == 0 && dy == 0) continue;
            uchar nx = middleX + dx;
            uchar ny = middleY + dy;
            if (nx < cols && ny < rows && solverBoard[nx][ny] == SolverCellState::Unknown)
            {
                midNeighbors.insert({nx, ny});
            }

            // adjacent cell neighbors
            nx = adjX + dx;
            ny = adjY + dy;
            if (nx < cols && ny < rows && solverBoard[nx][ny] == SolverCellState::Unknown)
            {
                adjNeighbors.insert({nx, ny});
            }

        }
    }
    
    // Find shared and unique neighbors
    QSet<QPair<uchar, uchar>> sharedNeighbors = midNeighbors & adjNeighbors;
    QSet<QPair<uchar, uchar>> midUniqueNeighbors = midNeighbors - sharedNeighbors;
    
    // Apply N-N-X pattern logic:
    // If sharedNeighbors == adjNeighbors -> All midUniqueNeighbors are safe
    if (sharedNeighbors == adjNeighbors && !midUniqueNeighbors.isEmpty())
    {
        for (const QPair<uchar, uchar>& cell : midUniqueNeighbors)
        {
            // If unable to clean as zero, this cell was not zero, so mark it as safe and add to treatment set
            if (!cleanZero(cell.first, cell.second))
            {
                // Mark as safe if not zero and add to treatment set
                solverBoard[cell.first][cell.second] = SolverCellState::Safe;
                cellsNeedingTreatment.insert(cell);
            }
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

    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            if (dx == 0 && dy == 0)
            {
                continue;
            }

            uchar nx = middleX + dx;
            uchar ny = middleY + dy;
            if (nx < cols && ny < rows && solverBoard[nx][ny] == SolverCellState::Unknown)
            {
                middleNeighbors.insert({nx, ny});
            }

            nx = firstX + dx;
            ny = firstY + dy;
            if (nx < cols && ny < rows && solverBoard[nx][ny] == SolverCellState::Unknown)
            {
                firstNeighbors.insert({nx, ny});
            }

            nx = secondX + dx;
            ny = secondY + dy;
            if (nx < cols && ny < rows && solverBoard[nx][ny] == SolverCellState::Unknown)
            {
                secondNeighbors.insert({nx, ny});
            }
        }
    }

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
    solverBoard[cell.first][cell.second] = SolverCellState::Mine;
    cellsNeedingTreatment.remove(cell);

    const QSet<QPair<uchar, uchar>> firstUniqueNeighbors = firstNeighbors - middleNeighbors;
    const QSet<QPair<uchar, uchar>> secondUniqueNeighbors = secondNeighbors - middleNeighbors;

    // Mark the unique neighbors of the first and second cells as safe
    for (const QPair<uchar, uchar>& uniqueCell : firstUniqueNeighbors)
    {
        // If unable to clean as zero, this cell was not zero, so mark it as safe and add to treatment set
        if (!cleanZero(uniqueCell.first, uniqueCell.second))
        {
            // Mark as safe if not zero and add to treatment set
            solverBoard[uniqueCell.first][uniqueCell.second] = SolverCellState::Safe;
            cellsNeedingTreatment.insert(uniqueCell);
        }
    }

    // Mark the unique neighbors of the second cell as safe
    for (const QPair<uchar, uchar>& uniqueCell : secondUniqueNeighbors)
    {
        // If unable to clean as zero, this cell was not zero, so mark it as safe and add to treatment set
        if (!cleanZero(uniqueCell.first, uniqueCell.second))
        {
            // Mark as safe if not zero and add to treatment set
            solverBoard[uniqueCell.first][uniqueCell.second] = SolverCellState::Safe;
            cellsNeedingTreatment.insert(uniqueCell);
        }
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

    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            if (dx == 0 && dy == 0)
            {
                continue;
            }

            uchar nx = middleX + dx;
            uchar ny = middleY + dy;
            if (nx < cols && ny < rows && solverBoard[nx][ny] == SolverCellState::Unknown)
            {
                middleNeighbors.insert({nx, ny});
            }

            nx = firstX + dx;
            ny = firstY + dy;
            if (nx < cols && ny < rows && solverBoard[nx][ny] == SolverCellState::Unknown)
            {
                firstNeighbors.insert({nx, ny});
            }

            nx = secondX + dx;
            ny = secondY + dy;
            if (nx < cols && ny < rows && solverBoard[nx][ny] == SolverCellState::Unknown)
            {
                secondNeighbors.insert({nx, ny});
            }
        }
    }

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

    // Mark the unique middle neighbor as a safe
    if (!cleanZero(middleCell.first, middleCell.second))
    {
        // Mark as safe if not zero and add to treatment set
        solverBoard[middleCell.first][middleCell.second] = SolverCellState::Safe;
        cellsNeedingTreatment.insert(middleCell);
    }

    const QSet<QPair<uchar, uchar>> firstUniqueNeighbors = firstNeighbors - middleNeighbors;
    const QSet<QPair<uchar, uchar>> secondUniqueNeighbors = secondNeighbors - middleNeighbors;

    // If there is only one unique neighbor for the first or second cell, mark it as a mine
    if (firstUniqueNeighbors.size() == 1)
    {
        const QPair<uchar, uchar>& firstCell = *firstUniqueNeighbors.cbegin();

        // Mark the unique neighbor of the first cell as a mine
        solverBoard[firstCell.first][firstCell.second] = SolverCellState::Mine;
        cellsNeedingTreatment.remove(firstCell);
    }

    // If there is only one unique neighbor for the second cell, mark it as a mine
    if (secondUniqueNeighbors.size() == 1)
    {
        const QPair<uchar, uchar>& secondCell = *secondUniqueNeighbors.cbegin();

        // Mark the unique neighbor of the second cell as a mine
        solverBoard[secondCell.first][secondCell.second] = SolverCellState::Mine;
        cellsNeedingTreatment.remove(secondCell);
    }

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
