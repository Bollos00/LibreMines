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
#include <QScreen>
#include <QDir>
#include <QLineEdit>
#include <QAction>
#include <QMenuBar>
#include <QStatusBar>

#include "libreminesgui.h"
#include "libreminesscoresdialog.h"

LibreMinesGui::CellGui::CellGui():
    button(nullptr),
    label(nullptr)
{

}

LibreMinesGui::LibreMinesGui(QWidget *parent, const int thatWidth, const int thatHeight, const int thatMaximumCellLength) :
    QMainWindow(parent),
    gameEngine(),
    principalMatrix( std::vector< std::vector<CellGui> >(0) ),
    iLimitHeightField( 0 ),
    iLimitWidthField( 0 ),
    iWidthMainWindow( 0 ),
    iHeightMainWindow( 0 ),
    cellLength( 0 ),
    maximumCellLength (thatMaximumCellLength),
    difficult( NONE ),
    imgZero ( new QImage(":/images_rsc/Media/Minesweeper_zero_dark.png") ),
    imgOne ( new QImage(":/images_rsc/Media/Minesweeper_one_dark.png") ),
    imgTwo ( new QImage(":/images_rsc/Media/Minesweeper_two_dark.png") ),
    imgThree ( new QImage(":/images_rsc/Media/Minesweeper_three_dark.png") ),
    imgFour( new QImage(":/images_rsc/Media/Minesweeper_four_dark.png") ),
    imgFive ( new QImage(":/images_rsc/Media/Minesweeper_five_dark.png") ),
    imgSix( new QImage(":/images_rsc/Media/Minesweeper_six_dark.png") ),
    imgSeven ( new QImage(":/images_rsc/Media/Minesweeper_seven_dark.png") ),
    imgEight ( new QImage(":/images_rsc/Media/Minesweeper_eight_dark.png") ),
    imgFlag ( new QImage(":/images_rsc/Media/Minesweeper_flag_dark.png") ),
    imgNoFlag ( new QImage(":/images_rsc/Media/Minesweeper_no_flag_dark.png") ),
    imgMine ( new QImage(":/images_rsc/Media/Minesweeper_mine_dark.png") ),
    imgBoom ( new QImage(":/images_rsc/Media/Minesweeper_boom_dark.png") ),
    imgWrongFlag ( new QImage(":/images_rsc/Media/Minesweeper_wrong_flag_dark.png") ),
    preferences( new LibreMinesPreferencesDialog(this) )
{
    connect(preferences, &LibreMinesPreferencesDialog::SIGNAL_optionChanged,
            this, &LibreMinesGui::SLOT_optionChanged);

    // Unable central widget when the preferences dialog is active
    connect(preferences, &LibreMinesPreferencesDialog::SIGNAL_visibilityChanged,
            [this](const bool visible){ this->centralWidget()->setEnabled(!visible); });

    // Create interface with the passed dimensions
    vConfigureInterface(thatWidth, thatHeight);

    qApp->installEventFilter(this);

    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowIcon(QIcon(":/icons_rsc/icons/libremines.svg"));

    // Initializr keyboard controller attributes
    controller.ctrlPressed = false;
    controller.active = false;
    controller.currentX = 0;
    controller.currentY = 0;

    // Load configuration file and set the theme
    vLastSessionLoadConfigurationFile();
    vConfigureTheme(preferences->optionTheme());
}

