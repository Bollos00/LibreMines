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


#include "libreminesgameengine.h"
#include "gameboardrandomgenerator.h"
#include "libreminessolver.h"

#include <QRandomGenerator>
#include <QTimer>

LibreMinesGameEngine::CellGameEngine::CellGameEngine():
    value(CellValue::ZERO),
    isHidden(true),
    flagState(FlagState::NoFlag)
{

}

LibreMinesGameEngine::LibreMinesGameEngine()
{

}

void LibreMinesGameEngine::vNewGame(const uchar _X,
                                    const uchar _Y,
                                    short i_nMines_,
                                    const uchar i_X_Clean,
                                    const uchar i_Y_Clean)
{
    // Update the instance parameters
    iX = _X;
    iY = _Y;
    nMines = i_nMines_;
    iMinesLeft = i_nMines_;

    // Check if we are just remaking the game with one cell clean
    const bool bRemakingGame = (i_X_Clean != 255 && i_Y_Clean != 255);

    if(!bRemakingGame)
    {
        bGameActive = false;
        bFirst = true;
    }

    principalMatrix.clear();
    
    if (bRemakingGame && bNoGuessMode)
    {
        QVector<QSharedPointer<LibreMinesSolver>> solvers(SOLVERS_COUNT);

        // Start the solvers
        for (QSharedPointer<LibreMinesSolver>& solver : solvers)
        {
            solver.reset(new LibreMinesSolver());
            solver->setRows(iY);
            solver->setCols(iX);
            solver->setNMines(i_nMines_);
            solver->setSafeX(i_X_Clean);
            solver->setSafeY(i_Y_Clean);
            solver->start();
        }

        // Keeps checking if any solver finished
        while (true)
        {
            for (const QSharedPointer<LibreMinesSolver>& solver : solvers)
            {
                // If any solver finished, get its matrix and stop checking
                if (solver->getSolverState() == LibreMinesSolver::SolverState::Finished)
                {
                    principalMatrix = solver->getBoard();
                    break;
                }
            }

            // If we got a matrix, interrupt all solvers and break the outer loop
            if (!principalMatrix.empty())
            {
                for (QSharedPointer<LibreMinesSolver>& solver : solvers)
                {
                    solver->interrupt();
                }

                for (QSharedPointer<LibreMinesSolver>& solver : solvers)
                {
                    solver->wait();
                }
                break;
            }
            else
            {
                QThread::msleep(100);
            }
        }
    }
    else
    {
        principalMatrix = GameBoardRandomGenerator::vGenerate(iY, iX, i_nMines_, i_X_Clean, i_Y_Clean);
    }
    
    iHiddenCells = iCellsToUnlock = iY*iX - nMines;
    bGameActive = true;

    if(bRemakingGame)
    {
        Q_EMIT SIGNAL_remakeGame();
    }
}

void LibreMinesGameEngine::vResetPrincipalMatrix()
{
    principalMatrix.clear();
}

bool LibreMinesGameEngine::bCleanCell(const uchar _X, const uchar _Y, const bool recursive)
{
    if(principalMatrix[_X][_Y].flagState == FlagState::NoFlag &&
       principalMatrix[_X][_Y].value == CellValue::MINE)
    {
        // If the user tried to unlock an unflaged mined cell, (s)he lose
        vGameLost(_X, _Y);
        return false;
    }

    if(principalMatrix[_X][_Y].isHidden &&
       principalMatrix[_X][_Y].flagState == FlagState::NoFlag)
    {
        // Unlock the cell
        principalMatrix[_X][_Y].isHidden = false;
        Q_EMIT SIGNAL_showCell(_X, _Y, recursive);

        // If the state of the cell is CellValue::ZERO, unlock all neighbor cells
        if(principalMatrix[_X][_Y].value == CellValue::ZERO)
        {
            for(short i = _X - 1; i <= _X + 1; i++)
            {
                if(i < 0 || i >= iX)
                    continue;
                    
                for(short j = _Y - 1; j <= _Y + 1; j++)
                {
                    if(j < 0 || j >= iY)
                        continue;
                        
                    // Skip the center cell
                    if(i == _X && j == _Y)
                        continue;
                        
                    if(principalMatrix[i][j].isHidden)
                        bCleanCell(i, j);
                }
            }
        }

        iHiddenCells--;
        if(iHiddenCells == 0)
        {
            // If there is none hidden cells left, the user wins
            vGameWon();
        }
    }

    return true;
}

