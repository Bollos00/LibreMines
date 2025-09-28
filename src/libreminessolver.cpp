#include "libreminessolver.h"

#include "gameboardrandomgenerator.h"
#include <QMutexLocker>

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
        // TODO: Check if the generated board is actually solvable and set solvable accordingly
        solvable = true;
    }
    
    // Set state to finished upon successful completion
    if (solvable)
    {
        QMutexLocker locker(&stateMutex);
        state = SolverState::Finished;
    }
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