LibreMinesGui::~LibreMinesGui()
{
    vResetPrincipalMatrix();

    vLastSessionSaveConfigurationFile();

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

bool LibreMinesGui::eventFilter(QObject* object, QEvent* event)
{
    Q_UNUSED(object)
    // If the game is not running, do not deal woth the event
    if(!gameEngine || !gameEngine->isGameActive())
        return false;

    if(principalMatrix.empty() ||
       !principalMatrix[0][0].button ||
       !principalMatrix[0][0].button->isEnabled())
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
                        const LibreMinesGameEngine::CellGameEngine& cell =
                                gameEngine->getPrincipalMatrix()[controller.currentX][controller.currentY];

                        if(cell.isHidden)
                        {
                            emit SIGNAL_cleanCell(controller.currentX, controller.currentY);
                        }
                        else if(preferences->optionCleanNeighborCellsWhenClickedOnShowedCell())
                        {
                            emit SIGNAL_cleanNeighborCells(controller.currentX, controller.currentY);
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
                        emit SIGNAL_addOrRemoveFlag(controller.currentX, controller.currentY);
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

void LibreMinesGui::vNewGame(const uchar _X,
                             const uchar _Y,
                             ushort i_nMines_)
{
    // Reset the controller attributes
    controller.ctrlPressed = false;
    controller.active = false;
    controller.currentX = 0;
    controller.currentY = 0;

    // Create a new matrix
    principalMatrix = std::vector<std::vector<CellGui>> (_X, std::vector<CellGui>(_Y));

    buttonQuitInGame->setEnabled(false);
    buttonRestartInGame->setEnabled(false);

    // Create the game engine instance
    gameEngine.reset(new LibreMinesGameEngine());

    gameEngine->setFirstCellClean(preferences->optionFirstCellClean());
    gameEngine->vNewGame(_X, _Y, i_nMines_);

    // Set the length of each cell
    if(iLimitWidthField/_X < iLimitHeightField/_Y)
        cellLength = iLimitWidthField/_X;
    else
        cellLength = iLimitHeightField/_Y;

    if(cellLength > maximumCellLength)
        cellLength = maximumCellLength;

    const QPixmap QPM_Zero = QPixmap::fromImage(*imgZero).scaled(cellLength, cellLength, Qt::KeepAspectRatio);
    const QPixmap QPM_NoFlag = QPixmap::fromImage(*imgNoFlag).scaled(cellLength, cellLength, Qt::KeepAspectRatio);

    const bool bCleanNeighborCellsWhenClickedOnShowedLabel = preferences->optionCleanNeighborCellsWhenClickedOnShowedCell();

    // Create each cell
    for(uchar j=0; j<_Y; j++)
    {
        for (uchar i=0; i<_X; i++)
        {
            CellGui& cell = principalMatrix[i][j];

            cell.label = new QLabel_adapted(this);
            cell.button = new QPushButton_adapted(this);

            cell.label->setGeometry(i*cellLength, j*cellLength, cellLength, cellLength);
            cell.label->setPixmap(QPM_Zero);
            cell.label->show();

            cell.button->setGeometry(i*cellLength, j*cellLength, cellLength, cellLength);
            cell.button->setIcon(QIcon(QPM_NoFlag));
            cell.button->setIconSize(QSize(cellLength, cellLength));
            cell.button->show();
            cell.button->setEnabled(false);

            connect(cell.button, &QPushButton_adapted::SIGNAL_Clicked,
                    this, &LibreMinesGui::SLOT_OnCellButtonClicked);

            if(bCleanNeighborCellsWhenClickedOnShowedLabel)
            {
                connect(cell.label, &QLabel_adapted::SIGNAL_clicked,
                        this, &LibreMinesGui::SLOT_onCellLabelClicked);
            }

            qApp->processEvents();
        }
    }

    vAjustInterfaceInGame();
    vShowInterfaceInGame();
    labelTimerInGame->setNum(0);
    labelYouWonYouLost->setText(" ");

    buttonQuitInGame->setEnabled(true);
    buttonRestartInGame->setEnabled(true);

    // Set the correct state of each cell
    vAttributeAllCells();

    // Communication (GameEngine -> GUI)
    connect(gameEngine.get(), &LibreMinesGameEngine::SIGNAL_showCell,
            this, &LibreMinesGui::SLOT_showCell);
    connect(gameEngine.get(), &LibreMinesGameEngine::SIGNAL_endGameScore,
            this, &LibreMinesGui::SLOT_endGameScore);
    connect(gameEngine.get(), &LibreMinesGameEngine::SIGNAL_currentTime,
            this, &LibreMinesGui::SLOT_currentTime);
    connect(gameEngine.get(), &LibreMinesGameEngine::SIGNAL_minesLeft,
            this, &LibreMinesGui::SLOT_minesLeft);
    connect(gameEngine.get(), &LibreMinesGameEngine::SIGNAL_flagCell,
            this, &LibreMinesGui::SLOT_flagCell);
    connect(gameEngine.get(), &LibreMinesGameEngine::SIGNAL_unflagCell,
            this, &LibreMinesGui::SLOT_unflagCell);
    connect(gameEngine.get(), &LibreMinesGameEngine::SIGNAL_gameWon,
            this, &LibreMinesGui::SLOT_gameWon);
    connect(gameEngine.get(), &LibreMinesGameEngine::SIGNAL_gameLost,
            this, &LibreMinesGui::SLOT_gameLost);
    connect(gameEngine.get(), &LibreMinesGameEngine::SIGNAL_remakeGame,
            this, &LibreMinesGui::SLOT_remakeGame);

    // Communication (GUI -> GameEngine)
    connect(this, &LibreMinesGui::SIGNAL_cleanCell,
            gameEngine.get(), &LibreMinesGameEngine::SLOT_cleanCell);
    if(bCleanNeighborCellsWhenClickedOnShowedLabel)
    {
        connect(this, &LibreMinesGui::SIGNAL_cleanNeighborCells,
                gameEngine.get(), &LibreMinesGameEngine::SLOT_cleanNeighborCells);
    }
    connect(this, &LibreMinesGui::SIGNAL_addOrRemoveFlag,
            gameEngine.get(), &LibreMinesGameEngine::SLOT_addOrRemoveFlag);
    connect(this, &LibreMinesGui::SIGNAL_stopGame,
            gameEngine.get(), &LibreMinesGameEngine::SLOT_stop);
}

void LibreMinesGui::vAttributeAllCells()
{
    const QPixmap
            QPM_Zero = QPixmap::fromImage(*imgZero).scaled(cellLength, cellLength, Qt::KeepAspectRatio),
            QPM_One = QPixmap::fromImage(*imgOne).scaled(cellLength, cellLength, Qt::KeepAspectRatio),
            QPM_Two = QPixmap::fromImage(*imgTwo).scaled(cellLength, cellLength, Qt::KeepAspectRatio),
            QPM_Three = QPixmap::fromImage(*imgThree).scaled(cellLength, cellLength, Qt::KeepAspectRatio),
            QPM_Four = QPixmap::fromImage(*imgFour).scaled(cellLength, cellLength, Qt::KeepAspectRatio),
            QPM_Five = QPixmap::fromImage(*imgFive).scaled(cellLength, cellLength, Qt::KeepAspectRatio),
            QPM_Six = QPixmap::fromImage(*imgSix).scaled(cellLength, cellLength, Qt::KeepAspectRatio),
            QPM_Seven = QPixmap::fromImage(*imgSeven).scaled(cellLength, cellLength, Qt::KeepAspectRatio),
            QPM_Eight = QPixmap::fromImage(*imgEight).scaled(cellLength, cellLength, Qt::KeepAspectRatio),
            QPM_Mine = QPixmap::fromImage(*imgMine).scaled(cellLength, cellLength, Qt::KeepAspectRatio);

    for(uchar j=0; j<gameEngine->lines(); ++j)
    {
        for(uchar i=0; i<gameEngine->rows(); ++i)
        {
            CellGui& cell = principalMatrix[i][j];

            cell.button->setEnabled(true);

            switch(gameEngine->getPrincipalMatrix()[i][j].state)
            {

                case ZERO:
                    cell.label->setPixmap(QPM_Zero);
                    break;
                case ONE:
                    cell.label->setPixmap(QPM_One);
                    break;
                case TWO:
                    cell.label->setPixmap(QPM_Two);
                    break;
                case THREE:
                    cell.label->setPixmap(QPM_Three);
                    break;
                case FOUR:
                    cell.label->setPixmap(QPM_Four);
                    break;
                case FIVE:
                    cell.label->setPixmap(QPM_Five);
                    break;
                case SIX:
                    cell.label->setPixmap(QPM_Six);
                    break;
                case SEVEN:
                    cell.label->setPixmap(QPM_Seven);
                    break;
                case EIGHT:
                    cell.label->setPixmap(QPM_Eight);
                    break;
                case MINE:
                    cell.label->setPixmap(QPM_Mine);
                    break;
            }
        }
    }
}

void LibreMinesGui::vResetPrincipalMatrix()
{
    for(std::vector<CellGui>& i: principalMatrix)
    {
        for (CellGui& j: i)
        {
            delete j.label;
            delete j.button;
        }
    }
    principalMatrix.clear();
}

void LibreMinesGui::vConfigureInterface(int width, int height)
{
    // Create the interface

    setCentralWidget(new QWidget(this));

    if(width == -1 || height == -1)
    {
        QRect rec = QGuiApplication::primaryScreen()->geometry();
        iHeightMainWindow  = rec.height();
        iWidthMainWindow = rec.width();
    }
    else
    {
        iWidthMainWindow = width;
        iHeightMainWindow = height;
    }


    actionPreferences = new QAction(this);
    actionAbout = new QAction(this);

    QMenuBar* menuBarGlobal = new QMenuBar(this);

    menuBarGlobal->setGeometry(0, 0, iWidthMainWindow, 100);

    menuOptions = new QMenu(menuBarGlobal);
    menuHelp = new QMenu(menuBarGlobal);

    this->setMenuBar(menuBarGlobal);
    this->setStatusBar(new QStatusBar(this));

    menuBarGlobal->addAction(menuOptions->menuAction());
    menuBarGlobal->addAction(menuHelp->menuAction());

    menuOptions->addAction(actionPreferences);
    menuHelp->addAction(actionAbout);

    menuOptions->setTitle("Options");
    menuHelp->setTitle("Help");

    actionPreferences->setText("Preferences...");
    actionAbout->setText("About...");

    this->setFont(QFont("Liberation Sans"));
    labelTimerInGame = new QLabel(centralWidget());
    lcd_numberMinesLeft = new QLCDNumber(centralWidget());
    buttonRestartInGame = new QPushButton(centralWidget());
    buttonQuitInGame = new QPushButton(centralWidget());
    labelYouWonYouLost = new QLabel(centralWidget());
    labelStatisLastMatch = new QLabel(centralWidget());

    labelTimerInGame->setFont(QFont("Liberation Sans", 40));
    labelTimerInGame->setNum(0);
    lcd_numberMinesLeft->setDecMode();
    lcd_numberMinesLeft->display(0);;
    buttonQuitInGame->setText("Quit");
    buttonRestartInGame->setText("Restart");
    labelYouWonYouLost->setFont(QFont("Liberation Sans", 15));

    vHideInterfaceInGame();

    this->setGeometry(0,0,iWidthMainWindow,iHeightMainWindow);
    iLimitWidthField = 8*iWidthMainWindow/10;
    iLimitHeightField = 9*iHeightMainWindow/10;


    buttonEasy = new QPushButton(centralWidget());
    buttonEasy->setText("Easy\n\n8x8\n\n10 Mines");
    buttonEasy->setFont(QFont("Liberation Sans",20));

    buttonMedium= new QPushButton(centralWidget());
    buttonMedium->setText("Medium\n\n16x16\n\n40 Mines");
    buttonMedium->setFont(QFont("Liberation Sans",20));


    buttonHard = new QPushButton(centralWidget());
    buttonHard->setText("Hard\n\n30x16\n\n99 Mines");
    buttonHard->setFont(QFont("Liberation Sans",20));

    buttonCustomizedNewGame = new QPushButton(centralWidget());
    buttonCustomizedNewGame->setText("Customized New Game");
    buttonCustomizedNewGame->setFont(QFont("Liberation Sans",20));

    sbCustomizedX = new QSpinBox(centralWidget());
    sbCustomizedX->setMinimum(10);
    sbCustomizedX->setMaximum(100);
    sbCustomizedX->setValue(20);

    sbCustomizedY = new QSpinBox(centralWidget());
    sbCustomizedY->setMinimum(10);
    sbCustomizedY->setMaximum(100);
    sbCustomizedY->setValue(20);


    sbCustomizednMines = new QSpinBox(centralWidget());
    sbCustomizednMines->setMinimum(0);
    sbCustomizednMines->setMaximum(100);
    sbCustomizednMines->setValue(20);


    labelCustomizedX = new QLabel(centralWidget());
    labelCustomizedX->setText("Width: ");

    labelCustomizedY = new QLabel(centralWidget());
    labelCustomizedY->setText("Height: ");

    labelCustomized_nMines = new QLabel(centralWidget());
    labelCustomized_nMines->setText("Percent of Mines: ");


    buttonEasy->setGeometry(iWidthMainWindow/20, iHeightMainWindow/20,
                            iWidthMainWindow/2 - 2*iWidthMainWindow/20, 4*(iHeightMainWindow/2 - 2*iHeightMainWindow/20)/5 );

    buttonMedium->setGeometry(buttonEasy->x() + buttonEasy->width() + iWidthMainWindow/20, iHeightMainWindow/20,
                              iWidthMainWindow/2 - 2*iWidthMainWindow/20, 4*(iHeightMainWindow/2 - 2*iHeightMainWindow/20)/5 );

    buttonHard->setGeometry(iWidthMainWindow/20, buttonEasy->y() + buttonEasy->height() + iHeightMainWindow/20,
                            iWidthMainWindow/2 - 2*iWidthMainWindow/20,  4*(iHeightMainWindow/2 - 2*iHeightMainWindow/20)/5 );

    buttonCustomizedNewGame->setGeometry(buttonEasy->x() + buttonEasy->width() + iWidthMainWindow/20, buttonEasy->y() + buttonEasy->height() + iHeightMainWindow/20,
                                         iWidthMainWindow/2 - 2*iWidthMainWindow/20, 2*(iHeightMainWindow/2 - 2*iHeightMainWindow/20)/5 );

    labelCustomized_nMines->setGeometry(buttonCustomizedNewGame->x(), buttonCustomizedNewGame->y()+buttonCustomizedNewGame->height(),
                                        buttonCustomizedNewGame->width()/2,2*(iHeightMainWindow/2 - 2*iHeightMainWindow/20)/(3*5));

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
            this, &LibreMinesGui::SLOT_Easy);

    connect(buttonMedium, &QPushButton::released,
            this, &LibreMinesGui::SLOT_Medium);

    connect(buttonHard, &QPushButton::released,
            this, &LibreMinesGui::SLOT_Hard);

    connect(buttonCustomizedNewGame, &QPushButton::released,
            this, &LibreMinesGui::SLOT_Customized);

    connect(buttonRestartInGame, &QPushButton::released,
            this, &LibreMinesGui::SLOT_Restart);

    connect(buttonQuitInGame, &QPushButton::released,
            this, &LibreMinesGui::SLOT_Quit);

    connect(actionPreferences, &QAction::triggered,
            preferences, &QDialog::show);

    // todo
    connect(actionAbout, &QAction::triggered,
            [this](){/* Show About Dialog*/});
}

void LibreMinesGui::vHideInterface()
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

    // Hide de menu bar too
    menuBar()->hide();
}

void LibreMinesGui::vShowInterface()
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

    menuBar()->show();
}

