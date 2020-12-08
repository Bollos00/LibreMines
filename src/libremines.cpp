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

#include <QtCore/QRandomGenerator>
#include <QDebug>
#include <QFont>
#include <QMouseEvent>
#include <QStyleFactory>
#include <QApplication>
#include <QKeyEvent>

#include "libremines.h"

LibreMines::LibreMines(QWidget *parent) :
    QMainWindow(parent),
    principalMatrix( std::vector< std::vector<Cell> >(0) ),
    iLimitHeight( 0 ),
    iLimitWidth( 0 ),
    iX( 0 ),
    iY( 0 ),
    nMines( 0 ),
    iTimeInSeconds( 0 ),
    iMinesLeft( 0 ),
    iHiddenCells( 0 ),
    fm( 0 ),
    iCellsToUnlock( 0 ),
    bFirst( 0 ),
    bFirstCellClean( 0 ),
    difficult( NONE ),
    imgZero ( new QImage(":/Media_rsc/Media/Minesweeper_zero_dark.png") ),
    imgOne ( new QImage(":/Media_rsc/Media/Minesweeper_one_dark.png") ),
    imgTwo ( new QImage(":/Media_rsc/Media/Minesweeper_two_dark.png") ),
    imgThree ( new QImage(":/Media_rsc/Media/Minesweeper_three_dark.png") ),
    imgFour( new QImage(":/Media_rsc/Media/Minesweeper_four_dark.png") ),
    imgFive ( new QImage(":/Media_rsc/Media/Minesweeper_five_dark.png") ),
    imgSix( new QImage(":/Media_rsc/Media/Minesweeper_six_dark.png") ),
    imgSeven ( new QImage(":/Media_rsc/Media/Minesweeper_seven_dark.png") ),
    imgEight ( new QImage(":/Media_rsc/Media/Minesweeper_eight_dark.png") ),
    imgFlag ( new QImage(":/Media_rsc/Media/Minesweeper_flag_dark.png") ),
    imgNoFlag ( new QImage(":/Media_rsc/Media/Minesweeper_no_flag_dark.png") ),
    imgMine ( new QImage(":/Media_rsc/Media/Minesweeper_mine_dark.png") ),
    imgBoom ( new QImage(":/Media_rsc/Media/Minesweeper_boom_dark.png") ),
    imgWrongFlag ( new QImage(":/Media_rsc/Media/Minesweeper_wrong_flag_dark.png") )
{
    vConfigureInterface();

    timerTimeInGame = nullptr;
    qApp->installEventFilter(this);

    controller.ctrlPressed = false;
    controller.active = false;
    controller.currentX = 0;
    controller.currentY = 0;


    vConfigureDarkMode(true);
}

LibreMines::~LibreMines()
{
    vResetPrincipalMatrix();

    delete imgZero;
    delete imgOne;
    delete imgTwo;
    delete imgThree;
    delete imgFour;
    delete imgFive;
    delete imgSix;
    delete imgSeven;
    delete imgEight;
    delete imgFlag;
    delete imgNoFlag;
    delete imgMine;
    delete imgBoom;
    delete imgWrongFlag;
}

bool LibreMines::eventFilter(QObject* object, QEvent* event)
{
    if(difficult == NONE || !bGameOn)
        return false;

    switch(event->type())
    {
        case QEvent::KeyPress:
        {
            Qt::Key key = (Qt::Key)((QKeyEvent*)event)->key();

            switch(key)
            {
                case Qt::Key_Control:
                    controller.ctrlPressed = true;
                    return true;

                default:
                    break;
            }

        }break;

        case QEvent::KeyRelease:
        {
            Qt::Key key = (Qt::Key)((QKeyEvent*)event)->key();

            switch(key)
            {
                case Qt::Key_Control:
                    controller.ctrlPressed = false;
                    return true;

                case Qt::Key_A:
                case Qt::Key_Left:
                    if(!controller.active)
                    {
                        controller.active = true;
                        vKeyboardControllerSetCurrentCell(0, 0);
                    }
                    else
                    {
                        vKeyboardControllerMoveLeft();
                    }
                    return true;

                case Qt::Key_W:
                case Qt::Key_Up:
                    if(!controller.active)
                    {
                        controller.active = true;
                        vKeyboardControllerSetCurrentCell(0, 0);
                    }
                    else
                    {
                        vKeyboardControllerMoveUp();
                    }
                    return true;

                case Qt::Key_S:
                case Qt::Key_Down:
                    if(!controller.active)
                    {
                        controller.active = true;
                        vKeyboardControllerSetCurrentCell(0, 0);
                    }
                    else
                    {
                        vKeyboardControllerMoveDown();
                    }
                    return true;

                case Qt::Key_D:
                case Qt::Key_Right:
                    if(!controller.active)
                    {
                        controller.active = true;
                        vKeyboardControllerSetCurrentCell(0, 0);
                    }
                    else
                    {
                        vKeyboardControllerMoveRight();
                    }
                    return true;

                case Qt::Key_1:
                case Qt::Key_O:
                    if(controller.active)
                    {
                        if(bFirst)
                        {
                            if(bFirstCellClean && principalMatrix[controller.currentX][controller.currentY].state != ZERO)
                            {
                                vNewGame(iX, iY, nMines, controller.currentX, controller.currentY);
                            }
                            bFirst = false;
                            vStartTimer();
                        }

                        if(principalMatrix[controller.currentX][controller.currentY].state == MINE)
                        {
                            if(!principalMatrix[controller.currentX][controller.currentY].hasFlag)
                            {
                                vGameLost(controller.currentX, controller.currentY);
                            }
                        }
                        else
                        {
                            vCleanCell(controller.currentX, controller.currentY);
                            vKeyboardControllerUpdateCurrentCell();
                        }
                        return true;
                    }
                    else
                    {
                        return false;
                    }

                case Qt::Key_2:
                case Qt::Key_P:
                    if(controller.active)
                    {
                        vAddOrRemoveFlag(controller.currentX, controller.currentY);
                        vKeyboardControllerSetCurrentCell(controller.currentX, controller.currentY);
                        return true;
                    }
                    else
                    {
                        return false;
                    }

                case Qt::Key_Escape:
                    if(controller.active)
                    {
                        controller.active = false;
                        vKeyboardControllUnsetCurrentCell();
                        return true;
                    }
                    else
                    {
                        return false;
                    }

                default:
                    break;
            }

        }break;

        default:
            break;
    }

    return false;
}

