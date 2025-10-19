#include "libreminessolver.h"

#include "gameboardrandomgenerator.h"
#include <QMutexLocker>
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

        qDebug() << "Tried boards: " << ++triesCount;
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

    qDebug() << "Initial step:";
    printSolverBoard(solverBoard, board);

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

        qDebug() << "Applying basic cleaning:";
        
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
            qDebug() << "Applying pattern cleaning:";
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
                if (board[nx][ny].value == CellValue::ZERO && solverBoard[nx][ny] == SolverCellState::Unknown)
                {
                    cleanZero(nx, ny);  // Recursively clear if the neighbor is also zero
                }
                else if (solverBoard[nx][ny] == SolverCellState::Unknown)
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
                    if (board[nx][ny].value != CellValue::ZERO)
                    {
                        cellsNeedingTreatment.insert({nx, ny});
                        solverBoard[nx][ny] = SolverCellState::Safe;
                    }
                    else
                    {
                        cleanZero(nx, ny);  // Recursively clear zero cells
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
    return changes;
}

bool LibreMinesSolver::detect12XPattern(uchar x, uchar y)
{
    // Check if current cell has exactly 2 remaining unsolved neighbors
    const int currentCellRemaining = countRemainingMines(x, y);
    
    // Current cell must have exactly 2 remaining unsolved neighbors
    if (currentCellRemaining != 2)
    {
        return false;
    }
    
    // Look for 1-2-X pattern in horizontal direction (left)
    if (x >= 1)
    {
        const int leftCellRemaining = countRemainingMines(x-1, y);
        if (leftCellRemaining == 1)
        {
            // Found 1-2-X pattern horizontally (left direction)
            if (apply12XPatternLogic(x, y, x-1, y))
            {
                return true;
            }
        }
    }
    
    // Look for X-2-1 pattern in horizontal direction (right)
    if (x < cols - 1)
    {
        const int rightCellRemaining = countRemainingMines(x+1, y);
        if (rightCellRemaining == 1)
        {
            // Found X-2-1 pattern horizontally (right direction)
            if (apply12XPatternLogic(x, y, x+1, y))
            {
                return true;
            }
        }
    }
    
    // Look for 1-2-X pattern in vertical direction (top)
    if (y >= 1)
    {
        const int topCellRemaining = countRemainingMines(x, y-1);
        if (topCellRemaining == 1)
        {
            // Found 1-2-X pattern vertically (top direction)
            if (apply12XPatternLogic(x, y, x, y-1))
            {
                return true;
            }
        }
    }
    
    // Look for X-2-1 pattern in vertical direction (bottom)
    if (y < rows - 1)
    {
        const int bottomCellRemaining = countRemainingMines(x, y+1);
        if (bottomCellRemaining == 1)
        {
            // Found X-2-1 pattern vertically (bottom direction)
            if (apply12XPatternLogic(x, y, x, y+1))
            {
                return true;
            }
        }
    }
    
    return false;
}