void LibreMinesGui::SLOT_Easy()
{
    vHideInterface();
    vNewGame(8, 8, 10);

    difficult = EASY;
}

void LibreMinesGui::SLOT_Medium()
{
    vHideInterface();
    vNewGame(16, 16, 40);

    difficult = MEDIUM;

}

void LibreMinesGui::SLOT_Hard()
{
    vHideInterface();
    vNewGame(30, 16, 99);

    difficult = HARD;

}

void LibreMinesGui::SLOT_Customized()
{
    vHideInterface();

    int x = sbCustomizedX->value();
    int y = sbCustomizedY->value();
    int mines = static_cast<int> (round(x*y*sbCustomizednMines->value()/100));
    vNewGame(x, y, mines);

    difficult = CUSTOMIZED;

}

void LibreMinesGui::vAjustInterfaceInGame()
{
    int width = this->width();
    int height = this->height();

    labelTimerInGame->setGeometry(85*width/100, height/20,
                                  15*width/100, height/8);
    lcd_numberMinesLeft->setGeometry(labelTimerInGame->x(), labelTimerInGame->y()+labelTimerInGame->height(),
                                     labelTimerInGame->width(), height/7);
    buttonRestartInGame->setGeometry(lcd_numberMinesLeft->x(), lcd_numberMinesLeft->y()+lcd_numberMinesLeft->height(),
                                     lcd_numberMinesLeft->width()/2, height/20);
    buttonQuitInGame->setGeometry(buttonRestartInGame->x()+buttonRestartInGame->width(), buttonRestartInGame->y(),
                                  buttonRestartInGame->width(), buttonRestartInGame->height());
    labelYouWonYouLost->setGeometry(lcd_numberMinesLeft->x(), buttonRestartInGame->y()+buttonRestartInGame->height()+height/10,
                                    lcd_numberMinesLeft->width(), lcd_numberMinesLeft->height());
    labelStatisLastMatch->setGeometry(labelYouWonYouLost->x(), labelYouWonYouLost->y() + labelYouWonYouLost->height(),
                                      labelYouWonYouLost->width(), height/5);
}