void LibreMines::vNewGame(const uchar _X,
                          const uchar _Y,
                          ushort i_nMines_,
                          const uchar i_X_Clean,
                          const uchar i_Y_Clean)
{
    buttonQuitInGame->setEnabled(false);
    buttonRestartInGame->setEnabled(false);

    iX = _X;
    iY = _Y;
    nMines = i_nMines_;
    iMinesLeft = i_nMines_;
    iHiddenCells = 0;

    const bool bRemakingGame = (i_X_Clean != 255 && i_Y_Clean != 255);

    if(!bRemakingGame)
    {
        bGameOn = false;
        bFirst = true;
        controller.ctrlPressed = false;
        controller.active = false;
        controller.currentX = 0;
        controller.currentY = 0;
        principalMatrix = std::vector<std::vector<Cell>> (iX, std::vector<Cell>(iY));
    }

    if(iLimitWidth/iX < iLimitHeight/iY)
        fm = iLimitWidth/iX;
    else
        fm = iLimitHeight/iY;

    const QPixmap
            QPM_Zero = QPixmap::fromImage(*imgZero).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_One = QPixmap::fromImage(*imgOne).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_Two = QPixmap::fromImage(*imgTwo).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_Three = QPixmap::fromImage(*imgThree).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_Four = QPixmap::fromImage(*imgFour).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_Five = QPixmap::fromImage(*imgFive).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_Six = QPixmap::fromImage(*imgSix).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_Seven = QPixmap::fromImage(*imgSeven).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_Eight = QPixmap::fromImage(*imgEight).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_NoFlag = QPixmap::fromImage(*imgNoFlag).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_Mine = QPixmap::fromImage(*imgMine).scaled(fm, fm, Qt::KeepAspectRatio);

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

                cell.label = new QLabel(this);
                cell.button = new QPushButton_adapted(this);

                cell.label->setGeometry(i*fm,j*fm,fm,fm);
                cell.label->show();

                cell.button->setGeometry(i*fm,j*fm,fm,fm);
                cell.button->setIcon(QIcon(QPM_NoFlag));
                cell.button->setIconSize(QSize(fm, fm));
                cell.button->show();
                cell.button->setEnabled(false);

                cell.state = ZERO;
                cell.isHidden = true;
                cell.hasFlag = false;

                connect(cell.button, &QPushButton_adapted::SIGNAL_Clicked,
                        this, &LibreMines::SLOT_OnButtonClicked);

                qApp->processEvents();
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
        uchar i = QRandomGenerator::global()->bounded(0,iX);
        uchar j = QRandomGenerator::global()->bounded(0,iY);

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



void LibreMines::vGameLost(const uchar _X, const uchar _Y)
{
    bGameOn = false;
    qDebug()<<"You Lost";

    timerTimeInGame->stop();

    switch (difficult)
    {
        case NONE:
            break;
        case EASY:
            labelYouWonYouLost->setText("You Lost\nDifficulty: EASY");
            break;
        case MEDIUM:
            labelYouWonYouLost->setText("You Lost\nDifficulty: MEDIUM");
            break;
        case HARD:
            labelYouWonYouLost->setText("You Lost\nDifficulty: HARD");
            break;
        case CUSTOMIZED:
            labelYouWonYouLost->setText("You Lost\nDifficulty: CUSTOM\n" +
                                        QString::number(iX) +
                                        "x" +
                                        QString::number(iY) +
                                        " : " +
                                        QString::number(nMines) +
                                        " Mines");
    }
    principalMatrix[_X][_Y].label->setPixmap(QPixmap::fromImage(*imgBoom).scaled(fm, fm, Qt::KeepAspectRatio));

    vGenerateStatics();

    for(uchar j=0; j<iY; j++)
    {
        for (uchar i=0; i<iX; i++)
        {
            Cell& cell = principalMatrix[i][j];

            if(cell.isHidden)
            {
                if(cell.state == MINE &&
                   !cell.hasFlag)
                {
                    cell.button->hide();
                }
                else if (cell.state != MINE &&
                         cell.hasFlag)
                {
                    cell.button->hide();
                    cell.label->setPixmap(QPixmap::fromImage(*imgWrongFlag).scaled(fm, fm, Qt::KeepAspectRatio));
                }
                else
                {
                    cell.button->setEnabled(false);
                }
            }
        }
    }

    if(controller.active)
    {
        controller.active = false;
        vKeyboardControllUnsetCurrentCell();
    }
}

void LibreMines::vGameWon()
{
    bGameOn = false;
    qDebug()<<"You won: "<<iTimeInSeconds<<" Seconds";

    timerTimeInGame->stop();

    switch (difficult)
    {
        case NONE:
            break;
        case EASY:
            labelYouWonYouLost->setText("You Won\nDifficulty: EASY");
            break;
        case MEDIUM:
            labelYouWonYouLost->setText("You Won\nDifficulty: MEDIUM");
            break;
        case HARD:
            labelYouWonYouLost->setText("You Won\nDifficulty: HARD");
            break;
        case CUSTOMIZED:
            labelYouWonYouLost->setText("You Won\nDifficulty: CUSTOM\n" + QString::number(iX) + "x" + QString::number(iY) + " : " + QString::number(nMines) + " Mines");
    }

    vGenerateStatics();
    for(uchar j=0; j<iY; j++)
    {
        for (uchar i=0; i<iX; i++)
        {
            principalMatrix[i][j].button->setEnabled(!principalMatrix[i][j].isHidden);
        }
    }

    if(controller.active)
    {
        controller.active = false;
        vKeyboardControllUnsetCurrentCell();
    }

}

void LibreMines::vCleanCell(const uchar _X, const uchar _Y)
{
    if(principalMatrix[_X][_Y].isHidden &&
       !principalMatrix[_X][_Y].hasFlag)
    {
        principalMatrix[_X][_Y].isHidden = false;
        principalMatrix[_X][_Y].button->hide();

        if(principalMatrix[_X][_Y].state == ZERO)
        {
            if(_X == 0 &&
               _Y == 0)
            {
                if(principalMatrix[_X+1][_Y].isHidden)
                    vCleanCell(_X+1, _Y);
                if(principalMatrix[_X][_Y+1].isHidden)
                    vCleanCell(_X, _Y+1);
                if(principalMatrix[_X+1][_Y+1].isHidden)
                    vCleanCell(_X+1, _Y+1);
            }
            else if(_X == 0 &&
                    _Y == iY-1)
            {
                if(principalMatrix[_X+1][_Y].isHidden)
                    vCleanCell(_X+1, _Y);
                if(principalMatrix[_X][_Y-1].isHidden)
                    vCleanCell(_X, _Y-1);
                if(principalMatrix[_X+1][_Y-1].isHidden)
                    vCleanCell(_X+1, _Y-1);
            }
            else if(_X == iX-1 &&
                    _Y == 0)
            {
                if(principalMatrix[_X-1][_Y].isHidden)
                    vCleanCell(_X-1, _Y);
                if(principalMatrix[_X][_Y+1].isHidden)
                    vCleanCell(_X, _Y+1);
                if(principalMatrix[_X-1][_Y+1].isHidden)
                    vCleanCell(_X-1, _Y+1);
            }
            else if(_X == iX-1 &&
                    _Y == iY-1)
            {
                if(principalMatrix[_X-1][_Y].isHidden)
                    vCleanCell(_X-1, _Y);
                if(principalMatrix[_X][_Y-1].isHidden)
                    vCleanCell(_X, _Y-1);
                if(principalMatrix[_X-1][_Y-1].isHidden)
                    vCleanCell(_X-1, _Y-1);
            }
            else if(_X == iX-1 &&
                    _Y == 0)
            {
                if(principalMatrix[_X-1][_Y].isHidden)
                    vCleanCell(_X-1, _Y);
                if(principalMatrix[_X][_Y+1].isHidden)
                    vCleanCell(_X, _Y+1);
                if(principalMatrix[_X-1][_Y+1].isHidden)
                    vCleanCell(_X-1, _Y+1);
            }
            else if(_X == iX-1 &&
                    _Y == iY-1)
            {
                if(principalMatrix[_X-1][_Y].isHidden)
                    vCleanCell(_X-1, _Y);
                if(principalMatrix[_X][_Y-1].isHidden)
                    vCleanCell(_X, _Y-1);
                if(principalMatrix[_X-1][_Y-1].isHidden)
                    vCleanCell(_X-1, _Y-1);
            }
            else if(_X == 0 &&
                    _Y > 0 &&
                    _Y < iY-1)
            {
                if(principalMatrix[_X+1][_Y].isHidden)
                    vCleanCell(_X+1, _Y);
                if(principalMatrix[_X][_Y+1].isHidden)
                    vCleanCell(_X, _Y+1);
                if(principalMatrix[_X+1][_Y+1].isHidden)
                    vCleanCell(_X+1, _Y+1);
                if(principalMatrix[_X][_Y-1].isHidden)
                    vCleanCell(_X, _Y-1);
                if(principalMatrix[_X+1][_Y-1].isHidden)
                    vCleanCell(_X+1, _Y-1);
            }
            else if(_X == iX-1 &&
                    _Y > 0 &&
                    _Y < iY-1)
            {
                if(principalMatrix[_X-1][_Y].isHidden)
                    vCleanCell(_X-1, _Y);
                if(principalMatrix[_X][_Y+1].isHidden)
                    vCleanCell(_X, _Y+1);
                if(principalMatrix[_X-1][_Y+1].isHidden)
                    vCleanCell(_X-1, _Y+1);
                if(principalMatrix[_X][_Y-1].isHidden)
                    vCleanCell(_X, _Y-1);
                if(principalMatrix[_X-1][_Y-1].isHidden)
                    vCleanCell(_X-1, _Y-1);
            }
            else if(_X > 0 &&
                    _X < iX-1 &&
                    _Y == 0)
            {
                if(principalMatrix[_X-1][_Y].isHidden)
                    vCleanCell(_X-1, _Y);
                if(principalMatrix[_X+1][_Y].isHidden)
                    vCleanCell(_X+1, _Y);
                if(principalMatrix[_X-1][_Y+1].isHidden)
                    vCleanCell(_X-1, _Y+1);
                if(principalMatrix[_X][_Y+1].isHidden)
                    vCleanCell(_X, _Y+1);
                if(principalMatrix[_X+1][_Y+1].isHidden)
                    vCleanCell(_X+1, _Y+1);
            }
            else if(_X > 0 &&
                    _X < iX-1 &&
                    _Y == iY-1)
            {
                if(principalMatrix[_X+1][_Y].isHidden)
                    vCleanCell(_X+1, _Y);
                if(principalMatrix[_X-1][_Y].isHidden)
                    vCleanCell(_X-1, _Y);
                if(principalMatrix[_X-1][_Y-1].isHidden)
                    vCleanCell(_X-1, _Y-1);
                if(principalMatrix[_X][_Y-1].isHidden)
                    vCleanCell(_X, _Y-1);
                if(principalMatrix[_X+1][_Y-1].isHidden)
                    vCleanCell(_X+1, _Y-1);
            }
            else
            {
                if(principalMatrix[_X-1][_Y-1].isHidden)
                    vCleanCell(_X-1, _Y-1);
                if(principalMatrix[_X-1][_Y].isHidden)
                    vCleanCell(_X-1, _Y);
                if(principalMatrix[_X-1][_Y+1].isHidden)
                    vCleanCell(_X-1, _Y+1);
                if(principalMatrix[_X][_Y-1].isHidden)
                    vCleanCell(_X, _Y-1);
                if(principalMatrix[_X][_Y+1].isHidden)
                    vCleanCell(_X, _Y+1);
                if(principalMatrix[_X+1][_Y-1].isHidden)
                    vCleanCell(_X+1, _Y-1);
                if(principalMatrix[_X+1][_Y].isHidden)
                    vCleanCell(_X+1, _Y);
                if(principalMatrix[_X+1][_Y+1].isHidden)
                    vCleanCell(_X+1, _Y+1);
            }
        }
        else
        {
            principalMatrix[_X][_Y].isHidden = false;
            principalMatrix[_X][_Y].button->hide();
        }
        iHiddenCells--;
        if(iHiddenCells == 0)
        {
            vKeyboardControllUnsetCurrentCell();
            controller.active = false;
            vGameWon();
        }
    }

}

void LibreMines::vResetPrincipalMatrix()
{
    for(std::vector<Cell>& i: principalMatrix)
    {
        for (Cell& j: i)
        {
            delete j.label;
            delete j.button;
        }
    }
    principalMatrix.clear();
}

void LibreMines::vConfigureInterface()
{
    this->setFont(QFont("Liberation Sans"));
    labelTimerInGame = new QLabel (this);
    lcd_numberMinesLeft = new QLCDNumber (this);
    buttonRestartInGame = new QPushButton (this);
    buttonQuitInGame = new QPushButton (this);
    labelYouWonYouLost = new QLabel(this);
    labelStatisLastMatch = new QLabel(this);

    labelTimerInGame->setFont(QFont("Liberation Sans", 40));
    labelTimerInGame->setNum(0);
    lcd_numberMinesLeft->setDecMode();
    lcd_numberMinesLeft->display(0);
    buttonQuitInGame->setText("Quit");
    buttonRestartInGame->setText("Restart");
    labelYouWonYouLost->setFont(QFont("Liberation Sans", 15));

    vHideInterfaceInGame();

    QRect rec = QApplication::desktop()->screenGeometry();
    int height  = rec.height();
    int width = rec.width();
    this->setGeometry(0,0,width,height);
    iLimitWidth = 8*width/10;
    iLimitHeight = 9*height/10;


    buttonEasy = new QPushButton(this);
    buttonEasy->setText("Easy\n\n8x8\n\n10 Mines");
    buttonEasy->setFont(QFont("Liberation Sans",20));

    buttonMedium= new QPushButton(this);
    buttonMedium->setText("Medium\n\n16x16\n\n40 Mines");
    buttonMedium->setFont(QFont("Liberation Sans",20));


    buttonHard = new QPushButton(this);
    buttonHard->setText("Hard\n\n30x16\n\n99 Mines");
    buttonHard->setFont(QFont("Liberation Sans",20));

    buttonCustomizedNewGame = new QPushButton(this);
    buttonCustomizedNewGame->setText("Customized New Game");
    buttonCustomizedNewGame->setFont(QFont("Liberation Sans",20));

    sbCustomizedX = new QSpinBox(this);
    sbCustomizedX->setMinimum(10);
    sbCustomizedX->setMaximum(100);
    sbCustomizedX->setValue(20);

    sbCustomizedY = new QSpinBox(this);
    sbCustomizedY->setMinimum(10);
    sbCustomizedY->setMaximum(100);
    sbCustomizedY->setValue(20);


    sbCustomizednMines = new QSpinBox(this);
    sbCustomizednMines->setMinimum(0);
    sbCustomizednMines->setMaximum(100);
    sbCustomizednMines->setValue(20);


    labelCustomizedX = new QLabel(this);
    labelCustomizedX->setText("Width: ");

    labelCustomizedY = new QLabel(this);
    labelCustomizedY->setText("Height: ");

    labelCustomized_nMines = new QLabel(this);
    labelCustomized_nMines->setText("Percent of Mines: ");

    cbFirstCellClean = new QCheckBox(this);
    cbFirstCellClean->setText("First Cell Clean");
    cbFirstCellClean->setChecked(false);

    cbDarkModeEnabled = new QCheckBox(this);
    cbDarkModeEnabled->setText("Disable Dark Mode");
    cbDarkModeEnabled->setChecked(true);

    buttonEasy->setGeometry(width/20, height/20,
                            width/2 - 2*width/20, 4*(height/2 - 2*height/20)/5 );

    buttonMedium->setGeometry(buttonEasy->x() + buttonEasy->width() + width/20, height/20,
                              width/2 - 2*width/20, 4*(height/2 - 2*height/20)/5 );

    buttonHard->setGeometry(width/20, buttonEasy->y() + buttonEasy->height() + height/20,
                            width/2 - 2*width/20,  4*(height/2 - 2*height/20)/5 );

    cbFirstCellClean->setGeometry(width/20, buttonHard->y() + buttonHard->height() + height/20,
                                  width/2 - 2*width/20,  (height/2 - 2*height/20)/10 );

    cbDarkModeEnabled->setGeometry(width/20, cbFirstCellClean->y() + cbFirstCellClean->height(),
                                   width/2 - 2*width/20,  (height/2 - 2*height/20)/10);

    buttonCustomizedNewGame->setGeometry(buttonEasy->x() + buttonEasy->width() + width/20, buttonEasy->y() + buttonEasy->height() + height/20,
                                         width/2 - 2*width/20, 2*(height/2 - 2*height/20)/5 );

    labelCustomized_nMines->setGeometry(buttonCustomizedNewGame->x(), buttonCustomizedNewGame->y()+buttonCustomizedNewGame->height(),
                                        buttonCustomizedNewGame->width()/2,2*(height/2 - 2*height/20)/(3*5));

    labelCustomizedX->setGeometry(labelCustomized_nMines->x(), labelCustomized_nMines->y()+labelCustomized_nMines->height(),
                                  labelCustomized_nMines->width(),labelCustomized_nMines->height());

    labelCustomizedY->setGeometry(labelCustomizedX->x(), labelCustomizedX->y()+labelCustomizedX->height(),
                                  labelCustomizedX->width(),labelCustomizedX->height());

    sbCustomizednMines->setGeometry(labelCustomized_nMines->x()+labelCustomized_nMines->width(), labelCustomized_nMines->y(),
                                    labelCustomized_nMines->width(), labelCustomized_nMines->height());

    sbCustomizedX->setGeometry(labelCustomizedX->x()+labelCustomizedX->width(), labelCustomizedX->y(),
                               labelCustomizedX->width(), labelCustomizedX->height());

    sbCustomizedY->setGeometry(labelCustomizedY->x()+labelCustomizedY->width(), labelCustomizedY->y(),
                               labelCustomizedY->width(), labelCustomizedY->height());


    connect(buttonEasy, &QPushButton::released,
            this, &LibreMines::SLOT_Easy);

    connect(buttonMedium, &QPushButton::released,
            this, &LibreMines::SLOT_Medium);

    connect(buttonHard, &QPushButton::released,
            this, &LibreMines::SLOT_Hard);

    connect(buttonCustomizedNewGame, &QPushButton::released,
            this, &LibreMines::SLOT_Customized);

    connect(buttonRestartInGame, &QPushButton::released,
            this, &LibreMines::SLOT_Restart);

    connect(buttonQuitInGame, &QPushButton::released,
            this, &LibreMines::SLOT_Quit);

    connect(cbFirstCellClean, &QPushButton::released,
            this, &LibreMines::SLOT_UpdateFirstCellClean);

    connect(cbDarkModeEnabled, &QPushButton::released,
            this, &LibreMines::SLOT_DarkMode);
}

void LibreMines::vHideInterface()
{
    buttonEasy->hide();
    buttonMedium->hide();
    buttonHard->hide();

    buttonCustomizedNewGame->hide();

    sbCustomizedX->hide();
    sbCustomizedY->hide();
    sbCustomizednMines->hide();

    labelCustomizedX->hide();
    labelCustomizedY->hide();
    labelCustomized_nMines->hide();

    cbFirstCellClean->hide();
    cbDarkModeEnabled->hide();
}

void LibreMines::vShowInterface()
{
    buttonEasy->show();
    buttonMedium->show();
    buttonHard->show();

    buttonCustomizedNewGame->show();

    sbCustomizedX->show();
    sbCustomizedY->show();
    sbCustomizednMines->show();

    labelCustomizedX->show();
    labelCustomizedY->show();
    labelCustomized_nMines->show();

    cbFirstCellClean->show();
    cbDarkModeEnabled->show();
}

void LibreMines::SLOT_Easy()
{
    vHideInterface();

    timerTimeInGame = new QTimer(this);

    vNewGame(8, 8, 10);

    difficult = EASY;
}

void LibreMines::SLOT_Medium()
{
    vHideInterface();

    timerTimeInGame = new QTimer(this);

    vNewGame(16, 16, 40);

    difficult = MEDIUM;

}

void LibreMines::SLOT_Hard()
{
    vHideInterface();

    timerTimeInGame = new QTimer(this);

    vNewGame(30, 16, 99);

    difficult = HARD;

}

void LibreMines::SLOT_Customized()
{
    vHideInterface();

    timerTimeInGame = new QTimer(this);

    int x = sbCustomizedX->value();
    int y = sbCustomizedY->value();
    int mines = static_cast<int> (round(x*y*sbCustomizednMines->value()/100));
    vNewGame(x, y, mines);

    difficult = CUSTOMIZED;

}

void LibreMines::vAjustInterfaceInGame()
{
    int width = this->width();
    int height = this->height();

    labelTimerInGame->setGeometry(85*width/100, height/20, 15*width/100, height/8);
    lcd_numberMinesLeft->setGeometry(labelTimerInGame->x(), labelTimerInGame->y()+labelTimerInGame->height(), labelTimerInGame->width(), height/7);
    buttonRestartInGame->setGeometry(lcd_numberMinesLeft->x(), lcd_numberMinesLeft->y()+lcd_numberMinesLeft->height(), lcd_numberMinesLeft->width()/2, height/20);
    buttonQuitInGame->setGeometry(buttonRestartInGame->x()+buttonRestartInGame->width(), buttonRestartInGame->y(), buttonRestartInGame->width(), buttonRestartInGame->height());
    labelYouWonYouLost->setGeometry(lcd_numberMinesLeft->x(), buttonRestartInGame->y()+buttonRestartInGame->height()+height/10, lcd_numberMinesLeft->width(), lcd_numberMinesLeft->height());
    labelStatisLastMatch->setGeometry(labelYouWonYouLost->x(), labelYouWonYouLost->y() + labelYouWonYouLost->height(),
                                      labelYouWonYouLost->width(), height/5);
}


void LibreMines::vHideInterfaceInGame()
{
    labelTimerInGame->hide();
    lcd_numberMinesLeft->hide();
    buttonRestartInGame->hide();
    buttonQuitInGame->hide();
    labelYouWonYouLost->hide();
    labelStatisLastMatch->hide();

}

void LibreMines::vShowInterfaceInGame()
{
    labelTimerInGame->show();
    lcd_numberMinesLeft->show();
    buttonRestartInGame->show();
    buttonQuitInGame->show();
    labelYouWonYouLost->show();
    labelStatisLastMatch->show();

}

void LibreMines::SLOT_Restart()
{
    if(timerTimeInGame->isActive()
       )
        timerTimeInGame->stop();

    delete timerTimeInGame;

    timerTimeInGame = new QTimer (this);

    vResetPrincipalMatrix();

    labelStatisLastMatch->setText(" ");

    vNewGame(iX, iY, nMines);
}

void LibreMines::SLOT_Quit(){

    if (timerTimeInGame->isActive())
        timerTimeInGame->stop();

    delete timerTimeInGame;

    labelStatisLastMatch->setText(" ");

    vResetPrincipalMatrix();
    vHideInterfaceInGame();
    vShowInterface();

    difficult = NONE;
}

void LibreMines::SLOT_UpdateTime()
{
    iTimeInSeconds++;
    labelTimerInGame->setNum(iTimeInSeconds);
}



void LibreMines::vAddOrRemoveFlag(const uchar _X, const uchar _Y)
{
    if(principalMatrix[_X][_Y].hasFlag)
    {
        principalMatrix[_X][_Y].button->setIcon(QIcon(QPixmap::fromImage(*imgNoFlag).scaled(fm, fm, Qt::KeepAspectRatio)));
        principalMatrix[_X][_Y].button->setIconSize(QSize(fm, fm));
        principalMatrix[_X][_Y].hasFlag = false;
        iMinesLeft++;
        lcd_numberMinesLeft->display(iMinesLeft);
    }
    else
    {
        principalMatrix[_X][_Y].button->setIcon(QIcon(QPixmap::fromImage(*imgFlag).scaled(fm, fm, Qt::KeepAspectRatio)));
        principalMatrix[_X][_Y].button->setIconSize(QSize(fm, fm));
        principalMatrix[_X][_Y].hasFlag = true;
        iMinesLeft--;
        lcd_numberMinesLeft->display(iMinesLeft);
    }

}

void LibreMines::SLOT_OnButtonClicked(const QMouseEvent *e)
{
    QPushButton_adapted *buttonClicked = (QPushButton_adapted *) sender();

    for(uchar j=0; j<iY; j++)
    {
        for (uchar i=0; i<iX; i++)
        {
            if(buttonClicked == principalMatrix[i][j].button)
            {
                switch (e->button())
                {
                    case Qt::RightButton:

                        vAddOrRemoveFlag(i, j);
                        return;

                    case Qt::LeftButton:

                        if(bFirst)
                        {
                            if(bFirstCellClean && principalMatrix[i][j].state != ZERO)
                            {
                                vNewGame(iX, iY, nMines, i, j);
                            }
                            bFirst = false;
                            vStartTimer();
                            qDebug() << "Start";
                        }

                        if(principalMatrix[i][j].state == MINE)
                        {
                            if(!principalMatrix[i][j].hasFlag)
                                vGameLost(i, j);
                        }
                        else
                            vCleanCell(i, j);

                        return;

                    default:
                        return;
                }
            }
        }
    }
}

void LibreMines::vStartTimer()
{
    iTimeInSeconds = 0;
    timerTimeInGame->start(1e3);

    connect(timerTimeInGame, &QTimer::timeout,
            this, &LibreMines::SLOT_UpdateTime);
}

void LibreMines::SLOT_UpdateFirstCellClean()
{
    bFirstCellClean = cbFirstCellClean->isChecked();
}

void LibreMines::SLOT_DarkMode()
{
    vConfigureDarkMode(cbDarkModeEnabled->isChecked());

    if(cbDarkModeEnabled->isChecked())
        cbDarkModeEnabled->setText("Disable dark mode");

    else
        cbDarkModeEnabled->setText("Enable dark mode");
}

void LibreMines::vGenerateStatics()
{
    int iCorrectFlags = 0,
            iWrongFlags = 0,
            iUnlockedCells = iCellsToUnlock - iHiddenCells;

    for (int i=0; i<(iX-1); i++)
    {
        for (int j=0; j<=(iY-1); j++)
        {
            if(principalMatrix[i][j].hasFlag)
            {
                if (principalMatrix[i][j].state == MINE)
                    iCorrectFlags++;
                else
                    iWrongFlags++;
            }
        }
    }
    if(iTimeInSeconds == 0)
        iTimeInSeconds = 1;

    double dFlagsPerSecond = (double)iCorrectFlags/iTimeInSeconds,
            dCellsPerSecond = (double)iUnlockedCells/iTimeInSeconds,
            dPercentageGameComplete = (double)100*iUnlockedCells/iCellsToUnlock;

    QString QS_Statics = "Correct Flags: " + QString::number(iCorrectFlags)
                         +"\nWrongFlags: " + QString::number(iWrongFlags)
                         +"\nUnlocked Cells: " + QString::number(iUnlockedCells)
                         +"\n"
                         +"\nFlags/s: " + QString::number(dFlagsPerSecond, 'f', 3)
                         +"\nCells/s: " + QString::number(dCellsPerSecond, 'f', 3)
                         +"\n"
                         +"\nGame Complete: " + QString::number(dPercentageGameComplete, 'f', 2) + " %";

    labelStatisLastMatch->setText(QS_Statics);
}

void LibreMines::vConfigureDarkMode(const bool bDark)
{
    if(bDark)
    {
        imgZero->load(":/Media_rsc/Images/Minesweeper_zero_dark.png");
        imgOne->load(":/Media_rsc/Images/Minesweeper_one_dark.png");
        imgTwo->load(":/Media_rsc/Images/Minesweeper_two_dark.png");
        imgThree->load(":/Media_rsc/Images/Minesweeper_three_dark.png");
        imgFour->load(":/Media_rsc/Images/Minesweeper_four_dark.png");
        imgFive->load(":/Media_rsc/Images/Minesweeper_five_dark.png");
        imgSix->load(":/Media_rsc/Images/Minesweeper_six_dark.png");
        imgSeven->load(":/Media_rsc/Images/Minesweeper_seven_dark.png");
        imgEight->load(":/Media_rsc/Images/Minesweeper_eight_dark.png");
        imgFlag->load(":/Media_rsc/Images/Minesweeper_flag_dark.png");
        imgNoFlag->load(":/Media_rsc/Images/Minesweeper_no_flag_dark.png");
        imgMine->load(":/Media_rsc/Images/Minesweeper_mine_dark.png");
        imgBoom->load(":/Media_rsc/Images/Minesweeper_boom_dark.png");
        imgWrongFlag->load(":/Media_rsc/Images/Minesweeper_wrong_flag_dark.png");

        qApp->setStyle (QStyleFactory::create ("Fusion"));
        QPalette darkPalette;
        darkPalette.setColor (QPalette::BrightText,      Qt::red);
        darkPalette.setColor (QPalette::WindowText,      Qt::white);
        darkPalette.setColor (QPalette::ToolTipBase,     Qt::white);
        darkPalette.setColor (QPalette::ToolTipText,     Qt::white);
        darkPalette.setColor (QPalette::Text,            Qt::white);
        darkPalette.setColor (QPalette::ButtonText,      Qt::white);
        darkPalette.setColor (QPalette::HighlightedText, Qt::black);
        darkPalette.setColor (QPalette::Window,          QColor (53, 53, 53));
        darkPalette.setColor (QPalette::Base,            QColor (25, 25, 25));
        darkPalette.setColor (QPalette::AlternateBase,   QColor (53, 53, 53));
        darkPalette.setColor (QPalette::Button,          QColor (53, 53, 53));
        darkPalette.setColor (QPalette::Link,            QColor (42, 130, 218));
        darkPalette.setColor (QPalette::Highlight,       QColor (42, 130, 218));

        qApp->setPalette(darkPalette);
    }
    else
    {
        imgZero->load(":/Media_rsc/Images/Minesweeper_zero_light.png");
        imgOne->load(":/Media_rsc/Images/Minesweeper_one_light.png");
        imgTwo->load(":/Media_rsc/Images/Minesweeper_two_light.png");
        imgThree->load(":/Media_rsc/Images/Minesweeper_three_light.png");
        imgFour->load(":/Media_rsc/Images/Minesweeper_four_light.png");
        imgFive->load(":/Media_rsc/Images/Minesweeper_five_light.png");
        imgSix->load(":/Media_rsc/Images/Minesweeper_six_light.png");
        imgSeven->load(":/Media_rsc/Images/Minesweeper_seven_light.png");
        imgEight->load(":/Media_rsc/Images/Minesweeper_eight_light.png");
        imgFlag->load(":/Media_rsc/Images/Minesweeper_flag_light.png");
        imgNoFlag->load(":/Media_rsc/Images/Minesweeper_no_flag_light.png");
        imgMine->load(":/Media_rsc/Images/Minesweeper_mine_light.png");
        imgBoom->load(":/Media_rsc/Images/Minesweeper_boom_light.png");
        imgWrongFlag->load(":/Media_rsc/Images/Minesweeper_wrong_flag_light.png");

        qApp->setStyle (QStyleFactory::create ("Fusion"));
        QPalette lightPalette;
        lightPalette.setColor (QPalette::BrightText,      Qt::cyan);
        lightPalette.setColor (QPalette::WindowText,      Qt::black);
        lightPalette.setColor (QPalette::ToolTipBase,     Qt::black);
        lightPalette.setColor (QPalette::ToolTipText,     Qt::black);
        lightPalette.setColor (QPalette::Text,            Qt::black);
        lightPalette.setColor (QPalette::ButtonText,      Qt::black);
        lightPalette.setColor (QPalette::HighlightedText, Qt::white);
        lightPalette.setColor (QPalette::Window,          QColor (202, 202, 202));
        lightPalette.setColor (QPalette::Base,            QColor (228, 228, 228));
        lightPalette.setColor (QPalette::AlternateBase,   QColor (202, 202, 202));
        lightPalette.setColor (QPalette::Button,          QColor (202, 202, 202));
        lightPalette.setColor (QPalette::Link,            QColor (213, 125, 37));
        lightPalette.setColor (QPalette::Highlight,       QColor (213, 225, 37));

        qApp->setPalette(lightPalette);
    }
}

void LibreMines::vKeyboardControllerSetCurrentCell(const uchar x, const uchar y)
{
    controller.currentX = x;
    controller.currentY = y;

    Cell& cell = principalMatrix[controller.currentX][controller.currentY];

    if(cell.isHidden)
    {

        QImage img = cell.hasFlag ? *imgFlag : *imgNoFlag;
        img.invertPixels();
        cell.button->setIcon(QIcon(QPixmap::fromImage(img).scaled(fm, fm, Qt::KeepAspectRatio)));
    }
    else
    {
        QImage img = QImage();

        if(cell.state == ZERO)
            img = *imgZero;
        else if(cell.state == ONE)
            img = *imgOne;
        else if(cell.state == TWO)
            img = *imgTwo;
        else if(cell.state == THREE)
            img = *imgThree;
        else if(cell.state == FOUR)
            img = *imgFour;
        else if(cell.state == FIVE)
            img = *imgFive;
        else if(cell.state == SIX)
            img = *imgSix;
        else if(cell.state == SEVEN)
            img = *imgSeven;
        else if(cell.state == EIGHT)
            img = *imgEight;
        else
            qFatal(Q_FUNC_INFO);

        img.invertPixels();

        cell.label->setPixmap(QPixmap::fromImage(img).scaled(fm, fm, Qt::KeepAspectRatio));
    }
}

void LibreMines::vKeyboardControllUnsetCurrentCell()
{
    Cell& cell = principalMatrix[controller.currentX][controller.currentY];

    if(cell.isHidden)
    {
        QImage* img = cell.hasFlag ? imgFlag : imgNoFlag;
        cell.button->setIcon(QIcon(QPixmap::fromImage(*img).scaled(fm, fm, Qt::KeepAspectRatio)));
    }
    else
    {
        QImage* img = nullptr;

        if(cell.state == ZERO)
            img = imgZero;
        else if(cell.state == ONE)
            img = imgOne;
        else if(cell.state == TWO)
            img = imgTwo;
        else if(cell.state == THREE)
            img = imgThree;
        else if(cell.state == FOUR)
            img = imgFour;
        else if(cell.state == FIVE)
            img = imgFive;
        else if(cell.state == SIX)
            img = imgSix;
        else if(cell.state == SEVEN)
            img = imgSeven;
        else if(cell.state == EIGHT)
            img = imgEight;
        else
            qFatal(Q_FUNC_INFO);

        cell.label->setPixmap(QPixmap::fromImage(*img).scaled(fm, fm, Qt::KeepAspectRatio));
    }
}

void LibreMines::vKeyboardControllerMoveLeft()
{
    vKeyboardControllUnsetCurrentCell();

    uchar destX = (controller.ctrlPressed) ? 0 : (controller.currentX == 0) ? iX - 1 : (controller.currentX - 1);

    vKeyboardControllerSetCurrentCell(destX, controller.currentY);
}

void LibreMines::vKeyboardControllerMoveRight()
{
    vKeyboardControllUnsetCurrentCell();

    uchar destX = (controller.ctrlPressed) ? iX - 1 : (controller.currentX == iX - 1) ? 0 : (controller.currentX + 1);

    vKeyboardControllerSetCurrentCell(destX, controller.currentY);
}

void LibreMines::vKeyboardControllerMoveDown()
{
    vKeyboardControllUnsetCurrentCell();

    uchar destY = (controller.ctrlPressed) ? iY - 1 : (controller.currentY == iY - 1) ? 0 : (controller.currentY + 1);

    vKeyboardControllerSetCurrentCell(controller.currentX, destY);
}

void LibreMines::vKeyboardControllerMoveUp()
{
    vKeyboardControllUnsetCurrentCell();

    uchar destY = (controller.ctrlPressed) ? 0 : (controller.currentY == 0) ? iY -1 : (controller.currentY - 1);

    vKeyboardControllerSetCurrentCell(controller.currentX, destY);
}

void LibreMines::vKeyboardControllerUpdateCurrentCell()
{
    vKeyboardControllUnsetCurrentCell();
    vKeyboardControllerSetCurrentCell(controller.currentX, controller.currentY);
}
