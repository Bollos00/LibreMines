/*****************************************************************************
 * LibreMines                                                                *
 * Copyright (C) 2020  Bruno Bollos Correa                                   *
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

LibreMinesGameEngine::Cell::Cell():
    state(ZERO),
    isHidden(true),
    hasFlag(false)
{

}

LibreMinesGameEngine::LibreMinesGameEngine()
{

}

void LibreMinesGameEngine::vNewGame(const uchar _X,
                                    const uchar _Y,
                                    ushort i_nMines_,
                                    const uchar i_X_Clean,
                                    const uchar i_Y_Clean)
{
    iX = _X;
    iY = _Y;
    nMines = i_nMines_;
    iMinesLeft = i_nMines_;
    iHiddenCells = 0;

    const bool bRemakingGame = (i_X_Clean != 255 && i_Y_Clean != 255);

    if(!bRemakingGame)
    {
        bGameActive = false;
        bFirst = true;
        principalMatrix = std::vector<std::vector<Cell>> (iX, std::vector<Cell>(iY));
    }


    if(bRemakingGame)
    {
        for(std::vector<Cell>& i: principalMatrix)
        {
            for(Cell& j: i)
            {
                j.state = ZERO;
            }
        }
    }
    else
    {
        for(uchar j=0; j<iY; j++)
        {
            for (uchar i=0; i<iX; i++)
            {
                Cell& cell = principalMatrix[i][j];

                cell.state = ZERO;
                cell.isHidden = true;
                cell.hasFlag = false;

//                qApp->processEvents();
            }
        }
    }


    QVector<Vector2Dshort> vt_vt2d_CleanPoints = QVector<Vector2Dshort>();

    if(bRemakingGame)
    {
        vt_vt2d_CleanPoints.reserve(9);

        for (short i=-1; i<=1; i++)
        {
            for (short j=-1; j<=1; j++)
            {
                vt_vt2d_CleanPoints.append(Vector2Dshort(i_X_Clean + i, i_Y_Clean + j));
            }
        }
    }

    while(i_nMines_ > 0)
    {
        uchar i = QRandomGenerator::global()->bounded(0, iX);
        uchar j = QRandomGenerator::global()->bounded(0, iY);

        bool bPointClean = false;

        if(bRemakingGame)
        {
            for (const Vector2Dshort& n: vt_vt2d_CleanPoints)
            {
                if(n.x == i &&
                   n.y == j)
                {
                    bPointClean = true;
                    break;
                }
            }
        }

        Cell& cell = principalMatrix[i][j];

        if(cell.state == ZERO &&
           !bPointClean)
        {
            i_nMines_--;
            cell.state = MINE;
            cell.label->setPixmap(QPM_Mine);
        }
    }

    for(uchar j=0; j<iY; j++)
    {
        for (uchar i=0; i<iX; i++)
        {
            Cell& cell = principalMatrix[i][j];

            cell.button->setEnabled(true);

            if(cell.state == ZERO)
            {
                iHiddenCells++;

                uchar minesNeighbors = 0;

                if(i == 0 &&
                   j == 0)
                {
                    if(principalMatrix[i+1][j].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j+1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j+1].state == MINE)
                        minesNeighbors++;
                }
                else if(i == 0 &&
                        j == iY-1)
                {
                    if(principalMatrix[i+1][j].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j-1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j-1].state == MINE)
                        minesNeighbors++;
                }
                else if(i == iX-1 &&
                        j==0)
                {
                    if(principalMatrix[i-1][j].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j+1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i-1][j+1].state == MINE)
                        minesNeighbors++;
                }
                else if(i == iX-1 &&
                        j == iY-1)
                {
                    if(principalMatrix[i-1][j].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j-1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i-1][j-1].state == MINE)
                        minesNeighbors++;
                }
                else if(i == 0 &&
                        j > 0 &&
                        j < iY-1)
                {
                    if(principalMatrix[i+1][j].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j+1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j+1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j-1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j-1].state == MINE)
                        minesNeighbors++;
                }
                else if(i == iX-1 &&
                        j >0 &&
                        j < iY-1)
                {
                    if(principalMatrix[i-1][j].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j+1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i-1][j+1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j-1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i-1][j-1].state == MINE)
                        minesNeighbors++;
                }
                else if(i > 0 &&
                        i < iX-1 &&
                        j == 0){
                    if(principalMatrix[i-1][j].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i-1][j+1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j+1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j+1].state == MINE)
                        minesNeighbors++;
                }
                else if(i > 0 &&
                        i < iX-1 &&
                        j == iY-1)
                {
                    if(principalMatrix[i+1][j].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i-1][j].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i-1][j-1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j-1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j-1].state == MINE)
                        minesNeighbors++;
                }
                else
                {
                    if(principalMatrix[i-1][j-1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i-1][j].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i-1][j+1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j-1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j+1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j-1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j+1].state == MINE)
                        minesNeighbors++;
                }

                switch (minesNeighbors)
                {
                    case 0:
                        cell.state = ZERO;
                        cell.label->setPixmap(QPM_Zero);
                        break;

                    case 1:
                        cell.state = ONE;
                        cell.label->setPixmap(QPM_One);
                        break;

                    case 2:
                        cell.state = TWO;
                        cell.label->setPixmap(QPM_Two);
                        break;

                    case 3:
                        cell.state = THREE;
                        cell.label->setPixmap(QPM_Three);
                        break;

                    case 4:
                        cell.state = FOUR;
                        cell.label->setPixmap(QPM_Four);
                        break;

                    case 5:
                        cell.state = FIVE;
                        cell.label->setPixmap(QPM_Five);
                        break;

                    case 6:
                        cell.state = SIX;
                        cell.label->setPixmap(QPM_Six);
                        break;

                    case 7:
                        cell.state = SEVEN;
                        cell.label->setPixmap(QPM_Seven);
                        break;

                    case 8:
                        cell.state = EIGHT;
                        cell.label->setPixmap(QPM_Eight);
                        break;
                }
            }
        }
    }

    iCellsToUnlock = iHiddenCells;

    vAjustInterfaceInGame();
    vShowInterfaceInGame();
    lcd_numberMinesLeft->display(iMinesLeft);
    labelTimerInGame->setNum(0);
    labelYouWonYouLost->setText(" ");

    buttonQuitInGame->setEnabled(true);
    buttonRestartInGame->setEnabled(true);

    bGameOn = true;

}

void LibreMinesGameEngine::vResetPrincipalMatrix()
{

}

void LibreMinesGameEngine::vCleanCell(const uchar _X, const uchar _Y)
{

}

void LibreMinesGameEngine::vAddOrRemoveFlag(const uchar _X, const uchar _Y)
{

}

void LibreMinesGameEngine::vGameLost(const uchar _X, const uchar _Y)
{

}

void LibreMinesGameEngine::vGameWon()
{

}

void LibreMinesGameEngine::vStartTimer()
{

}

void LibreMinesGameEngine::vGenerateEndGameStatics()
{

}