void LibreMinesGui::vHideInterfaceInGame()
{
    labelTimerInGame->hide();
    lcd_numberMinesLeft->hide();
    buttonRestartInGame->hide();
    buttonQuitInGame->hide();
    labelYouWonYouLost->hide();
    labelStatisLastMatch->hide();
}

void LibreMinesGui::vShowInterfaceInGame()
{
    labelTimerInGame->show();
    lcd_numberMinesLeft->show();
    buttonRestartInGame->show();
    buttonQuitInGame->show();
    labelYouWonYouLost->show();
    labelStatisLastMatch->show();
}

void LibreMinesGui::SLOT_Restart()
{
    vResetPrincipalMatrix();

    labelStatisLastMatch->setText(" ");

    uchar x = gameEngine->rows();
    uchar y = gameEngine->lines();
    ushort mines = gameEngine->mines();

    emit SIGNAL_stopGame();
    vResetPrincipalMatrix();
    vNewGame(x, y, mines);
}

void LibreMinesGui::SLOT_Quit()
{
    labelStatisLastMatch->setText(" ");

    emit SIGNAL_stopGame();

    vResetPrincipalMatrix();
    vHideInterfaceInGame();
    vShowInterface();

    difficult = NONE;

    gameEngine.reset();
}

void LibreMinesGui::SLOT_OnCellButtonClicked(const QMouseEvent *const e)
{
    if(!gameEngine->isGameActive() || controller.active)
        return;

    QPushButton_adapted *buttonClicked = (QPushButton_adapted *) sender();

    for(uchar j=0; j<gameEngine->lines(); j++)
    {
        for (uchar i=0; i<gameEngine->rows(); i++)
        {
            // Find the emissor of the signal
            if(buttonClicked == principalMatrix[i][j].button)
            {
                switch (e->button())
                {
                    case Qt::RightButton:
                        emit SIGNAL_addOrRemoveFlag(i, j);
                        return;

                    case Qt::LeftButton:
                        emit SIGNAL_cleanCell(i, j);
                        return;

                    default:
                        return;
                }
            }
        }
    }
}