bool LibreMinesSolver::detect11XPattern(uchar x, uchar y)
{
    // Check if current cell has exactly 1 remaining unsolved neighbor
    const int currentCellRemaining = countRemainingMines(x, y);
    
    // Current cell must have exactly 1 remaining unsolved neighbor
    if (currentCellRemaining != 1)
    {
        return false;
    }
    
    // Look for 1-1-X pattern in horizontal direction (left)
    if (x >= 1)
    {
        const int leftCellRemaining = countRemainingMines(x-1, y);
        if (leftCellRemaining == 1)
        {
            // Found 1-1-X pattern horizontally (left direction)
            if (apply11XPatternLogic(x, y, x-1, y))
            {
                return true;
            }
        }
    }
    
    // Look for 1-1-X pattern in horizontal direction (right)
    if (x < cols - 1)
    {
        const int rightCellRemaining = countRemainingMines(x+1, y);
        if (rightCellRemaining == 1)
        {
            // Found 1-1-X pattern horizontally (right direction)
            if (apply11XPatternLogic(x, y, x+1, y))
            {
                return true;
            }
        }
    }
    
    // Look for 1-1-X pattern in vertical direction (top)
    if (y >= 1)
    {
        const int topCellRemaining = countRemainingMines(x, y-1);
        if (topCellRemaining == 1)
        {
            // Found 1-1-X pattern vertically (top direction)
            if (apply11XPatternLogic(x, y, x, y-1))
            {
                return true;
            }
        }
    }
    
    // Look for 1-1-X pattern in vertical direction (bottom)
    if (y < rows - 1)
    {
        const int bottomCellRemaining = countRemainingMines(x, y+1);
        if (bottomCellRemaining == 1)
        {
            // Found 1-1-X pattern vertically (bottom direction)
            if (apply11XPatternLogic(x, y, x, y+1))
            {
                return true;
            }
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

bool LibreMinesSolver::apply12XPatternLogic(uchar middleX, uchar middleY, uchar adjacentX, uchar adjacentY)
{
    // Find neighbors of middle cell
    QSet<QPair<uchar, uchar>> middleNeighbors;
    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            if (dx == 0 && dy == 0) continue;
            uchar nx = middleX + dx;
            uchar ny = middleY + dy;
            if (nx < cols && ny < rows && solverBoard[nx][ny] == SolverCellState::Unknown)
            {
                middleNeighbors.insert({nx, ny});
            }
        }
    }
    
    // Find neighbors of adjacent cell
    QSet<QPair<uchar, uchar>> adjacentNeighbors;
    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            if (dx == 0 && dy == 0) continue;
            uchar nx = adjacentX + dx;
            uchar ny = adjacentY + dy;
            if (nx < cols && ny < rows && solverBoard[nx][ny] == SolverCellState::Unknown)
            {
                adjacentNeighbors.insert({nx, ny});
            }
        }
    }
    
    // Find shared and unique neighbors
    QSet<QPair<uchar, uchar>> sharedNeighbors = middleNeighbors & adjacentNeighbors;
    QSet<QPair<uchar, uchar>> middleUniqueNeighbors = middleNeighbors - sharedNeighbors;
    
    // Apply 1-2-X pattern logic:
    // If we have exactly one mine in middle unique neighbors, then it must be a mine
    if (sharedNeighbors.size() == 2 && middleUniqueNeighbors.size() == 1)
    {
        QPair<uchar, uchar> uniqueCell = *middleUniqueNeighbors.begin();
        solverBoard[uniqueCell.first][uniqueCell.second] = SolverCellState::Mine;
        cellsNeedingTreatment.remove(uniqueCell);
        return true;
    }
    
    return false;
}

bool LibreMinesSolver::apply11XPatternLogic(uchar middleX, uchar middleY, uchar adjX, uchar adjY)
{
    // Find neighbors of first cell
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
        }
    }
    
    // Find neighbors of second cell
    QSet<QPair<uchar, uchar>> adjNeighbors;
    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            if (dx == 0 && dy == 0) continue;
            uchar nx = adjX + dx;
            uchar ny = adjY + dy;
            if (nx < cols && ny < rows && solverBoard[nx][ny] == SolverCellState::Unknown)
            {
                adjNeighbors.insert({nx, ny});
            }
        }
    }
    
    // Find shared and unique neighbors
    QSet<QPair<uchar, uchar>> sharedNeighbors = midNeighbors & adjNeighbors;
    QSet<QPair<uchar, uchar>> midUniqueNeighbors = midNeighbors - sharedNeighbors;
    
    // Apply 1-1-X pattern logic:
    // If sharedNeighbors == adjNeighbors -> All midUniqueNeighbors are safe
    if (sharedNeighbors == adjNeighbors && !midUniqueNeighbors.isEmpty())
    {
        for (const auto& cell : midUniqueNeighbors)
        {
            solverBoard[cell.first][cell.second] = SolverCellState::Safe;
            cellsNeedingTreatment.insert(cell);
        }
        return true;
    }

    return false;
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