void LibreMinesGameEngine::vGameLost(const uchar _X, const uchar _Y)
{
    qint64 timeInNs = elapsedPreciseTimeInGame.nsecsElapsed();
    bGameActive = false;

    timerTimeInGame.reset();
    Q_EMIT SIGNAL_gameLost(_X, _Y);
    vGenerateEndGameScore(timeInNs);
}

void LibreMinesGameEngine::vGameWon()
{
    qint64 timeInNs = elapsedPreciseTimeInGame.nsecsElapsed();
    bGameActive = false;

    timerTimeInGame.reset();
    Q_EMIT SIGNAL_gameWon();
    vGenerateEndGameScore(timeInNs);
}


void LibreMinesGameEngine::vGenerateEndGameScore(qint64 iTimeInNs, bool ignorePreferences)
{
    static qint64 timeLastGame = -1;

    if(iTimeInNs != -1)
        timeLastGame = iTimeInNs;

    int iCorrectFlags = 0,
            iWrongFlags = 0,
            iUnlockedCells = iCellsToUnlock - iHiddenCells;

    for (int i=0; i<iX; i++)
    {
        for (int j=0; j<iY; j++)
        {
            if(principalMatrix[i][j].flagState == FlagState::HasFlag)
            {
                if (principalMatrix[i][j].value == CellValue::MINE)
                    iCorrectFlags++;
                else
                    iWrongFlags++;
            }
        }
    }

    double timeInSecs = (double)timeLastGame*1e-9;

    double dFlagsPerSecond = (double)iCorrectFlags/timeInSecs,
            dCellsPerSecond = (double)iUnlockedCells/timeInSecs,
            dPercentageGameComplete = (double)100*iUnlockedCells/iCellsToUnlock;

    LibreMinesScore score;
    score.iTimeInNs = timeLastGame;
    score.gameDifficulty = GameDifficulty::NONE;
    score.width = iX;
    score.height = iY;
    score.mines = nMines;
    score.bGameCompleted = iUnlockedCells == iCellsToUnlock;
    score.dPercentageGameCompleted = dPercentageGameComplete;

    Q_EMIT SIGNAL_endGameScore(score, iCorrectFlags, iWrongFlags, iUnlockedCells,
                               dFlagsPerSecond, dCellsPerSecond, ignorePreferences);
}

const std::vector<std::vector<LibreMinesGameEngine::CellGameEngine> >& LibreMinesGameEngine::getPrincipalMatrix() const
{
    return principalMatrix;
}

uchar LibreMinesGameEngine::rows() const
{
    return iX;
}

uchar LibreMinesGameEngine::lines() const
{
    return iY;
}

short LibreMinesGameEngine::mines() const
{
    return nMines;
}

ushort LibreMinesGameEngine::cellsToUnlock() const
{
    return iCellsToUnlock;
}

ushort LibreMinesGameEngine::hiddenCells() const
{
    return iHiddenCells;
}

bool LibreMinesGameEngine::isGameActive() const
{
    return bGameActive;
}

void LibreMinesGameEngine::setFirstCellClean(const bool x)
{
    bFirstCellClean = x;
}

void LibreMinesGameEngine::setUseQuestionMark(const bool x)
{
    bUseQuestionMark = x;
}

void LibreMinesGameEngine::setNoGuessMode(const bool x)
{
    bNoGuessMode = x;
}