void LibreMinesGui::SLOT_onCellLabelClicked(const QMouseEvent *const e)
{
    if(!gameEngine->isGameActive() || controller.active)
        return;

    QLabel_adapted *buttonClicked = (QLabel_adapted *) sender();

    for(uchar j=0; j<gameEngine->lines(); j++)
    {
        for (uchar i=0; i<gameEngine->rows(); i++)
        {
            // Find the emissor of the signal
            if(buttonClicked == principalMatrix[i][j].label)
            {
                switch (e->button())
                {
                    case Qt::LeftButton:
                        emit SIGNAL_cleanNeighborCells(i, j);
                        return;

                    default:
                        return;
                }
            }
        }
    }

}

void LibreMinesGui::SLOT_showCell(const uchar _X, const uchar _Y)
{
    principalMatrix[_X][_Y].button->hide();

    if(controller.active && controller.currentX == _X && controller.currentY == _Y)
    {
        vKeyboardControllUnsetCurrentCell();
        vKeyboardControllerSetCurrentCell(controller.currentX, controller.currentY);
    }
}

void LibreMinesGui::SLOT_endGameScore(LibreMinesScore score,
                                      int iCorrectFlags,
                                      int iWrongFlags,
                                      int iUnlockedCells,
                                      double dFlagsPerSecond,
                                      double dCellsPerSecond)
{
    score.gameDifficulty = difficult;
    score.username = preferences->optionUsername();
    if(score.username.isEmpty())
        score.username = qgetenv("USER");

    // Save the score of the current game on the file scoresLibreMines on
    //  the "~/.local/share/libremines/" directory. If the file does not
    //  exist, a new one will be created.
    if(score.dPercentageGameCompleted != 0)
    {
        QDir destDir = QDir::home();

        destDir.setFilter(QDir::AllDirs);

        if(!destDir.cd(".local"))
        {
            Q_ASSERT(destDir.mkdir(".local"));
            Q_ASSERT(destDir.cd(".local"));
        }
        if(!destDir.cd("share"))
        {
            Q_ASSERT(destDir.mkdir("share"));
            Q_ASSERT(destDir.cd("share"));
        }
        if(!destDir.cd("libremines"))
        {
            Q_ASSERT(destDir.mkdir("libremines"));
            Q_ASSERT(destDir.cd("libremines"));
        }

        QScopedPointer<QFile> fileScores( new QFile(destDir.absoluteFilePath("scoresLibreMines")) );

        if(!fileScores->exists())
        {
            fileScores->open(QIODevice::NewOnly);
            fileScores->close();
        }

        bool saveScore = false;
        // Search for existing scores on the current level
        {
            fileScores->open(QIODevice::ReadOnly);

            QList<LibreMinesScore> scores;

            QDataStream stream(fileScores.get());
            stream.setVersion(QDataStream::Qt_5_12);

            while(!stream.atEnd())
            {
                LibreMinesScore s;
                stream >> s;

                if(s.gameDifficulty == score.gameDifficulty &&
                   s.length == score.length &&
                   s.width == score.width &&
                   s.mines == score.mines)
                {
                    scores.append(s);
                }
            }
            LibreMinesScore::sort(scores);

//            qDebug() << "\n##########################################";
//            for(const auto& s: scores)
//            {
//                qDebug() << "$$$$$$$$$$$$$$$$$$$$$$$$$$$";
//                qDebug() << s;
//                qDebug() << "$$$$$$$$$$$$$$$$$$$$$$$$$$$\n";
//            }
//            qDebug() << "##########################################";

            int index = 0;
            for(int i=0; i<scores.size(); ++i)
            {
                if(LibreMinesScore::bFirstIsBetter(scores.at(i), score))
                    index = i+1;
                else
                    break;
            }

            QString strGameDiffuclty;
            if(score.gameDifficulty == EASY)
                strGameDiffuclty = "Easy";
            else if(score.gameDifficulty == MEDIUM)
                strGameDiffuclty = "Medium";
            else if(score.gameDifficulty == HARD)
                strGameDiffuclty = "Hard";
            else if(score.gameDifficulty == CUSTOMIZED)
            {
                strGameDiffuclty = "Customized " + QString::number(score.width) +
                                   " x " + QString::number(score.length) + " : " +
                                   QString::number(score.mines);
            }

            // open the dialog
            LibreMinesScoresDialog dialog(this, scores.size() + 1);
            dialog.setWindowTitle(strGameDiffuclty);
            dialog.setWindowIcon(QIcon(":/icons_rsc/icons/libremines.svg"));

            dialog.setScores(scores, &score, index);
            int result = dialog.exec(); Q_UNUSED(result);

            if(dialog.bSaveEditableScore())
            {
                score.username = dialog.stringUserName();
                saveScore = true;
            }
        }

        if(saveScore)
        {
            qDebug() << "Saving score";

            fileScores.reset( new QFile(destDir.absoluteFilePath("scoresLibreMines")) );

            if(fileScores->exists())
                fileScores->open(QIODevice::Append);
            else
                fileScores->open(QIODevice::WriteOnly);

            QDataStream stream(fileScores.get());

            stream.setVersion(QDataStream::Qt_5_12);

            stream << score;
        }
    }

    QString QS_Statics =
            "Total time: " + QString::number(score.iTimeInNs*1e-9, 'f', 3) + " secs"
            +"\nCorrect Flags: " + QString::number(iCorrectFlags)
            +"\nWrongFlags: " + QString::number(iWrongFlags)
            +"\nUnlocked Cells: " + QString::number(iUnlockedCells)
            +"\n"
            +"\nFlags/s: " + QString::number(dFlagsPerSecond, 'f', 3)
            +"\nCells/s: " + QString::number(dCellsPerSecond, 'f', 3)
            +"\n"
            +"\nGame Complete: " + QString::number(score.dPercentageGameCompleted, 'f', 2) + " %";

    labelStatisLastMatch->setText(QS_Statics);
}

