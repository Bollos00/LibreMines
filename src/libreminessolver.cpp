#include "libreminessolver.h"

#include "gameboardrandomgenerator.h"
#include <QMutexLocker>
void printSolverBoard(const std::vector<std::vector<LibreMinesSolver::SolverCellState>>& solverBoard)
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
                case LibreMinesSolver::SolverCellState::Unknown: c = '?'; break;
                case LibreMinesSolver::SolverCellState::Safe:    c = 'S'; break;
                case LibreMinesSolver::SolverCellState::Mine:    c = 'M'; break;
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
    {
        QMutexLocker locker(&stateMutex);
        state = SolverState::Running;
    }
    
    bool solvable = false;

    while (!solvable)
    {
        if (cmd == SolverCommand::CmdInterrupt)
        {
            QMutexLocker locker(&stateMutex);
            state = SolverState::Interrupted;
            break;
        }

        board = GameBoardRandomGenerator::vGenerate(rows, cols, nMines, safeX, safeY);
        solverBoard.resize(rows, std::vector<SolverCellState>(cols, SolverCellState::Unknown));
        // TODO: Check if the generated board is actually solvable and set solvable accordingly
        solvable = solve();
    }
    
    // Set state to finished upon successful completion
    if (solvable)
    {
        QMutexLocker locker(&stateMutex);
        state = SolverState::Finished;
    }
}

bool LibreMinesSolver::solve()
{
    // Implement the solving logic here

    // First clear all neighbor cells of the safe cell
    cleanZero(safeX, safeY);

    qDebug() << "Initial step:";
    printSolverBoard(solverBoard);

    // Treatment loop: keep processing cells until no more changes can be made
    bool globalChanges = true;
    while (globalChanges && !cellsNeedingTreatment.isEmpty())
    {
        globalChanges = false;
        
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
                // Remove the successfully treated cell from the set
                cellsNeedingTreatment.remove(cellCoord);
            }
        }
        
        // Debug output for each iteration
        if (globalChanges)
        {
            qDebug() << "Treatment iteration completed with changes:";
            printSolverBoard(solverBoard);
        }
    }

    return true;
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
            if (nx < rows && ny < cols)
            {
                if (board[nx][ny].value == CellValue::ZERO && solverBoard[nx][ny] == SolverCellState::Unknown)
                {
                    cleanZero(nx, ny);  // Recursively clear if the neighbor is also zero
                }
                else
                {
                    // Mark as safe if not zero and add to treatment set
                    solverBoard[nx][ny] = SolverCellState::Safe;
                    cellsNeedingTreatment.insert({nx, ny});
                }
            }
        }
    }

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
            if (nx < rows && ny < cols)
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
                if (nx < rows && ny < cols && solverBoard[nx][ny] == SolverCellState::Unknown)
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
                if (nx < rows && ny < cols && solverBoard[nx][ny] == SolverCellState::Unknown)
                {
                    solverBoard[nx][ny] = SolverCellState::Mine;
                }
            }
        }
        return true;
    }

    return false;
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
