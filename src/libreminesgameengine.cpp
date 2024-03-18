/*****************************************************************************
 * LibreMines                                                                *
 * Copyright (C) 2020-2024  Bruno Bollos Correa                              *
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

    iHiddenCells = 0;

    // Check if we are just remaking the game with one cell clean
    const bool bRemakingGame = (i_X_Clean != 255 && i_Y_Clean != 255);

    if(!bRemakingGame)
    {
        bGameActive = false;
        bFirst = true;
        principalMatrix = std::vector<std::vector<CellGameEngine>> (iX, std::vector<CellGameEngine>(iY));
    }


    if(bRemakingGame)
    {
        for(std::vector<CellGameEngine>& i: principalMatrix)
        {
            for(CellGameEngine& j: i)
            {
                j.value = CellValue::ZERO;
            }
        }
    }
    else
    {
        for(uchar j=0; j<iY; j++)
        {
            for (uchar i=0; i<iX; i++)
            {
                CellGameEngine& cell = principalMatrix[i][j];

                cell.value = CellValue::ZERO;
                cell.isHidden = true;
                cell.flagState = FlagState::NoFlag;

//                qApp->processEvents();
            }
        }
    }

    // Add mines on random places until the number of mines is correct
    while(i_nMines_ > 0)
    {
        uchar i = QRandomGenerator::global()->bounded(0, iX);
        uchar j = QRandomGenerator::global()->bounded(0, iY);

        // Avoid cells neighbor of clean cell when remaking the game
        if(bRemakingGame && i <= i_X_Clean+1 && i >= i_X_Clean-1 && j <= i_Y_Clean+1 && j >= i_Y_Clean-1)
        {
            continue;
        }

        CellGameEngine& cell = principalMatrix[i][j];

        if(cell.value == CellValue::ZERO)
        {
            i_nMines_--;
            cell.value = CellValue::MINE;
        }
    }

    // Update the state of all cells
    for(uchar j=0; j<iY; j++)
    {
        for (uchar i=0; i<iX; i++)
        {
            CellGameEngine& cell = principalMatrix[i][j];

            if(cell.value == CellValue::ZERO)
            {
                iHiddenCells++;

                uchar minesNeighbors = 0;

                if(i == 0 &&
                   j == 0)
                {
                    if(principalMatrix[i+1][j].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j+1].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j+1].value == CellValue::MINE)
                        minesNeighbors++;
                }
                else if(i == 0 &&
                        j == iY-1)
                {
                    if(principalMatrix[i+1][j].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j-1].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j-1].value == CellValue::MINE)
                        minesNeighbors++;
                }
                else if(i == iX-1 &&
                        j==0)
                {
                    if(principalMatrix[i-1][j].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j+1].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i-1][j+1].value == CellValue::MINE)
                        minesNeighbors++;
                }
                else if(i == iX-1 &&
                        j == iY-1)
                {
                    if(principalMatrix[i-1][j].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j-1].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i-1][j-1].value == CellValue::MINE)
                        minesNeighbors++;
                }
                else if(i == 0 &&
                        j > 0 &&
                        j < iY-1)
                {
                    if(principalMatrix[i+1][j].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j+1].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j+1].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j-1].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j-1].value == CellValue::MINE)
                        minesNeighbors++;
                }
                else if(i == iX-1 &&
                        j >0 &&
                        j < iY-1)
                {
                    if(principalMatrix[i-1][j].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j+1].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i-1][j+1].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j-1].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i-1][j-1].value == CellValue::MINE)
                        minesNeighbors++;
                }
                else if(i > 0 &&
                        i < iX-1 &&
                        j == 0){
                    if(principalMatrix[i-1][j].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i-1][j+1].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j+1].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j+1].value == CellValue::MINE)
                        minesNeighbors++;
                }
                else if(i > 0 &&
                        i < iX-1 &&
                        j == iY-1)
                {
                    if(principalMatrix[i+1][j].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i-1][j].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i-1][j-1].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j-1].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j-1].value == CellValue::MINE)
                        minesNeighbors++;
                }
                else
                {
                    if(principalMatrix[i-1][j-1].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i-1][j].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i-1][j+1].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j-1].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j+1].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j-1].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j].value == CellValue::MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j+1].value == CellValue::MINE)
                        minesNeighbors++;
                }

                cell.value = (CellValue)minesNeighbors;
            }
        }
    }
    iCellsToUnlock = iHiddenCells;
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
            if(_X == 0 &&
               _Y == 0)
            {
                if(principalMatrix[_X+1][_Y].isHidden)
                    bCleanCell(_X+1, _Y);
                if(principalMatrix[_X][_Y+1].isHidden)
                    bCleanCell(_X, _Y+1);
                if(principalMatrix[_X+1][_Y+1].isHidden)
                    bCleanCell(_X+1, _Y+1);
            }
            else if(_X == 0 &&
                    _Y == iY-1)
            {
                if(principalMatrix[_X+1][_Y].isHidden)
                    bCleanCell(_X+1, _Y);
                if(principalMatrix[_X][_Y-1].isHidden)
                    bCleanCell(_X, _Y-1);
                if(principalMatrix[_X+1][_Y-1].isHidden)
                    bCleanCell(_X+1, _Y-1);
            }
            else if(_X == iX-1 &&
                    _Y == 0)
            {
                if(principalMatrix[_X-1][_Y].isHidden)
                    bCleanCell(_X-1, _Y);
                if(principalMatrix[_X][_Y+1].isHidden)
                    bCleanCell(_X, _Y+1);
                if(principalMatrix[_X-1][_Y+1].isHidden)
                    bCleanCell(_X-1, _Y+1);
            }
            else if(_X == iX-1 &&
                    _Y == iY-1)
            {
                if(principalMatrix[_X-1][_Y].isHidden)
                    bCleanCell(_X-1, _Y);
                if(principalMatrix[_X][_Y-1].isHidden)
                    bCleanCell(_X, _Y-1);
                if(principalMatrix[_X-1][_Y-1].isHidden)
                    bCleanCell(_X-1, _Y-1);
            }
            else if(_X == iX-1 &&
                    _Y == 0)
            {
                if(principalMatrix[_X-1][_Y].isHidden)
                    bCleanCell(_X-1, _Y);
                if(principalMatrix[_X][_Y+1].isHidden)
                    bCleanCell(_X, _Y+1);
                if(principalMatrix[_X-1][_Y+1].isHidden)
                    bCleanCell(_X-1, _Y+1);
            }
            else if(_X == iX-1 &&
                    _Y == iY-1)
            {
                if(principalMatrix[_X-1][_Y].isHidden)
                    bCleanCell(_X-1, _Y);
                if(principalMatrix[_X][_Y-1].isHidden)
                    bCleanCell(_X, _Y-1);
                if(principalMatrix[_X-1][_Y-1].isHidden)
                    bCleanCell(_X-1, _Y-1);
            }
            else if(_X == 0 &&
                    _Y > 0 &&
                    _Y < iY-1)
            {
                if(principalMatrix[_X+1][_Y].isHidden)
                    bCleanCell(_X+1, _Y);
                if(principalMatrix[_X][_Y+1].isHidden)
                    bCleanCell(_X, _Y+1);
                if(principalMatrix[_X+1][_Y+1].isHidden)
                    bCleanCell(_X+1, _Y+1);
                if(principalMatrix[_X][_Y-1].isHidden)
                    bCleanCell(_X, _Y-1);
                if(principalMatrix[_X+1][_Y-1].isHidden)
                    bCleanCell(_X+1, _Y-1);
            }
            else if(_X == iX-1 &&
                    _Y > 0 &&
                    _Y < iY-1)
            {
                if(principalMatrix[_X-1][_Y].isHidden)
                    bCleanCell(_X-1, _Y);
                if(principalMatrix[_X][_Y+1].isHidden)
                    bCleanCell(_X, _Y+1);
                if(principalMatrix[_X-1][_Y+1].isHidden)
                    bCleanCell(_X-1, _Y+1);
                if(principalMatrix[_X][_Y-1].isHidden)
                    bCleanCell(_X, _Y-1);
                if(principalMatrix[_X-1][_Y-1].isHidden)
                    bCleanCell(_X-1, _Y-1);
            }
            else if(_X > 0 &&
                    _X < iX-1 &&
                    _Y == 0)
            {
                if(principalMatrix[_X-1][_Y].isHidden)
                    bCleanCell(_X-1, _Y);
                if(principalMatrix[_X+1][_Y].isHidden)
                    bCleanCell(_X+1, _Y);
                if(principalMatrix[_X-1][_Y+1].isHidden)
                    bCleanCell(_X-1, _Y+1);
                if(principalMatrix[_X][_Y+1].isHidden)
                    bCleanCell(_X, _Y+1);
                if(principalMatrix[_X+1][_Y+1].isHidden)
                    bCleanCell(_X+1, _Y+1);
            }
            else if(_X > 0 &&
                    _X < iX-1 &&
                    _Y == iY-1)
            {
                if(principalMatrix[_X+1][_Y].isHidden)
                    bCleanCell(_X+1, _Y);
                if(principalMatrix[_X-1][_Y].isHidden)
                    bCleanCell(_X-1, _Y);
                if(principalMatrix[_X-1][_Y-1].isHidden)
                    bCleanCell(_X-1, _Y-1);
                if(principalMatrix[_X][_Y-1].isHidden)
                    bCleanCell(_X, _Y-1);
                if(principalMatrix[_X+1][_Y-1].isHidden)
                    bCleanCell(_X+1, _Y-1);
            }
            else
            {
                if(principalMatrix[_X-1][_Y-1].isHidden)
                    bCleanCell(_X-1, _Y-1);
                if(principalMatrix[_X-1][_Y].isHidden)
                    bCleanCell(_X-1, _Y);
                if(principalMatrix[_X-1][_Y+1].isHidden)
                    bCleanCell(_X-1, _Y+1);
                if(principalMatrix[_X][_Y-1].isHidden)
                    bCleanCell(_X, _Y-1);
                if(principalMatrix[_X][_Y+1].isHidden)
                    bCleanCell(_X, _Y+1);
                if(principalMatrix[_X+1][_Y-1].isHidden)
                    bCleanCell(_X+1, _Y-1);
                if(principalMatrix[_X+1][_Y].isHidden)
                    bCleanCell(_X+1, _Y);
                if(principalMatrix[_X+1][_Y+1].isHidden)
                    bCleanCell(_X+1, _Y+1);
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
    score.heigth = iY;
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


void LibreMinesGameEngine::SLOT_cleanCell(const uchar _X, const uchar _Y)
{
    if(bFirst && bFirstCellClean && principalMatrix[_X][_Y].value != CellValue::ZERO)
    {
        vNewGame(iX, iY, nMines, _X, _Y);
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

    timerTimeInGame->start(1000);
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