void LibreMinesGui::SLOT_currentTime(const ushort time)
{
    labelTimerInGame->setNum(time);
}

void LibreMinesGui::SLOT_minesLeft(const ushort minesLeft)
{
    lcd_numberMinesLeft->display(minesLeft);
}

void LibreMinesGui::SLOT_flagCell(const uchar _X, const uchar _Y)
{
    if(principalMatrix[_X][_Y].button->isHidden())
        qDebug(Q_FUNC_INFO);
    else
    {
        principalMatrix[_X][_Y].button->setIcon(QIcon(QPixmap::fromImage(*imgFlag).scaled(cellLength, cellLength, Qt::KeepAspectRatio)));
        principalMatrix[_X][_Y].button->setIconSize(QSize(cellLength, cellLength));
    }

    if(controller.active && controller.currentX == _X && controller.currentY == _Y)
    {
        vKeyboardControllerSetCurrentCell(controller.currentX, controller.currentY);
    }
}

void LibreMinesGui::SLOT_unflagCell(const uchar _X, const uchar _Y)
{
    if(principalMatrix[_X][_Y].button->isHidden())
        qDebug(Q_FUNC_INFO);
    else
    {
        principalMatrix[_X][_Y].button->setIcon(QIcon(QPixmap::fromImage(*imgNoFlag).scaled(cellLength, cellLength, Qt::KeepAspectRatio)));
        principalMatrix[_X][_Y].button->setIconSize(QSize(cellLength, cellLength));
    }

    if(controller.active && controller.currentX == _X && controller.currentY == _Y)
    {
        vKeyboardControllerSetCurrentCell(controller.currentX, controller.currentY);
    }
}

void LibreMinesGui::SLOT_remakeGame()
{
    vAttributeAllCells();
}

void LibreMinesGui::SLOT_gameWon()
{
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
            labelYouWonYouLost->setText("You Won\nDifficulty: CUSTOM\n" +
                                        QString::number(gameEngine->rows()) +
                                        "x" + QString::number(gameEngine->lines()) +
                                        " : " + QString::number(gameEngine->mines()) + " Mines");
    }

    for(uchar j=0; j<gameEngine->lines(); j++)
    {
        for (uchar i=0; i<gameEngine->rows(); i++)
        {
            principalMatrix[i][j].button->setEnabled(!principalMatrix[i][j].button->isHidden());
        }
    }

    if(controller.active)
    {
        controller.active = false;
        vKeyboardControllUnsetCurrentCell();
    }

}

