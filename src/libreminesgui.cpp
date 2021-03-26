/*****************************************************************************
 * LibreMines                                                                *
 * Copyright (C) 2020-2021  Bruno Bollos Correa                              *
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
#include <QShortcut>
#include <QMessageBox>

#include "libreminesgui.h"
#include "libreminesscoresdialog.h"
#include "libreminesconfig.h"
#include "libreminesviewscores.h"

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
    preferences( new LibreMinesPreferencesDialog(this) )
{   
    connect(preferences, &LibreMinesPreferencesDialog::SIGNAL_optionChanged,
            this, &LibreMinesGui::SLOT_optionChanged);

    // Unable central widget when the preferences dialog is active
    // Also update the preferences ehrn finished
    connect(preferences, &LibreMinesPreferencesDialog::SIGNAL_visibilityChanged,
            [this](const bool visible)
    {
        this->centralWidget()->setEnabled(!visible);
        this->vUpdatePreferences();
    });

    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this), &QShortcut::activated,
            this, &LibreMinesGui::SLOT_quitApplication);

    // Create interface with the passed dimensions
    vCreateGUI(thatWidth, thatHeight);

    qApp->installEventFilter(this);

//    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowIcon(QIcon(":/icons_rsc/icons/libremines.svg"));
    this->setWindowTitle("LibreMines");

    // Initializr keyboard controller attributes
    controller.ctrlPressed = false;
    controller.active = false;
    controller.currentX = 0;
    controller.currentY = 0;

    // Load configuration file and set the theme
    vLastSessionLoadConfigurationFile();
    vSetApplicationTheme(preferences->optionApplicationTheme());
    vSetMinefieldTheme(preferences->optionMinefieldTheme());
}

LibreMinesGui::~LibreMinesGui()
{
    vResetPrincipalMatrix();

    vLastSessionSaveConfigurationFile();
}

bool LibreMinesGui::eventFilter(QObject* object, QEvent* event)
{
    Q_UNUSED(object)

    // If the game is not running, do not deal woth the event
    if(!gameEngine || !gameEngine->isGameActive())
        return false;

    // If the GUI is not ready
    if(principalMatrix.empty() ||
       !principalMatrix[0][0].button ||
       !principalMatrix[0][0].button->isEnabled())
        return false;

    // If the keyboard controller has invalid settings
    if(!controller.valid)
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

            if(key == Qt::Key_Control)
            {
                controller.ctrlPressed = false;
                return true;
            }

            // Active the controller depending on the key
            if(!controller.active)
            {
                if(key == controller.keyLeft ||
                   key == controller.keyUp ||
                   key == controller.keyDown ||
                   key == controller.keyRight)
                {
                    controller.active = true;
                    vKeyboardControllerSetCurrentCell(0, 0);
                    return true;
                }
            }
            else
            {
                if(key == controller.keyLeft)
                {
                    vKeyboardControllerMoveLeft();
                    return true;
                }
                if(key == controller.keyUp)
                {
                    vKeyboardControllerMoveUp();
                    return true;
                }
                if(key == controller.keyDown)
                {
                    vKeyboardControllerMoveDown();
                    return true;
                }
                if(key == controller.keyRight)
                {
                    vKeyboardControllerMoveRight();
                    return true;
                }
                if(key == controller.keyReleaseCell)
                {
                    const LibreMinesGameEngine::CellGameEngine& cell =
                            gameEngine->getPrincipalMatrix()[controller.currentX][controller.currentY];

                    if(cell.isHidden)
                    {
                        Q_EMIT SIGNAL_cleanCell(controller.currentX, controller.currentY);
                    }
                    else if(preferences->optionCleanNeighborCellsWhenClickedOnShowedCell())
                    {
                        Q_EMIT SIGNAL_cleanNeighborCells(controller.currentX, controller.currentY);
                    }
                    return true;

                }
                if(key == controller.keyFlagCell)
                {
                    Q_EMIT SIGNAL_addOrRemoveFlag(controller.currentX, controller.currentY);
                    return true;
                }
                if(key == Qt::Key_Escape)
                {
                    controller.active = false;
                    vKeyboardControllUnsetCurrentCell();
                    return true;
                }
            }

        }break;

        default:
            break;
    }

    return false;
}

void LibreMinesGui::closeEvent(QCloseEvent *e)
{
    SLOT_quitApplication();
    e->ignore();
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

    if(!controller.valid)
    {
        QMessageBox::warning(this, "Keyboard Controller is invalid",
                             "Dear user, unfortunately your Keyboard Controller preferences"
                             " are invalid. Therefore you will not be able to play with the keyboard.\n"
                             "To fix it go to (Main Meun > Options > Preferences) and edit your preferences.");
    }

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

    // Update the pixmaps
    vSetMinefieldTheme(preferences->optionMinefieldTheme());

    const bool bCleanNeighborCellsWhenClickedOnShowedLabel = preferences->optionCleanNeighborCellsWhenClickedOnShowedCell();

    widgetBoardContents->show();
    scrollAreaBoard->show();
    scrollAreaBoard->setGeometry(0, 0, iLimitWidthField, iLimitHeightField);
    widgetBoardContents->setGeometry(0, 0, _X*cellLength, _Y*cellLength);

    // Create each cell
    for(uchar j=0; j<_Y; j++)
    {
        for (uchar i=0; i<_X; i++)
        {
            CellGui& cell = principalMatrix[i][j];

//            cell.label = new QLabel_adapted(this);
//            cell.button = new QPushButton_adapted(this);

            cell.label = new QLabel_adapted(widgetBoardContents);
            cell.button = new QPushButton_adapted(widgetBoardContents);

            layoutBoard->addWidget(cell.label, j, i);
            layoutBoard->addWidget(cell.button, j, i);

//            cell.label->setGeometry(i*cellLength, j*cellLength, cellLength, cellLength);
            cell.label->setPixmap(*pmZero);
            cell.label->show();

//            cell.button->setGeometry(i*cellLength, j*cellLength, cellLength, cellLength);
            cell.button->setIcon(QIcon(*pmNoFlag));
            cell.button->setIconSize(QSize(cellLength, cellLength));
            cell.button->show();
            cell.button->setEnabled(false);

            connect(cell.button, &QPushButton_adapted::SIGNAL_released,
                    this, &LibreMinesGui::SLOT_OnCellButtonReleased);

            if(bCleanNeighborCellsWhenClickedOnShowedLabel)
            {
                connect(cell.label, &QLabel_adapted::SIGNAL_released,
                        this, &LibreMinesGui::SLOT_onCellLabelReleased);
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

    // Set the initial value of mines left to the total number
    //  of mines
    SLOT_minesLeft(gameEngine->mines());
}

void LibreMinesGui::vAttributeAllCells()
{
    for(uchar j=0; j<gameEngine->lines(); ++j)
    {
        for(uchar i=0; i<gameEngine->rows(); ++i)
        {
            CellGui& cell = principalMatrix[i][j];

            cell.button->setEnabled(true);

            switch(gameEngine->getPrincipalMatrix()[i][j].state)
            {

                case ZERO:
                    cell.label->setPixmap(*pmZero);
                    break;
                case ONE:
                    cell.label->setPixmap(*pmOne);
                    break;
                case TWO:
                    cell.label->setPixmap(*pmTwo);
                    break;
                case THREE:
                    cell.label->setPixmap(*pmThree);
                    break;
                case FOUR:
                    cell.label->setPixmap(*pmFour);
                    break;
                case FIVE:
                    cell.label->setPixmap(*pmFive);
                    break;
                case SIX:
                    cell.label->setPixmap(*pmSix);
                    break;
                case SEVEN:
                    cell.label->setPixmap(*pmSeven);
                    break;
                case EIGHT:
                    cell.label->setPixmap(*pmEight);
                    break;
                case MINE:
                    cell.label->setPixmap(*pmMine);
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

void LibreMinesGui::vCreateGUI(int width, int height)
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
    actionHighScores = new QAction(this);
    actionAbout = new QAction(this);
    actionAboutQt = new QAction(this);

    QMenuBar* menuBarGlobal = new QMenuBar(this);

    menuBarGlobal->setGeometry(0, 0, iWidthMainWindow, 100);

    menuOptions = new QMenu(menuBarGlobal);
    menuHelp = new QMenu(menuBarGlobal);

    this->setMenuBar(menuBarGlobal);
    this->setStatusBar(new QStatusBar(this));

    menuBarGlobal->addAction(menuOptions->menuAction());
    menuBarGlobal->addAction(menuHelp->menuAction());

    menuOptions->addActions({actionPreferences, actionHighScores});
    menuHelp->addActions({actionAbout, actionAboutQt});

    menuOptions->setTitle("Options");
    menuHelp->setTitle("Help");

    actionPreferences->setText("Preferences...");
    actionHighScores->setText("High Scores...");
    actionAbout->setText("About...");
    actionAboutQt->setText("About Qt...");

    this->setFont(QFont("Liberation Sans"));
    labelTimerInGame = new QLabel(centralWidget());
    lcd_numberMinesLeft = new QLCDNumber(centralWidget());
    buttonRestartInGame = new QPushButton(centralWidget());
    buttonQuitInGame = new QPushButton(centralWidget());
    labelYouWonYouLost = new QLabel(centralWidget());
    labelStatisLastMatch = new QLabel(centralWidget());

    scrollAreaBoard = new QScrollArea(centralWidget());
    widgetBoardContents = new QWidget();

    layoutBoard = new QGridLayout();
    layoutBoard->setSpacing(0);

    widgetBoardContents->setLayout(layoutBoard);
    scrollAreaBoard->setWidget(widgetBoardContents);

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


    sbCustomizedPercentageMines = new QSpinBox(centralWidget());
    sbCustomizedPercentageMines->setMinimum(0);
    sbCustomizedPercentageMines->setMaximum(100);
    sbCustomizedPercentageMines->setValue(20);

    sbCustomizedNumbersOfMines = new QSpinBox(centralWidget());
    sbCustomizedNumbersOfMines->setMinimum(0);
    sbCustomizedNumbersOfMines->setMaximum(sbCustomizedX->value() * sbCustomizedY->value() * sbCustomizedPercentageMines->value() / 100);
    sbCustomizedNumbersOfMines->setValue(sbCustomizedNumbersOfMines->maximum() * sbCustomizedPercentageMines->value() / 100);

    cbCustomizedMinesInPercentage = new QCheckBox(centralWidget());

    labelCustomizedX = new QLabel(centralWidget());
    labelCustomizedX->setText("Width: ");

    labelCustomizedY = new QLabel(centralWidget());
    labelCustomizedY->setText("Height: ");

    labelCustomizedPercentageMines = new QLabel(centralWidget());
    labelCustomizedPercentageMines->setText("Percent of Mines: ");

    labelCustomizedNumbersOfMines = new QLabel(centralWidget());
    labelCustomizedNumbersOfMines->setText("Number of Mines: ");

    buttonEasy->setGeometry(iWidthMainWindow/20, iHeightMainWindow/20,
                            iWidthMainWindow/2 - 2*iWidthMainWindow/20, 4*(iHeightMainWindow/2 - 2*iHeightMainWindow/20)/5 );

    buttonMedium->setGeometry(buttonEasy->x() + buttonEasy->width() + iWidthMainWindow/20, iHeightMainWindow/20,
                              iWidthMainWindow/2 - 2*iWidthMainWindow/20, 4*(iHeightMainWindow/2 - 2*iHeightMainWindow/20)/5 );

    buttonHard->setGeometry(iWidthMainWindow/20, buttonEasy->y() + buttonEasy->height() + iHeightMainWindow/20,
                            iWidthMainWindow/2 - 2*iWidthMainWindow/20,  4*(iHeightMainWindow/2 - 2*iHeightMainWindow/20)/5 );

    buttonCustomizedNewGame->setGeometry(buttonEasy->x() + buttonEasy->width() + iWidthMainWindow/20, buttonEasy->y() + buttonEasy->height() + iHeightMainWindow/20,
                                         iWidthMainWindow/2 - 2*iWidthMainWindow/20, 2*(iHeightMainWindow/2 - 2*iHeightMainWindow/20)/5 );

    labelCustomizedPercentageMines->setGeometry(buttonCustomizedNewGame->x(), buttonCustomizedNewGame->y()+buttonCustomizedNewGame->height(),
                                        buttonCustomizedNewGame->width()/2,2*(iHeightMainWindow/2 - 2*iHeightMainWindow/20)/(3*5));

    labelCustomizedNumbersOfMines->setGeometry(labelCustomizedPercentageMines->geometry());

    labelCustomizedX->setGeometry(labelCustomizedPercentageMines->x(), labelCustomizedPercentageMines->y()+labelCustomizedPercentageMines->height(),
                                  labelCustomizedPercentageMines->width(),labelCustomizedPercentageMines->height());

    labelCustomizedY->setGeometry(labelCustomizedX->x(), labelCustomizedX->y()+labelCustomizedX->height(),
                                  labelCustomizedX->width(),labelCustomizedX->height());

    sbCustomizedPercentageMines->setGeometry(labelCustomizedPercentageMines->x()+labelCustomizedPercentageMines->width(), labelCustomizedPercentageMines->y(),
                                    9*labelCustomizedPercentageMines->width()/10, labelCustomizedPercentageMines->height());

    sbCustomizedNumbersOfMines->setGeometry(sbCustomizedPercentageMines->geometry());

    cbCustomizedMinesInPercentage->setGeometry(sbCustomizedNumbersOfMines->x() + sbCustomizedNumbersOfMines->width(), sbCustomizedNumbersOfMines->y(),
                                               labelCustomizedNumbersOfMines->width()/10, sbCustomizedNumbersOfMines->height());

    sbCustomizedX->setGeometry(labelCustomizedX->x()+labelCustomizedX->width(), labelCustomizedX->y(),
                               labelCustomizedX->width(), labelCustomizedX->height());

    sbCustomizedY->setGeometry(labelCustomizedY->x()+labelCustomizedY->width(), labelCustomizedY->y(),
                               labelCustomizedY->width(), labelCustomizedY->height());

    labelCustomizedNumbersOfMines->hide();
    sbCustomizedNumbersOfMines->hide();

    cbCustomizedMinesInPercentage->setStyleSheet(
                "QCheckBox::indicator "
                "{"
                "    width: " + QString::number(cbCustomizedMinesInPercentage->width()) + "px;"
                "    height: " + QString::number(cbCustomizedMinesInPercentage->width()) + "px;"
                "}");

    cbCustomizedMinesInPercentage->setChecked(true);

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

    connect(actionAbout, &QAction::triggered,
            this, &LibreMinesGui::SLOT_showAboutDialog);

    connect(actionAboutQt, &QAction::triggered,
            [this](){ QMessageBox::aboutQt(this, "LibreMines"); });

    connect(actionHighScores, &QAction::triggered,
            this, &LibreMinesGui::SLOT_showHighScores);

    connect(cbCustomizedMinesInPercentage, &QCheckBox::stateChanged,
            [this](const int state)
    {
       if(state == Qt::Checked)
       {
           sbCustomizedPercentageMines->show();
           labelCustomizedPercentageMines->show();

           sbCustomizedNumbersOfMines->hide();
           labelCustomizedNumbersOfMines->hide();
       }
       else
       {
           sbCustomizedPercentageMines->hide();
           labelCustomizedPercentageMines->hide();

           sbCustomizedNumbersOfMines->show();
           labelCustomizedNumbersOfMines->show();
       }
    });

    connect(sbCustomizedPercentageMines, QOverload<int>::of(&QSpinBox::valueChanged),
            [this](const int value)
    {
        if(!sbCustomizedPercentageMines->isHidden())
            sbCustomizedNumbersOfMines->setValue(sbCustomizedX->value() * sbCustomizedY->value() * value / 100);
    });

    connect(sbCustomizedNumbersOfMines, QOverload<int>::of(&QSpinBox::valueChanged),
            [this](const int value)
    {
        if(!sbCustomizedNumbersOfMines->isHidden())
            sbCustomizedPercentageMines->setValue(100 * value / (sbCustomizedX->value() * sbCustomizedY->value()) );
    });

    auto updateCustomizedNumberOfMinesMaximum
    {
        [this]()
        {
            sbCustomizedNumbersOfMines->setMaximum(sbCustomizedX->value() * sbCustomizedY->value());
            sbCustomizedNumbersOfMines->setValue(sbCustomizedX->value() * sbCustomizedY->value() * sbCustomizedPercentageMines->value() / 100);
        }
    };

    connect(sbCustomizedX, QOverload<int>::of(&QSpinBox::valueChanged),
            updateCustomizedNumberOfMinesMaximum);
    connect(sbCustomizedY, QOverload<int>::of(&QSpinBox::valueChanged),
            updateCustomizedNumberOfMinesMaximum);
}

void LibreMinesGui::vHideMainMenu()
{
    buttonEasy->hide();
    buttonMedium->hide();
    buttonHard->hide();

    buttonCustomizedNewGame->hide();

    sbCustomizedX->hide();
    sbCustomizedY->hide();
    sbCustomizedPercentageMines->hide();
    sbCustomizedNumbersOfMines->hide();

    labelCustomizedX->hide();
    labelCustomizedY->hide();
    labelCustomizedPercentageMines->hide();
    labelCustomizedNumbersOfMines->hide();

    cbCustomizedMinesInPercentage->hide();

    // Hide de menu bar too
    menuBar()->hide();
}

void LibreMinesGui::vShowMainMenu()
{
    buttonEasy->show();
    buttonMedium->show();
    buttonHard->show();

    buttonCustomizedNewGame->show();

    sbCustomizedX->show();
    sbCustomizedY->show();

    labelCustomizedX->show();
    labelCustomizedY->show();

    if(cbCustomizedMinesInPercentage->isChecked())
    {
        sbCustomizedPercentageMines->show();
        labelCustomizedPercentageMines->show();
    }
    else
    {
        sbCustomizedNumbersOfMines->show();
        labelCustomizedNumbersOfMines->show();
    }

    cbCustomizedMinesInPercentage->show();

    menuBar()->show();
}

void LibreMinesGui::SLOT_Easy()
{
    vHideMainMenu();
    vNewGame(8, 8, 10);

    difficult = EASY;
}

void LibreMinesGui::SLOT_Medium()
{
    vHideMainMenu();
    vNewGame(16, 16, 40);

    difficult = MEDIUM;

}

void LibreMinesGui::SLOT_Hard()
{
    vHideMainMenu();
    vNewGame(30, 16, 99);

    difficult = HARD;

}

void LibreMinesGui::SLOT_Customized()
{
    vHideMainMenu();
    vNewGame(sbCustomizedX->value(), sbCustomizedY->value(), sbCustomizedNumbersOfMines->value());

    difficult = CUSTOMIZED;

}

void LibreMinesGui::vAjustInterfaceInGame()
{
    const int width = this->width();
    const int height = this->height();

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
    widgetBoardContents->hide();
    scrollAreaBoard->hide();
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
    if(gameEngine && gameEngine->isGameActive())
    {
        QMessageBox messageBox;

        messageBox.setText("There is a game happening.");
        messageBox.setInformativeText("Are you sure you want to quit?");
        messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        messageBox.setDefaultButton(QMessageBox::No);
        int result = messageBox.exec();

        if(result == QMessageBox::No)
            return;
    }

    vResetPrincipalMatrix();

    labelStatisLastMatch->setText(" ");
    labelYouWonYouLost->setText(" ");

    const uchar x = gameEngine->rows();
    const uchar y = gameEngine->lines();
    const ushort mines = gameEngine->mines();

    Q_EMIT SIGNAL_stopGame();
    vNewGame(x, y, mines);
}

void LibreMinesGui::SLOT_Quit()
{
    if(gameEngine && gameEngine->isGameActive())
    {
        QMessageBox messageBox;

        messageBox.setText("There is a game happening.");
        messageBox.setInformativeText("Are you sure you want to quit?");
        messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        messageBox.setDefaultButton(QMessageBox::No);
        int result = messageBox.exec();

        if(result == QMessageBox::No)
            return;
    }

    labelStatisLastMatch->setText(" ");

    Q_EMIT SIGNAL_stopGame();

    vResetPrincipalMatrix();
    vHideInterfaceInGame();
    vShowMainMenu();

    difficult = NONE;

    gameEngine.reset();
}

void LibreMinesGui::SLOT_OnCellButtonReleased(const QMouseEvent *const e)
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
                        Q_EMIT SIGNAL_addOrRemoveFlag(i, j);
                        return;

                    case Qt::LeftButton:
                        Q_EMIT SIGNAL_cleanCell(i, j);
                        return;

                    default:
                        return;
                }
            }
        }
    }
}

void LibreMinesGui::SLOT_onCellLabelReleased(const QMouseEvent *const e)
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
                        Q_EMIT SIGNAL_cleanNeighborCells(i, j);
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
                   s.heigth == score.heigth &&
                   s.width == score.width &&
                   s.mines == score.mines)
                {
                    scores.append(s);
                }
            }
            LibreMinesScore::sort(scores);

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
                                   " x " + QString::number(score.heigth) + " : " +
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
        principalMatrix[_X][_Y].button->setIcon(QIcon(*pmFlag));
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
        principalMatrix[_X][_Y].button->setIcon(QIcon(*pmNoFlag));
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
    principalMatrix[_X][_Y].label->setPixmap(*pmBoom);


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
                    cellGui.label->setPixmap(*pmWrongFlag);
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
    if(name.compare("ApplicationTheme", Qt::CaseInsensitive) == 0)
    {
        vSetApplicationTheme(value);
    }
    else if(name.compare("MinefieldTheme", Qt::CaseInsensitive) == 0)
    {
        vSetMinefieldTheme(value);
    }
}

void LibreMinesGui::SLOT_quitApplication()
{
    if(gameEngine && gameEngine->isGameActive())
    {
        QMessageBox messageBox;

        messageBox.setText("There is a game happening.");
        messageBox.setInformativeText("Are you sure you want to quit?");
        messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        messageBox.setDefaultButton(QMessageBox::No);
        int result = messageBox.exec();

        if(result == QMessageBox::No)
            return;
    }

    this->deleteLater();
}

void LibreMinesGui::SLOT_showAboutDialog()
{
    QString text =
            "LibreMines version " + QString(LIBREMINES_PROJECT_VERSION) + "\n"
            "\n"
            "LibreMines is a free/libre and open source"
            " Qt based Minesweeper game.\n"
            "\n"
            "Get the source code on <https://github.com/Bollos00/LibreMines>";

    QMessageBox::about(this, "LibreMines", text);
}

void LibreMinesGui::SLOT_showHighScores()
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

    fileScores->open(QIODevice::ReadOnly);

    QList<LibreMinesScore> scores;

    QDataStream stream(fileScores.get());
    stream.setVersion(QDataStream::Qt_5_12);

    while(!stream.atEnd())
    {
        LibreMinesScore s;
        stream >> s;

        scores.append(s);
    }

    // open the dialog
    LibreMinesViewScores dialog(this);
    dialog.setWindowIcon(QIcon(":/icons_rsc/icons/libremines.svg"));
    dialog.setScores(scores);
    dialog.exec();
}

void LibreMinesGui::vSetApplicationTheme(const QString& theme)
{
    if(theme.compare("Dark", Qt::CaseInsensitive) == 0)
    {
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
        lightPalette.setColor (QPalette::Highlight,       QColor (42, 130, 218));

        qApp->setPalette(lightPalette);
    }
}

void LibreMinesGui::vSetMinefieldTheme(const QString &theme)
{
    QString prefix;
    if(theme.compare("ClassicLight", Qt::CaseInsensitive) == 0)
    {
        prefix = ":/minefield_icons/classic_light/Minesweeper_";
    }
    else if(theme.compare("ClassicDark", Qt::CaseInsensitive) == 0)
    {
        prefix = ":/minefield_icons/classic_dark/Minesweeper_";
    }
    else
    {
        qWarning() << "Minefield selected " << theme << " does not exist";
        return;
    }

    pmZero.reset( new QPixmap( QPixmap(prefix + "0.svg").scaled(cellLength, cellLength, Qt::KeepAspectRatio)));
    pmOne.reset( new QPixmap(  QPixmap(prefix + "1.svg").scaled(cellLength, cellLength, Qt::KeepAspectRatio)));
    pmTwo.reset( new QPixmap(  QPixmap(prefix + "2.svg").scaled(cellLength, cellLength, Qt::KeepAspectRatio)));
    pmThree.reset( new QPixmap(QPixmap(prefix + "3.svg").scaled(cellLength, cellLength, Qt::KeepAspectRatio)));
    pmFour.reset( new QPixmap( QPixmap(prefix + "4.svg").scaled(cellLength, cellLength, Qt::KeepAspectRatio)));
    pmFive.reset( new QPixmap( QPixmap(prefix + "5.svg").scaled(cellLength, cellLength, Qt::KeepAspectRatio)));
    pmSix.reset( new QPixmap(  QPixmap(prefix + "6.svg").scaled(cellLength, cellLength, Qt::KeepAspectRatio)));
    pmSeven.reset( new QPixmap(QPixmap(prefix + "7.svg").scaled(cellLength, cellLength, Qt::KeepAspectRatio)));
    pmEight.reset( new QPixmap(QPixmap(prefix + "8.svg").scaled(cellLength, cellLength, Qt::KeepAspectRatio)));
    pmFlag.reset( new QPixmap(QPixmap(prefix + "flag.svg").scaled(cellLength, cellLength, Qt::KeepAspectRatio)));
    pmNoFlag.reset( new QPixmap(QPixmap(prefix + "no_flag.svg").scaled(cellLength, cellLength, Qt::KeepAspectRatio)));
    pmMine.reset( new QPixmap(QPixmap(prefix + "mine.svg").scaled(cellLength, cellLength, Qt::KeepAspectRatio)));
    pmBoom.reset( new QPixmap(QPixmap(prefix + "boom.svg").scaled(cellLength, cellLength, Qt::KeepAspectRatio)));
    pmWrongFlag.reset( new QPixmap(QPixmap(prefix + "wrong_flag.svg").scaled(cellLength, cellLength, Qt::KeepAspectRatio)));
}

void LibreMinesGui::vKeyboardControllerSetCurrentCell(const uchar x, const uchar y)
{
    controller.currentX = x;
    controller.currentY = y;

    const LibreMinesGameEngine::CellGameEngine& cellGE = gameEngine->getPrincipalMatrix()[controller.currentX][controller.currentY];
    CellGui& cellGui= principalMatrix[controller.currentX][controller.currentY];

    if(cellGE.isHidden)
    {

        QImage img = cellGE.hasFlag ? pmFlag->toImage() : pmNoFlag->toImage();
        img.invertPixels();
        cellGui.button->setIcon(QIcon(QPixmap::fromImage(img)));
    }
    else
    {
        const QPixmap* pm = nullptr;

        if(cellGE.state == ZERO)
            pm = pmZero.get();
        else if(cellGE.state == ONE)
            pm = pmOne.get();
        else if(cellGE.state == TWO)
            pm = pmTwo.get();
        else if(cellGE.state == THREE)
            pm = pmThree.get();
        else if(cellGE.state == FOUR)
            pm = pmFour.get();
        else if(cellGE.state == FIVE)
            pm = pmFive.get();
        else if(cellGE.state == SIX)
            pm = pmSix.get();
        else if(cellGE.state == SEVEN)
            pm = pmSeven.get();
        else if(cellGE.state == EIGHT)
            pm = pmEight.get();
        else
            qFatal(Q_FUNC_INFO);

        QImage img = pm->toImage();
        img.invertPixels();

        cellGui.label->setPixmap(QPixmap::fromImage(img));
    }
}

void LibreMinesGui::vKeyboardControllUnsetCurrentCell()
{
    const LibreMinesGameEngine::CellGameEngine& cellGE = gameEngine->getPrincipalMatrix()[controller.currentX][controller.currentY];
    CellGui& cellGui= principalMatrix[controller.currentX][controller.currentY];

    if(cellGE.isHidden)
    {
        QPixmap* pm = cellGE.hasFlag ? pmFlag.get() : pmNoFlag.get();
        cellGui.button->setIcon(QIcon(*pm));
    }
    else
    {
        QPixmap* pm = nullptr;

        if(cellGE.state == ZERO)
            pm = pmZero.get();
        else if(cellGE.state == ONE)
            pm = pmOne.get();
        else if(cellGE.state == TWO)
            pm = pmTwo.get();
        else if(cellGE.state == THREE)
            pm = pmThree.get();
        else if(cellGE.state == FOUR)
            pm = pmFour.get();
        else if(cellGE.state == FIVE)
            pm = pmFive.get();
        else if(cellGE.state == SIX)
            pm = pmSix.get();
        else if(cellGE.state == SEVEN)
            pm = pmSeven.get();
        else if(cellGE.state == EIGHT)
            pm = pmEight.get();
        else
            qFatal(Q_FUNC_INFO);

        cellGui.label->setPixmap(*pm);
    }
}

void LibreMinesGui::vKeyboardControllerMoveLeft()
{   
    vKeyboardControllUnsetCurrentCell();

    uchar destX = 0;

    if(controller.ctrlPressed)
    {
        if(preferences->optionWhenCtrlIsPressed() == LibreMines::Jump3Cells &&
           controller.currentX >= 3)
        {
            destX = controller.currentX - 3;
        }
        else if(preferences->optionWhenCtrlIsPressed() == LibreMines::Jump5Cells &&
            controller.currentX >= 5)
        {
            destX = controller.currentX - 5;
        }
        else if(preferences->optionWhenCtrlIsPressed() == LibreMines::Jump10Cells &&
                controller.currentX >= 10)
        {
            destX = controller.currentX - 10;
        }
//        else
//        {
//            destX = 0;
//        }
    }
    else
    {
        destX = (controller.currentX == 0) ? gameEngine->rows() - 1 : controller.currentX - 1;
    }

    vKeyboardControllerSetCurrentCell(destX, controller.currentY);
}

void LibreMinesGui::vKeyboardControllerMoveRight()
{
    vKeyboardControllUnsetCurrentCell();

    uchar destX = gameEngine->rows() - 1;

    if(controller.ctrlPressed)
    {
        if(preferences->optionWhenCtrlIsPressed() == LibreMines::Jump3Cells &&
            controller.currentX < gameEngine->rows() - 3)
        {
            destX = controller.currentX + 3;
        }
        else if(preferences->optionWhenCtrlIsPressed() == LibreMines::Jump5Cells &&
            controller.currentX < gameEngine->rows() - 5)
        {
            destX = controller.currentX + 5;
        }
        else if(preferences->optionWhenCtrlIsPressed() == LibreMines::Jump10Cells &&
                controller.currentX < gameEngine->rows() - 10)
        {
            destX = controller.currentX + 10;
        }
    }
    else
    {
        destX = (controller.currentX == gameEngine->rows() - 1) ? 0 : (controller.currentX + 1);
    }

    vKeyboardControllerSetCurrentCell(destX, controller.currentY);

}

void LibreMinesGui::vKeyboardControllerMoveDown()
{
    vKeyboardControllUnsetCurrentCell();

    uchar destY = gameEngine->lines() - 1;

    if(controller.ctrlPressed)
    {
        if(preferences->optionWhenCtrlIsPressed() == LibreMines::Jump3Cells &&
            controller.currentY < gameEngine->lines() - 3)
        {
            destY = controller.currentY + 3;
        }
        else if(preferences->optionWhenCtrlIsPressed() == LibreMines::Jump5Cells &&
            controller.currentY < gameEngine->lines() - 5)
        {
            destY = controller.currentY + 5;
        }
        else if(preferences->optionWhenCtrlIsPressed() == LibreMines::Jump10Cells &&
                controller.currentY < gameEngine->lines() - 10)
        {
            destY = controller.currentY + 10;
        }
    }
    else
    {
        destY = (controller.currentY == gameEngine->lines() - 1) ? 0 : (controller.currentY + 1);
    }

    vKeyboardControllerSetCurrentCell(controller.currentX, destY);

}

void LibreMinesGui::vKeyboardControllerMoveUp()
{
    vKeyboardControllUnsetCurrentCell();

    uchar destY = 0;

    if(controller.ctrlPressed)
    {
        if(preferences->optionWhenCtrlIsPressed() == LibreMines::Jump3Cells &&
            controller.currentY >= 3)
        {
            destY = controller.currentY - 3;
        }
        else if(preferences->optionWhenCtrlIsPressed() == LibreMines::Jump5Cells &&
            controller.currentY >= 5)
        {
            destY = controller.currentY - 5;
        }
        else if(preferences->optionWhenCtrlIsPressed() == LibreMines::Jump10Cells &&
                controller.currentY >= 10)
        {
            destY = controller.currentY - 10;
        }
    }
    else
    {
        destY = (controller.currentY == 0) ? gameEngine->lines() - 1 : controller.currentY - 1;
    }

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
           else if(terms.at(0).compare("ApplicationTheme", Qt::CaseInsensitive) == 0)
           {
               if(terms.size() != 2)
                   continue;

               preferences->setOptionApplicationTheme(terms.at(1));
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

               sbCustomizedPercentageMines->setValue(terms.at(1).toInt());
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
           else if(terms.at(0).compare("KeyboardControllerKeys", Qt::CaseInsensitive) == 0)
           {
               if(terms.size() != 7)
                   continue;

               preferences->setOptionKeyboardControllerKeys(
                           {
                               terms.at(1).toInt(nullptr, 16),
                               terms.at(2).toInt(nullptr, 16),
                               terms.at(3).toInt(nullptr, 16),
                               terms.at(4).toInt(nullptr, 16),
                               terms.at(5).toInt(nullptr, 16),
                               terms.at(6).toInt(nullptr, 16),
                           });
           }
           else if(terms.at(0).compare("MinefieldTheme", Qt::CaseInsensitive) == 0)
           {
               if(terms.size() != 2)
                   continue;

               preferences->setOptionMinefieldTheme(terms.at(1));
           }
           else if(terms.at(0).compare("CustomizedMinesInPercentage", Qt::CaseInsensitive) == 0)
           {
               if(terms.size() != 2)
                   continue;

               cbCustomizedMinesInPercentage->setChecked(terms.at(1).compare("On", Qt::CaseInsensitive) == 0);
           }
           else if(terms.at(0).compare("WhenCtrlIsPressed", Qt::CaseInsensitive) == 0)
           {
               if(terms.size() != 2)
                   continue;

               preferences->setOptionWhenCtrlIsPressed(terms.at(1).toInt());
           }

        }
    }

    vUpdatePreferences();
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

    QScopedPointer<QFile> fileLastSession( new QFile(destDir.absoluteFilePath("libreminesLastSession.txt")) );

    fileLastSession->open(QIODevice::WriteOnly);

    QTextStream stream(fileLastSession.get());

    stream << "FirstCellClean" << ' ' << (preferences->optionFirstCellClean() ? "On" : "Off") << '\n'
           << "ApplicationTheme" << ' ' << preferences->optionApplicationTheme() << '\n'
           << "ClearNeighborCellsWhenClickedOnShowedCell" << ' ' << (preferences->optionCleanNeighborCellsWhenClickedOnShowedCell() ? "On" : "Off") << '\n'
           << "Username" << ' ' << preferences->optionUsername() << '\n'
           << "CustomizedPercentageOfMines" << ' ' << sbCustomizedPercentageMines->value() << '\n'
           << "CustomizedX" << ' ' << sbCustomizedX->value() << '\n'
           << "CustomizedY" << ' ' << sbCustomizedY->value() << '\n'
           << "KeyboardControllerKeys" << ' ' << preferences->optionKeyboardControllerKeysString() << '\n'
           << "ApplicationTheme" << ' ' << preferences->optionMinefieldTheme() << '\n'
           << "CustomizedMinesInPercentage" << ' ' << (cbCustomizedMinesInPercentage->isChecked() ? "On" : "Off") << '\n'
           << "WhenCtrlIsPressed" << ' ' << preferences->optionWhenCtrlIsPressed() << '\n';
}

void LibreMinesGui::vUpdatePreferences()
{
    const QList<int> keys = preferences->optionKeyboardControllerKeys();

    controller.keyLeft = keys.at(0);
    controller.keyUp = keys.at(1);
    controller.keyRight = keys.at(2);
    controller.keyDown = keys.at(3);
    controller.keyReleaseCell= keys.at(4);
    controller.keyFlagCell = keys.at(5);

    controller.valid = true;
    for (int i=0; i<keys.size()-1; ++i)
    {
        for(int j=i+1; j<keys.size(); ++j)
        {
            controller.valid &= keys[i] != keys[j];
        }
    }
    controller.valid &= !keys.contains(-1);

    if(preferences->optionUsername().isEmpty())
    {
        preferences->setOptionUsername(qgetenv("USER"));
    }
}