void LibreMinesGameEngine::SLOT_cleanCell(const uchar _X, const uchar _Y)
{
    // Remake the game if it's the first click and the user asked for a clean first cell
    // On No Guess mode, always remake the game on first click
    if(bFirst && bFirstCellClean && (principalMatrix[_X][_Y].value != CellValue::ZERO || bNoGuessMode))
    {
        // Check if it's impossible to place all mines while keeping the clicked cell and its neighbors clean
        // This prevents infinite loop when nMines > (total cells - 9)
        const int totalCells = iX * iY;
        const int minRequiredFreeCells = 9; // clicked cell + 8 neighbors
        
        // Cannot guarantee 9 free cells, so don't regenerate the game
        // Just start the timer and proceed with current mine layout
        if(nMines <= (totalCells - minRequiredFreeCells))
        {
            vNewGame(iX, iY, nMines, _X, _Y);
        }
        
        SLOT_startTimer();
        bFirst = false;
    }
    else if(bFirst)
    {
        SLOT_startTimer();
        bFirst = false;
    }
    bCleanCell(_X, _Y, false);
}

void LibreMinesGameEngine::SLOT_changeFlagState(const uchar _X, const uchar _Y)
{
    if(!principalMatrix[_X][_Y].isHidden)
        return;

    switch(principalMatrix[_X][_Y].flagState) {
        case FlagState::NoFlag:
        {
            principalMatrix[_X][_Y].flagState = FlagState::HasFlag;
            iMinesLeft--;
            Q_EMIT SIGNAL_flagCell(_X, _Y);
        }break;
        case FlagState::HasFlag:
        {
            iMinesLeft++;
            if(bUseQuestionMark)
            {
                principalMatrix[_X][_Y].flagState = FlagState::Question;
                Q_EMIT SIGNAL_questionCell(_X, _Y);
            }
            else
            {
                principalMatrix[_X][_Y].flagState = FlagState::NoFlag;
                Q_EMIT SIGNAL_unflagCell(_X, _Y);
            }
        }break;
        case FlagState::Question:
        {
            principalMatrix[_X][_Y].flagState = FlagState::NoFlag;
            Q_EMIT SIGNAL_unflagCell(_X, _Y);
        }break;
    }

    Q_EMIT SIGNAL_minesLeft(iMinesLeft);
}

void LibreMinesGameEngine::SLOT_stop()
{
    timerTimeInGame.reset();
    bGameActive = false;
    vResetPrincipalMatrix();
}

void LibreMinesGameEngine::SLOT_startTimer()
{
    timerTimeInGame.reset(new QTimer());
    QObject::connect(timerTimeInGame.get(), &QTimer::timeout,
                     this, &LibreMinesGameEngine::SLOT_UpdateTime);

    // reduce the timer interval to 100 ms.
    // This fixes issue #83 where the display timer was one second ahead of real time
    timerTimeInGame->start(100);
    elapsedPreciseTimeInGame.start();
}

void LibreMinesGameEngine::SLOT_cleanNeighborCells(const uchar _X, const uchar _Y)
{

    bool recursive = false;
    // Clean all hided and unflaged neighbor flags
    for(short i=_X-1; i<=_X+1; i++)
    {
        if(i<0 || i>=iX)
            continue;

        for(short j=_Y-1; j<=_Y+1; j++)
        {
            if(j<0 || j>=iY)
                continue;

            const CellGameEngine& cell = principalMatrix[i][j];

            if(cell.isHidden && cell.flagState == FlagState::NoFlag)
            {
                if(!bCleanCell(i, j, recursive))
                    return;

                recursive = true;
            }
        }
    }
}

void LibreMinesGameEngine::SLOT_generateEndGameScoreAgain()
{
    vGenerateEndGameScore(-1, true);
}

void LibreMinesGameEngine::SLOT_UpdateTime()
{
    Q_EMIT SIGNAL_currentTime(elapsedPreciseTimeInGame.elapsed()/1000);
}