void LibreMinesGui::SLOT_gameLost(const uchar _X, const uchar _Y)
{
    qDebug()<<"You Lost";

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
                                        QString::number(gameEngine->rows()) +
                                        "x" +
                                        QString::number(gameEngine->lines()) +
                                        " : " +
                                        QString::number(gameEngine->mines()) +
                                        " Mines");
    }
    principalMatrix[_X][_Y].label->setPixmap(QPixmap::fromImage(*imgBoom).scaled(cellLength, cellLength, Qt::KeepAspectRatio));


    for(uchar j=0; j<gameEngine->lines(); j++)
    {
        for (uchar i=0; i<gameEngine->rows(); i++)
        {
            CellGui& cellGui = principalMatrix[i][j];
            const LibreMinesGameEngine::CellGameEngine& cellGE = gameEngine->getPrincipalMatrix()[i][j];

            if(cellGE.isHidden)
            {
                if(cellGE.state == MINE &&
                   !cellGE.hasFlag)
                {
                    cellGui.button->hide();
                }
                else if (cellGE.state != MINE &&
                         cellGE.hasFlag)
                {
                    cellGui.button->hide();
                    cellGui.label->setPixmap(QPixmap::fromImage(*imgWrongFlag).scaled(cellLength, cellLength, Qt::KeepAspectRatio));
                }
                else
                {
                    cellGui.button->setEnabled(false);
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

void LibreMinesGui::SLOT_optionChanged(const QString &name, const QString &value)
{
    if(name.compare("Theme", Qt::CaseInsensitive) == 0)
    {
        vConfigureTheme(value);
    }
}

void LibreMinesGui::vConfigureTheme(const QString& theme)
{
    if(theme.compare("Dark", Qt::CaseInsensitive) == 0)
    {
        imgZero->load(":/images_rsc/Images/Minesweeper_zero_dark.png");
        imgOne->load(":/images_rsc/Images/Minesweeper_one_dark.png");
        imgTwo->load(":/images_rsc/Images/Minesweeper_two_dark.png");
        imgThree->load(":/images_rsc/Images/Minesweeper_three_dark.png");
        imgFour->load(":/images_rsc/Images/Minesweeper_four_dark.png");
        imgFive->load(":/images_rsc/Images/Minesweeper_five_dark.png");
        imgSix->load(":/images_rsc/Images/Minesweeper_six_dark.png");
        imgSeven->load(":/images_rsc/Images/Minesweeper_seven_dark.png");
        imgEight->load(":/images_rsc/Images/Minesweeper_eight_dark.png");
        imgFlag->load(":/images_rsc/Images/Minesweeper_flag_dark.png");
        imgNoFlag->load(":/images_rsc/Images/Minesweeper_no_flag_dark.png");
        imgMine->load(":/images_rsc/Images/Minesweeper_mine_dark.png");
        imgBoom->load(":/images_rsc/Images/Minesweeper_boom_dark.png");
        imgWrongFlag->load(":/images_rsc/Images/Minesweeper_wrong_flag_dark.png");

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
    else if(theme.compare("Light", Qt::CaseInsensitive) == 0)
    {
        imgZero->load(":/images_rsc/Images/Minesweeper_zero_light.png");
        imgOne->load(":/images_rsc/Images/Minesweeper_one_light.png");
        imgTwo->load(":/images_rsc/Images/Minesweeper_two_light.png");
        imgThree->load(":/images_rsc/Images/Minesweeper_three_light.png");
        imgFour->load(":/images_rsc/Images/Minesweeper_four_light.png");
        imgFive->load(":/images_rsc/Images/Minesweeper_five_light.png");
        imgSix->load(":/images_rsc/Images/Minesweeper_six_light.png");
        imgSeven->load(":/images_rsc/Images/Minesweeper_seven_light.png");
        imgEight->load(":/images_rsc/Images/Minesweeper_eight_light.png");
        imgFlag->load(":/images_rsc/Images/Minesweeper_flag_light.png");
        imgNoFlag->load(":/images_rsc/Images/Minesweeper_no_flag_light.png");
        imgMine->load(":/images_rsc/Images/Minesweeper_mine_light.png");
        imgBoom->load(":/images_rsc/Images/Minesweeper_boom_light.png");
        imgWrongFlag->load(":/images_rsc/Images/Minesweeper_wrong_flag_light.png");

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

void LibreMinesGui::vKeyboardControllerSetCurrentCell(const uchar x, const uchar y)
{
    controller.currentX = x;
    controller.currentY = y;

    const LibreMinesGameEngine::CellGameEngine& cellGE = gameEngine->getPrincipalMatrix()[controller.currentX][controller.currentY];
    CellGui& cellGui= principalMatrix[controller.currentX][controller.currentY];

    if(cellGE.isHidden)
    {

        QImage img = cellGE.hasFlag ? *imgFlag : *imgNoFlag;
        img.invertPixels();
        cellGui.button->setIcon(QIcon(QPixmap::fromImage(img).scaled(cellLength, cellLength, Qt::KeepAspectRatio)));
    }
    else
    {
        QImage img = QImage();

        if(cellGE.state == ZERO)
            img = *imgZero;
        else if(cellGE.state == ONE)
            img = *imgOne;
        else if(cellGE.state == TWO)
            img = *imgTwo;
        else if(cellGE.state == THREE)
            img = *imgThree;
        else if(cellGE.state == FOUR)
            img = *imgFour;
        else if(cellGE.state == FIVE)
            img = *imgFive;
        else if(cellGE.state == SIX)
            img = *imgSix;
        else if(cellGE.state == SEVEN)
            img = *imgSeven;
        else if(cellGE.state == EIGHT)
            img = *imgEight;
        else
            qFatal(Q_FUNC_INFO);

        img.invertPixels();

        cellGui.label->setPixmap(QPixmap::fromImage(img).scaled(cellLength, cellLength, Qt::KeepAspectRatio));
    }
}

void LibreMinesGui::vKeyboardControllUnsetCurrentCell()
{
    const LibreMinesGameEngine::CellGameEngine& cellGE = gameEngine->getPrincipalMatrix()[controller.currentX][controller.currentY];
    CellGui& cellGui= principalMatrix[controller.currentX][controller.currentY];

    if(cellGE.isHidden)
    {
        QImage* img = cellGE.hasFlag ? imgFlag : imgNoFlag;
        cellGui.button->setIcon(QIcon(QPixmap::fromImage(*img).scaled(cellLength, cellLength, Qt::KeepAspectRatio)));
    }
    else
    {
        QImage* img = nullptr;

        if(cellGE.state == ZERO)
            img = imgZero;
        else if(cellGE.state == ONE)
            img = imgOne;
        else if(cellGE.state == TWO)
            img = imgTwo;
        else if(cellGE.state == THREE)
            img = imgThree;
        else if(cellGE.state == FOUR)
            img = imgFour;
        else if(cellGE.state == FIVE)
            img = imgFive;
        else if(cellGE.state == SIX)
            img = imgSix;
        else if(cellGE.state == SEVEN)
            img = imgSeven;
        else if(cellGE.state == EIGHT)
            img = imgEight;
        else
            qFatal(Q_FUNC_INFO);

        cellGui.label->setPixmap(QPixmap::fromImage(*img).scaled(cellLength, cellLength, Qt::KeepAspectRatio));
    }
}

void LibreMinesGui::vKeyboardControllerMoveLeft()
{
    vKeyboardControllUnsetCurrentCell();

    uchar destX = (controller.ctrlPressed) ? 0 : (controller.currentX == 0) ? gameEngine->rows() - 1 : (controller.currentX - 1);

    vKeyboardControllerSetCurrentCell(destX, controller.currentY);
}

void LibreMinesGui::vKeyboardControllerMoveRight()
{
    vKeyboardControllUnsetCurrentCell();

    uchar destX = (controller.ctrlPressed) ? gameEngine->rows() - 1 : (controller.currentX == gameEngine->rows() - 1) ? 0 : (controller.currentX + 1);

    vKeyboardControllerSetCurrentCell(destX, controller.currentY);
}

void LibreMinesGui::vKeyboardControllerMoveDown()
{
    vKeyboardControllUnsetCurrentCell();

    uchar destY = (controller.ctrlPressed) ? gameEngine->lines() - 1 : (controller.currentY == gameEngine->lines() - 1) ? 0 : (controller.currentY + 1);

    vKeyboardControllerSetCurrentCell(controller.currentX, destY);
}

void LibreMinesGui::vKeyboardControllerMoveUp()
{
    vKeyboardControllUnsetCurrentCell();

    uchar destY = (controller.ctrlPressed) ? 0 : (controller.currentY == 0) ? gameEngine->lines() -1 : (controller.currentY - 1);

    vKeyboardControllerSetCurrentCell(controller.currentX, destY);
}

void LibreMinesGui::vLastSessionLoadConfigurationFile()
{
    QDir destDir = QDir::home();

    destDir.setFilter(QDir::AllDirs);

    if(!destDir.cd(".local"))
    {
        Q_ASSERT(destDir.mkdir(".local"));
        Q_ASSERT(destDir.cd(".local"));
    }
    if(!destDir.cd("share"))
    {
        Q_ASSERT(destDir.mkdir("share"));
        Q_ASSERT(destDir.cd("share"));
    }
    if(!destDir.cd("libremines"))
    {
        Q_ASSERT(destDir.mkdir("libremines"));
        Q_ASSERT(destDir.cd("libremines"));
    }

    QScopedPointer<QFile> fileScores( new QFile(destDir.absoluteFilePath("libreminesLastSession.txt")) );

    if(fileScores->open(QIODevice::ReadOnly))
    {
        QTextStream stream(fileScores.get());

        while(!stream.atEnd())
        {
            QString s = stream.readLine();

            if(s.at(0) == '#')
                continue;

           QStringList terms = s.split(" ");

           if(terms.size() < 2)
               continue;

           if(terms.at(0).compare("FirstCellClean", Qt::CaseInsensitive) == 0)
           {
               if(terms.size() != 2)
                   continue;

               preferences->setOptionFirstCellClean(terms.at(1));
           }
           else if(terms.at(0).compare("Theme", Qt::CaseInsensitive) == 0)
           {
               if(terms.size() != 2)
                   continue;

               preferences->setOptionTheme(terms.at(1));
           }
           else if(terms.at(0).compare("ClearNeighborCellsWhenClickedOnShowedCell", Qt::CaseInsensitive) == 0)
           {
               if(terms.size() != 2)
                   continue;

               preferences->setOptionCleanNeighborCellsWhenClickedOnShowedCell(terms.at(1));
           }
           else if(terms.at(0).compare("Username", Qt::CaseInsensitive) == 0)
           {
               if(terms.size() != 2)
                   continue;

               preferences->setOptionUsername(terms.at(1));
           }
           else if(terms.at(0).compare("CustomizedPercentageOfMines", Qt::CaseInsensitive) == 0)
           {
               if(terms.size() != 2)
                   continue;

               sbCustomizednMines->setValue(terms.at(1).toInt());
           }
           else if(terms.at(0).compare("CustomizedX", Qt::CaseInsensitive) == 0)
           {
               if(terms.size() != 2)
                   continue;

               sbCustomizedX->setValue(terms.at(1).toInt());
           }
           else if(terms.at(0).compare("CustomizedY", Qt::CaseInsensitive) == 0)
           {
               if(terms.size() != 2)
                   continue;

               sbCustomizedY->setValue(terms.at(1).toInt());
           }
        }
    }
}

void LibreMinesGui::vLastSessionSaveConfigurationFile()
{
    QDir destDir = QDir::home();

    destDir.setFilter(QDir::AllDirs);

    if(!destDir.cd(".local"))
    {
        Q_ASSERT(destDir.mkdir(".local"));
        Q_ASSERT(destDir.cd(".local"));
    }
    if(!destDir.cd("share"))
    {
        Q_ASSERT(destDir.mkdir("share"));
        Q_ASSERT(destDir.cd("share"));
    }
    if(!destDir.cd("libremines"))
    {
        Q_ASSERT(destDir.mkdir("libremines"));
        Q_ASSERT(destDir.cd("libremines"));
    }

    QScopedPointer<QFile> fileScores( new QFile(destDir.absoluteFilePath("libreminesLastSession.txt")) );

    fileScores->open(QIODevice::WriteOnly);

    QTextStream stream(fileScores.get());

    stream << "FirstCellClean" << ' ' << (preferences->optionFirstCellClean() ? "On" : "Off") << '\n'
           << "Theme" << ' ' << preferences->optionTheme() << '\n'
           << "ClearNeighborCellsWhenClickedOnShowedCell" << ' ' << (preferences->optionCleanNeighborCellsWhenClickedOnShowedCell() ? "On" : "Off") << '\n'
           << "Username" << ' ' << preferences->optionUsername() << '\n'
           << "CustomizedPercentageOfMines" << ' ' << sbCustomizednMines->value() << '\n'
           << "CustomizedX" << ' ' << sbCustomizedX->value() << '\n'
           << "CustomizedY" << ' ' << sbCustomizedY->value() << '\n';
}
