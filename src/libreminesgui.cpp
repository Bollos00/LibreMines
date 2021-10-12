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


#include <QRandomGenerator>
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
#include <QTranslator>
#include <QStandardPaths>

#include "libreminesgui.h"
#include "libreminesscoresdialog.h"
#include "libreminesconfig.h"
#include "libreminesviewscoresdialog.h"

LibreMinesGui::CellGui::CellGui():
    button(nullptr),
    label(nullptr)
{

}

LibreMinesGui::LibreMinesGui(QWidget *parent, const int thatWidth, const int thatHeight) :
    QMainWindow(parent),
    gameEngine(),
    principalMatrix( std::vector< std::vector<CellGui> >(0) ),
    iLimitHeightField( 0 ),
    iLimitWidthField( 0 ),
    cellLength( 0 ),
    difficult( NONE ),
    preferences( new LibreMinesPreferencesDialog(this) )
{
    this->resize(800, 600);

    connect(preferences, &LibreMinesPreferencesDialog::SIGNAL_optionChanged,
            this, &LibreMinesGui::SLOT_optionChanged);

    // Unable central widget when the preferences dialog is active
    // Also update the preferences when finished
    connect(preferences, &LibreMinesPreferencesDialog::SIGNAL_visibilityChanged,
            [this](const bool visible)
    {
        this->centralWidget()->setEnabled(!visible);
        this->vUpdatePreferences();
    });

    // Quit the application with Ctrl + Q
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q), this), &QShortcut::activated,
            this, &LibreMinesGui::SLOT_quitApplication);

    // Create interface with the passed dimensions
    vCreateGUI(thatWidth, thatHeight);
    vShowMainMenu();

    qApp->installEventFilter(this);

    this->setWindowIcon(QIcon(":/icons_rsc/icons/libremines.svg"));
    this->setWindowTitle("LibreMines");

    // Initializr keyboard controller attributes
    controller.ctrlPressed = false;
    controller.active = false;
    controller.currentX = 0;
    controller.currentY = 0;

    // Load configuration file and set the theme
    vLastSessionLoadConfigurationFile();
    vSetMinefieldTheme(preferences->optionMinefieldTheme());
    vSetFacesReaction(preferences->optionFacesReaction());

    // Necessary for some reason
    QTimer::singleShot(100, [this](){ vSetApplicationTheme(preferences->optionApplicationStyle()); });
}

LibreMinesGui::~LibreMinesGui()
{
    vResetPrincipalMatrix();

    vLastSessionSaveConfigurationFile();
}

bool LibreMinesGui::eventFilter(QObject* object, QEvent* event)
{
    Q_UNUSED(object)

    // Just handle those two events
    if(event->type() != QEvent::KeyPress &&
       event->type() != QEvent::KeyRelease)
        return false;

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


    // Lock the cursor on the lower left of the screen while the controller is activated
//    if(controller.active)
//    {
//        qApp->overrideCursor()->setPos(90*qApp->primaryScreen()->geometry().width()/100,
//                                       90*qApp->primaryScreen()->geometry().height()/100);
//    }

    switch(event->type())
    {
        case QEvent::KeyPress:
        {
            Qt::Key key = (Qt::Key)((QKeyEvent*)event)->key();

            if(key == Qt::Key_Control)
            {
                controller.ctrlPressed = true;
                return true;
            }
            if(controller.active)
            {
                if(key == controller.keyLeft ||
                   key == controller.keyUp ||
                   key == controller.keyDown ||
                   key == controller.keyRight ||
                   key == controller.keyCenterCell ||
                   key == controller.keyFlagCell ||
                   key == controller.keyReleaseCell)
                {
                    return true;
                }
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
            // Additionally hide the cursor when the controller is activated
            if(!controller.active)
            {
                if(key == controller.keyLeft ||
                   key == controller.keyUp ||
                   key == controller.keyDown ||
                   key == controller.keyRight)
                {
                    controller.active = true;
                    vKeyboardControllerSetCurrentCell(0, 0);
                    qApp->setOverrideCursor(QCursor(Qt::BlankCursor));
                    qApp->overrideCursor()->setPos(90*qApp->primaryScreen()->geometry().width()/100,
                                                   90*qApp->primaryScreen()->geometry().height()/100);

                    this->setFocus();

                    return true;
                }
            }
            else
            {
                if(key == controller.keyLeft ||
                   key == controller.keyUp ||
                   key == controller.keyDown ||
                   key == controller.keyRight ||
                   key == controller.keyCenterCell ||
                   key == controller.keyFlagCell ||
                   key == controller.keyReleaseCell)
                {
                    this->setFocus();
                }


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
                        return true;
                    }
                    if(preferences->optionCleanNeighborCellsWhenClickedOnShowedCell())
                    {
                        Q_EMIT SIGNAL_cleanNeighborCells(controller.currentX, controller.currentY);
                        return true;
                    }

                }
                if(key == controller.keyFlagCell)
                {
                    Q_EMIT SIGNAL_addOrRemoveFlag(controller.currentX, controller.currentY);
                    return true;
                }
                if(key == Qt::Key_Space)
                {
                    vKeyboardControllerCenterCurrentCell();
                    return true;
                }
                if(key == Qt::Key_Escape)
                {
                    controller.active = false;
                    vKeyboardControllUnsetCurrentCell();
                    qApp->restoreOverrideCursor();
                    return true;
                }
            }

        }break;

        default:
            break;
    }

    return false;
}

void LibreMinesGui::resizeEvent(QResizeEvent *e)
{
    // If we just call the function here, the application freezes
    QTimer::singleShot(1, [this](){ vAdjustMainMenu(); vAdjustInterfaceInGame(); });
    QWidget::resizeEvent(e);
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
    vAdjustInterfaceInGame();
    vShowInterfaceInGame();


    // Reset the controller attributes
    controller.ctrlPressed = false;
    controller.active = false;
    controller.currentX = 0;
    controller.currentY = 0;
    qApp->restoreOverrideCursor();

    if(!controller.valid)
    {
        QMessageBox::warning(this, tr("Keyboard Controller is invalid"),
                             tr("Dear user, unfortunately your Keyboard Controller preferences"
                             " are invalid. Therefore you will not be able to play with the keyboard.\n"
                             "To fix it go to (Main Meun > Options > Preferences) and edit your preferences."));
    }

    // Create a new matrix
    principalMatrix = std::vector<std::vector<CellGui>> (_X, std::vector<CellGui>(_Y));

    buttonQuitInGame->setEnabled(false);
    buttonRestartInGame->setEnabled(false);
    buttonSaveMinefieldAsImage->setEnabled(false);

    // Create the game engine instance
    gameEngine.reset(new LibreMinesGameEngine());

    gameEngine->setFirstCellClean(preferences->optionFirstCellClean());
    gameEngine->vNewGame(_X, _Y, i_nMines_);

    // Set the length of each cell
    if(iLimitWidthField/_X < iLimitHeightField/_Y)
        cellLength = iLimitWidthField/_X;
    else
        cellLength = iLimitHeightField/_Y;

    if(cellLength < preferences->optionMinimumCellLength())
        cellLength = preferences->optionMinimumCellLength();
    else if(cellLength > preferences->optionMaximumCellLength())
        cellLength = preferences->optionMaximumCellLength();


    // Update the pixmaps
    vSetMinefieldTheme(preferences->optionMinefieldTheme());
    // Update faces reaction
    vSetFacesReaction(preferences->optionFacesReaction());

    widgetBoardContents->setGeometry(0, 0, _X*cellLength, _Y*cellLength);

    const bool bCleanNeighborCellsWhenClickedOnShowedLabel = preferences->optionCleanNeighborCellsWhenClickedOnShowedCell();

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

            cell.label->resize(cellLength, cellLength);
            cell.label->setPixmap(*pmZero);
            cell.label->show();

            cell.button->resize(cellLength, cellLength);
            cell.button->setIcon(QIcon(*pmNoFlag));
            cell.button->setIconSize(QSize(cellLength, cellLength));
            cell.button->show();
            cell.button->setEnabled(false);

            connect(cell.button, &QPushButton_adapted::SIGNAL_released,
                    this, &LibreMinesGui::SLOT_OnCellButtonReleased);
            connect(cell.button, &QPushButton_adapted::SIGNAL_clicked,
                    this, &LibreMinesGui::SLOT_OnCellButtonClicked);

            if(bCleanNeighborCellsWhenClickedOnShowedLabel)
            {
                connect(cell.label, &QLabel_adapted::SIGNAL_released,
                        this, &LibreMinesGui::SLOT_onCellLabelReleased);
                connect(cell.label, &QLabel_adapted::SIGNAL_clicked,
                        this, &LibreMinesGui::SLOT_onCellLabelClicked);
            }

            if(preferences->optionMinefieldGenerationAnimation() == LibreMines::AnimationOn ||
               (preferences->optionMinefieldGenerationAnimation() == LibreMines::AnimationLimited &&
                                   i == _X - 1))
            {
                qApp->processEvents();
            }
        }
    }

    labelTimerInGame->setNum(0);
    labelYouWonYouLost->setText(" ");

    buttonQuitInGame->setEnabled(true);
    buttonRestartInGame->setEnabled(true);
    buttonSaveMinefieldAsImage->setEnabled(true);

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

    if(preferences->optionProgressBar())
    {
        progressBarGameCompleteInGame->setRange(-gameEngine->cellsToUnlock(), 0);
        progressBarGameCompleteInGame->setValue(-gameEngine->cellsToUnlock());

        connect(gameEngine.get(), &LibreMinesGameEngine::SIGNAL_showCell,
                [this](){ progressBarGameCompleteInGame->setValue(-gameEngine->hiddenCells()); });
    }

    // Set the initial value of mines left to the total number
    //  of mines
    SLOT_minesLeft(gameEngine->mines());

    labelFaceReactionInGame->setPixmap(*pmSmillingFace);
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

void LibreMinesGui::vCreateGUI(const int width, const int height)
{
    // Create the interface

    setCentralWidget(new QWidget(this));

    // Actions and Menu Bar
    actionPreferences = new QAction(this);
    actionHighScores = new QAction(this);
    actionToggleFullScreen = new QAction(this);
    actionAbout = new QAction(this);
    actionAboutQt = new QAction(this);
    actionGitHubHomePage = new QAction(this);

    QMenuBar* menuBarGlobal = new QMenuBar(this);

    menuBarGlobal->setGeometry(0, 0, this->width(), 100);

    menuOptions = new QMenu(menuBarGlobal);
    menuHelp = new QMenu(menuBarGlobal);

    this->setMenuBar(menuBarGlobal);
    this->setStatusBar(new QStatusBar(this));

    menuBarGlobal->addAction(menuOptions->menuAction());
    menuBarGlobal->addAction(menuHelp->menuAction());

    menuOptions->addActions({actionPreferences, actionHighScores, actionToggleFullScreen});
    menuHelp->addActions({actionAbout, actionAboutQt, actionGitHubHomePage});

    menuOptions->setTitle(tr("Options"));
    menuHelp->setTitle(tr("Help"));

    actionPreferences->setText(tr("Preferences..."));
    actionHighScores->setText(tr("High Scores..."));
    actionToggleFullScreen->setText(tr("Toggle Full Screen"));
    actionToggleFullScreen->setShortcut(QKeySequence(Qt::Key_F11));

    actionAbout->setText(tr("About..."));
    actionAboutQt->setText(tr("About Qt..."));
    actionGitHubHomePage->setText(tr("GitHub Homepage..."));
    // Actions and Menu Bar


    // Interface In Game
    this->setFont(QFont("Liberation Sans"));
    labelFaceReactionInGame = new QLabel(centralWidget());
    labelTimerInGame = new QLabel(centralWidget());
    lcd_numberMinesLeft = new QLCDNumber(centralWidget());
    progressBarGameCompleteInGame = new QProgressBar(centralWidget());
    buttonRestartInGame = new QPushButton(centralWidget());
    buttonQuitInGame = new QPushButton(centralWidget());
    buttonSaveMinefieldAsImage = new QPushButton(centralWidget());
    labelYouWonYouLost = new QLabel(centralWidget());
    labelStatisLastMatch = new QLabel(centralWidget());

    scrollAreaBoard = new QScrollArea(centralWidget());
    widgetBoardContents = new QWidget();

    layoutBoard = new QGridLayout();
    layoutBoard->setSpacing(0);

    widgetBoardContents->setLayout(layoutBoard);
    widgetBoardContents->setFocusPolicy(Qt::NoFocus);
    scrollAreaBoard->setWidget(widgetBoardContents);
    scrollAreaBoard->setFocusPolicy(Qt::NoFocus);

    labelTimerInGame->setFont(QFont("Liberation Sans", 40));
    labelTimerInGame->setNum(0);
    lcd_numberMinesLeft->setDecMode();
    lcd_numberMinesLeft->display(0);
    progressBarGameCompleteInGame->setTextVisible(false);
    buttonRestartInGame->setText(tr("Restart"));
    buttonQuitInGame->setText(tr("Quit"));
    buttonSaveMinefieldAsImage->setText(tr("Save Minefield as Image"));
    labelYouWonYouLost->setFont(QFont("Liberation Sans", 15));

    buttonSaveMinefieldAsImage->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_P));
    buttonRestartInGame->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));

    vAdjustInterfaceInGame();
    vHideInterfaceInGame();
    // Interface In Game


    // Create Main Menu Widgets
    buttonEasy = new QPushButton(centralWidget());
    buttonEasy->setText(tr("Easy") + "\n\n8x8\n\n" + tr("10 Mines"));
    buttonEasy->setFont(QFont("Liberation Sans",20));

    buttonMedium= new QPushButton(centralWidget());
    buttonMedium->setText(tr("Medium") + "\n\n16x16\n\n" + tr("40 Mines"));
    buttonMedium->setFont(QFont("Liberation Sans", 20));


    buttonHard = new QPushButton(centralWidget());
    buttonHard->setText(tr("Hard") + "\n\n30x16\n\n" + tr("99 Mines"));
    buttonHard->setFont(QFont("Liberation Sans", 20));

    buttonCustomizedNewGame = new QPushButton(centralWidget());
    buttonCustomizedNewGame->setText(tr("Customized Game"));
    buttonCustomizedNewGame->setFont(QFont("Liberation Sans", 20));

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
    labelCustomizedX->setText(tr("Width: "));

    labelCustomizedY = new QLabel(centralWidget());
    labelCustomizedY->setText(tr("Height: "));

    labelCustomizedPercentageMines = new QLabel(centralWidget());
    labelCustomizedPercentageMines->setText(tr("Percent of Mines: "));

    labelCustomizedNumbersOfMines = new QLabel(centralWidget());
    labelCustomizedNumbersOfMines->setText(tr("Number of Mines: "));
    // Create Main Menu Widgets

    cbCustomizedMinesInPercentage->setChecked(true);

    labelCustomizedNumbersOfMines->hide();
    sbCustomizedNumbersOfMines->hide();

    connect(buttonEasy, &QPushButton::released,
            this, &LibreMinesGui::SLOT_Easy);

    connect(buttonMedium, &QPushButton::released,
            this, &LibreMinesGui::SLOT_Medium);

    connect(buttonHard, &QPushButton::released,
            this, &LibreMinesGui::SLOT_Hard);

    connect(buttonCustomizedNewGame, &QPushButton::released,
            this, &LibreMinesGui::SLOT_Customized);

    connect(buttonRestartInGame, &QPushButton::released,
            this, &LibreMinesGui::SLOT_RestartGame);

    connect(buttonQuitInGame, &QPushButton::released,
            this, &LibreMinesGui::SLOT_QuitGame);

    connect(buttonSaveMinefieldAsImage, &QPushButton::released,
            this, &LibreMinesGui::SLOT_saveMinefieldAsImage);

    connect(actionPreferences, &QAction::triggered,
            preferences, &QDialog::show);

    connect(actionHighScores, &QAction::triggered,
            this, &LibreMinesGui::SLOT_showHighScores);

    connect(actionToggleFullScreen, &QAction::triggered,
            this, &LibreMinesGui::SLOT_toggleFullScreen);


    connect(actionAbout, &QAction::triggered,
            this, &LibreMinesGui::SLOT_showAboutDialog);

    connect(actionAboutQt, &QAction::triggered,
            [this](){ QMessageBox::aboutQt(this, "LibreMines"); });

    connect(actionGitHubHomePage, &QAction::triggered,
            [](){ QDesktopServices::openUrl(QUrl("https://github.com/Bollos00/LibreMines")); });

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

    // Tab order of application
    setTabOrder(buttonEasy, buttonMedium);
    setTabOrder(buttonMedium, buttonHard);
    setTabOrder(buttonHard, buttonCustomizedNewGame);
    setTabOrder(buttonCustomizedNewGame, sbCustomizedNumbersOfMines);
    setTabOrder(sbCustomizedNumbersOfMines, sbCustomizedPercentageMines);
    setTabOrder(sbCustomizedPercentageMines, cbCustomizedMinesInPercentage);
    setTabOrder(cbCustomizedMinesInPercentage, sbCustomizedX);
    setTabOrder(sbCustomizedX, sbCustomizedY);
    setTabOrder(sbCustomizedY, buttonRestartInGame);
    setTabOrder(buttonRestartInGame, buttonQuitInGame);
    setTabOrder(buttonQuitInGame, buttonSaveMinefieldAsImage);

    if(width == -1 || height == -1)
    {
        this->showNormal();
        this->showMaximized();
    }
    else
    {
        resize(width, height);
    }

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

void LibreMinesGui::vAdjustMainMenu()
{

    // Width and Height of the main menu
    const int w = centralWidget()->width();
    const int h = centralWidget()->height();

    buttonEasy->setGeometry(w/20, h/20,
                            2*w/5, 2*h/5);

    buttonMedium->setGeometry(buttonEasy->x() + buttonEasy->width() + w/10, h/20,
                              2*w/5, 2*h/5);

    buttonHard->setGeometry(w/20, buttonEasy->y() + buttonEasy->height() + h/10,
                            2*w/5, 2*h/5);

    buttonCustomizedNewGame->setGeometry(buttonEasy->x() + buttonEasy->width() + w/10,
                                         buttonEasy->y() + buttonEasy->height() + h/10,
                                         2*w/5,
                                         1*h/5);

    labelCustomizedPercentageMines->setGeometry(buttonCustomizedNewGame->x(),
                                                buttonCustomizedNewGame->y() + buttonCustomizedNewGame->height(),
                                                buttonCustomizedNewGame->width()/2,
                                                buttonCustomizedNewGame->height()/3);

    labelCustomizedNumbersOfMines->setGeometry(labelCustomizedPercentageMines->geometry());

    labelCustomizedX->setGeometry(labelCustomizedPercentageMines->x(),
                                  labelCustomizedPercentageMines->y()+labelCustomizedPercentageMines->height(),
                                  labelCustomizedPercentageMines->width(),
                                  labelCustomizedPercentageMines->height());

    labelCustomizedY->setGeometry(labelCustomizedX->x(),
                                  labelCustomizedX->y()+labelCustomizedX->height(),
                                  labelCustomizedX->width(),
                                  labelCustomizedX->height());

    sbCustomizedPercentageMines->setGeometry(labelCustomizedPercentageMines->x()+labelCustomizedPercentageMines->width(),
                                             labelCustomizedPercentageMines->y(),
                                             9*labelCustomizedPercentageMines->width()/10,
                                             labelCustomizedPercentageMines->height());

    sbCustomizedNumbersOfMines->setGeometry(sbCustomizedPercentageMines->geometry());

    cbCustomizedMinesInPercentage->setGeometry(sbCustomizedNumbersOfMines->x() + sbCustomizedNumbersOfMines->width(),
                                               sbCustomizedNumbersOfMines->y(),
                                               labelCustomizedNumbersOfMines->width()/10,
                                               sbCustomizedNumbersOfMines->height());

    sbCustomizedX->setGeometry(labelCustomizedX->x()+labelCustomizedX->width(), labelCustomizedX->y(),
                               labelCustomizedX->width(), labelCustomizedX->height());

    sbCustomizedY->setGeometry(labelCustomizedY->x()+labelCustomizedY->width(), labelCustomizedY->y(),
                               labelCustomizedY->width(), labelCustomizedY->height());

    cbCustomizedMinesInPercentage->setStyleSheet(
                "QCheckBox::indicator "
                "{"
                "    width: " + QString::number(cbCustomizedMinesInPercentage->width()) + "px;"
                "    height: " + QString::number(cbCustomizedMinesInPercentage->width()) + "px;"
                "}");
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

void LibreMinesGui::vAdjustInterfaceInGame()
{
    const int w = centralWidget()->width();
    const int h = centralWidget()->height();


    iLimitWidthField = 8*w/10;
    iLimitHeightField = h;

    scrollAreaBoard->setGeometry(0, 0, iLimitWidthField, iLimitHeightField);

    labelFaceReactionInGame->setGeometry(88*w /100, h /20,
                                         9*w /100, 9*w /100);
    labelTimerInGame->setGeometry(85*w /100, labelFaceReactionInGame->y() + labelFaceReactionInGame->height(),
                                  15*w /100, h /14);
    lcd_numberMinesLeft->setGeometry(labelTimerInGame->x(), labelTimerInGame->y()+labelTimerInGame->height(),
                                     labelTimerInGame->width(), h /7);
    progressBarGameCompleteInGame->setGeometry(lcd_numberMinesLeft->x(), lcd_numberMinesLeft->y()+lcd_numberMinesLeft->height(),
                                     lcd_numberMinesLeft->width(), h /20);
    buttonRestartInGame->setGeometry(progressBarGameCompleteInGame->x(), progressBarGameCompleteInGame->y()+progressBarGameCompleteInGame->height(),
                                     progressBarGameCompleteInGame->width()/2, h /20);
    buttonQuitInGame->setGeometry(buttonRestartInGame->x()+buttonRestartInGame->width(), buttonRestartInGame->y(),
                                  buttonRestartInGame->width(), buttonRestartInGame->height());
    buttonSaveMinefieldAsImage->setGeometry(buttonRestartInGame->x(), buttonRestartInGame->y() + buttonRestartInGame->height()*1.1,
                                            progressBarGameCompleteInGame->width(), progressBarGameCompleteInGame->height());
    labelYouWonYouLost->setGeometry(lcd_numberMinesLeft->x(), buttonRestartInGame->y()+buttonRestartInGame->height()+h /20,
                                    lcd_numberMinesLeft->width(), lcd_numberMinesLeft->height());
    labelStatisLastMatch->setGeometry(labelYouWonYouLost->x(), labelYouWonYouLost->y() + labelYouWonYouLost->height(),
                                      labelYouWonYouLost->width(), h /5);
}


void LibreMinesGui::vHideInterfaceInGame()
{
    labelFaceReactionInGame->hide();
    labelTimerInGame->hide();
    lcd_numberMinesLeft->hide();
    progressBarGameCompleteInGame->hide();
    buttonRestartInGame->hide();
    buttonQuitInGame->hide();
    buttonSaveMinefieldAsImage->hide();
    labelYouWonYouLost->hide();
    labelStatisLastMatch->hide();
    widgetBoardContents->hide();
    scrollAreaBoard->hide();
}

void LibreMinesGui::vShowInterfaceInGame()
{
    labelFaceReactionInGame->show();
    labelTimerInGame->show();
    lcd_numberMinesLeft->show();
    if(preferences->optionProgressBar())
        progressBarGameCompleteInGame->show();
    buttonRestartInGame->show();
    buttonQuitInGame->show();
    buttonSaveMinefieldAsImage->show();
    labelYouWonYouLost->show();
    labelStatisLastMatch->show();
    widgetBoardContents->show();
    scrollAreaBoard->show();
}

void LibreMinesGui::SLOT_RestartGame()
{
    // If the Interface in Game is hidden or not enable, return
    if(!buttonRestartInGame->isVisible() || !buttonRestartInGame->isEnabled())
        return;

    // Check if there is a game happening. If there is one, create a dialog asking
    //  if the user want to quit the game
    if(gameEngine && gameEngine->isGameActive() &&
       progressBarGameCompleteInGame->value() != progressBarGameCompleteInGame->minimum())
    {
        QMessageBox messageBox;

        messageBox.setText(tr("There is a game happening."));
        messageBox.setInformativeText(tr("Are you sure you want to quit?"));
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

void LibreMinesGui::SLOT_QuitGame()
{
    // Check if there is a game happening. If there is one, create a dialog asking
    //  if the user want to quit the game
    if(gameEngine && gameEngine->isGameActive() &&
       progressBarGameCompleteInGame->value() != progressBarGameCompleteInGame->minimum())
    {
        QMessageBox messageBox;

        messageBox.setText(tr("There is a game happening."));
        messageBox.setInformativeText(tr("Are you sure you want to quit?"));
        messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        messageBox.setDefaultButton(QMessageBox::No);
        int result = messageBox.exec();

        if(result == QMessageBox::No)
            return;
    }

    qApp->restoreOverrideCursor();

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

    labelFaceReactionInGame->setPixmap(*pmSmillingFace);

    // if the button is released outside its area no not treat the event
    if(e->localPos().x() >= cellLength || e->localPos().x() < 0 ||
       e->localPos().y() >= cellLength || e->localPos().y() < 0)
    {
        return;
    }

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

void LibreMinesGui::SLOT_OnCellButtonClicked(const QMouseEvent *const e)
{
    if(!gameEngine->isGameActive() || controller.active)
        return;

    if(e->button() != Qt::LeftButton)
        return;

    labelFaceReactionInGame->setPixmap(*pmOpenMouthFace);
}

void LibreMinesGui::SLOT_onCellLabelReleased(const QMouseEvent *const e)
{
    if(!gameEngine->isGameActive() || controller.active)
        return;

    labelFaceReactionInGame->setPixmap(*pmSmillingFace);

    // if the button is released outside its area no not treat the event
    if(e->localPos().x() >= cellLength || e->localPos().x() < 0 ||
       e->localPos().y() >= cellLength || e->localPos().y() < 0)
    {
        return;
    }

    QLabel_adapted *buttonClicked = (QLabel_adapted *) sender();

    for(uchar j=0; j<gameEngine->lines(); j++)
    {
        for (uchar i=0; i<gameEngine->rows(); i++)
        {
            // Find the emissor of the signal
            if(buttonClicked == principalMatrix[i][j].label)
            {
                if(e->button() != Qt::LeftButton)
                    return;

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

void LibreMinesGui::SLOT_onCellLabelClicked(const QMouseEvent *const e)
{
    Q_UNUSED(e)

    if(!gameEngine->isGameActive() || controller.active)
        return;

    labelFaceReactionInGame->setPixmap(*pmGrimacingFace);

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
    QString QS_Statics =
            tr("Total time: ") + QString::number(score.iTimeInNs*1e-9, 'f', 3) + tr(" secs") + '\n'
            + tr("Correct Flags: ") + QString::number(iCorrectFlags) + '\n'
            + tr("Wrong Flags: ") + QString::number(iWrongFlags) + '\n'
            + tr("Unlocked Cells: ") + QString::number(iUnlockedCells) + '\n'
            + '\n'
            + tr("Flags/s: ") + QString::number(dFlagsPerSecond, 'f', 3)  + '\n'
            + tr("Cells/s: ") + QString::number(dCellsPerSecond, 'f', 3) + '\n'
            + '\n'
            + tr("Game Complete: ") + QString::number(score.dPercentageGameCompleted, 'f', 2) + " %";

    labelStatisLastMatch->setText(QS_Statics);

    score.gameDifficulty = difficult;
    score.username = preferences->optionUsername();
    if(score.username.isEmpty())
#ifdef Q_OS_WINDOWS
    {
        score.username = qgetenv("USERNAME");
    }
#else
    {
        score.username = qgetenv("USER");
    }
#endif

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
                strGameDiffuclty = tr("Easy");
            else if(score.gameDifficulty == MEDIUM)
                strGameDiffuclty = tr("Medium");
            else if(score.gameDifficulty == HARD)
                strGameDiffuclty = tr("Hard");
            else if(score.gameDifficulty == CUSTOMIZED)
            {
                strGameDiffuclty = tr("Customized ") + QString::number(score.width) +
                                   " x " + QString::number(score.heigth) + " : " +
                                   QString::number(score.mines);
            }

            AskToSaveMatchScore behaviour = preferences->optionAskToSaveMatchScoreBehaviour();

            if(behaviour == LibreMines::SaveAlways ||
               ((behaviour & LibreMines::SaveWhenGameCompleted) && score.bGameCompleted) ||
               ((behaviour & LibreMines::SaveWhenNewHighScore) && index == 0))
            {
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
            labelYouWonYouLost->setText(tr("You Won") + '\n' + tr("Difficulty: EASY"));
            break;
        case MEDIUM:
            labelYouWonYouLost->setText(tr("You Won") + '\n' + tr("Difficulty: MEDIUM"));
            break;
        case HARD:
            labelYouWonYouLost->setText(tr("You Won") + '\n' + tr("Difficulty: HARD"));
            break;
        case CUSTOMIZED:
            labelYouWonYouLost->setText(tr("You Won") +'\n' + tr("Difficulty: CUSTOM\n") +
                                        QString::number(gameEngine->rows()) +
                                        "x" + QString::number(gameEngine->lines()) +
                                        " : " + QString::number(gameEngine->mines()) + tr(" Mines"));
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
        qApp->restoreOverrideCursor();
        vKeyboardControllUnsetCurrentCell();
    }

    labelFaceReactionInGame->setPixmap(*pmGrinningFace);
}

void LibreMinesGui::SLOT_gameLost(const uchar _X, const uchar _Y)
{
    switch (difficult)
    {
        case NONE:
            break;
        case EASY:
            labelYouWonYouLost->setText(tr("You Lost") + '\n' + tr("Difficulty: EASY"));
            break;
        case MEDIUM:
            labelYouWonYouLost->setText(tr("You Lost") + '\n' + tr("Difficulty: MEDIUM"));
            break;
        case HARD:
            labelYouWonYouLost->setText(tr("You Lost") + '\n' + tr("Difficulty: HARD"));
            break;
        case CUSTOMIZED:
            labelYouWonYouLost->setText(tr("You Lost") + '\n' + tr("Difficulty: CUSTOM\n") +
                                        QString::number(gameEngine->rows()) +
                                        "x" +
                                        QString::number(gameEngine->lines()) +
                                        " : " +
                                        QString::number(gameEngine->mines()) +
                                        tr(" Mines"));
    }
    principalMatrix[_X][_Y].label->setPixmap(*pmBoom);


    for(uchar j=0; j<gameEngine->lines(); j++)
    {
        for (uchar i=0; i<gameEngine->rows(); i++)
        {
            CellGui& cellGui = principalMatrix[i][j];
            const LibreMinesGameEngine::CellGameEngine& cellGE = gameEngine->getPrincipalMatrix()[i][j];

//            cellGui.button->setEnabled(false);
//            cellGui.label->setEnabled(false);

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
            }
        }
    }

    if(controller.active)
    {
        controller.active = false;
        qApp->restoreOverrideCursor();
        vKeyboardControllUnsetCurrentCell();
    }

    labelFaceReactionInGame->setPixmap(*pmDizzyFace);
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
    if(gameEngine && gameEngine->isGameActive() &&
       progressBarGameCompleteInGame->value() != progressBarGameCompleteInGame->minimum())
    {
        QMessageBox messageBox;

        messageBox.setText(tr("There is a game happening."));
        messageBox.setInformativeText(tr("Are you sure you want to quit?"));
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
            "LibreMines " + QString(LIBREMINES_PROJECT_VERSION) + "\n" +
            tr("Copyright (C) 2020-2021  Bruno Bollos Correa\n"
            "\n"
            "This program is free software: you can redistribute it and/or modify"
            " it under the terms of the GNU General Public License as published by"
            " the Free Software Foundation, either version 3 of the License, or"
            " (at your option) any later version.\n"
            "\n"
            "This program is distributed in the hope that it will be useful,"
            " but WITHOUT ANY WARRANTY; without even the implied warranty of"
            " MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"
            " GNU General Public License for more details.\n"
            "\n"
            "You should have received a copy of the GNU General Public License"
            " along with this program.  If not, see <http://www.gnu.org/licenses/>.\n"
            "\n"
            "Get the source code of LibreMines on\n") +
            "<" + QString(LIBREMINES_PROJECT_HOMEPAGE_URL) + ">";

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
    LibreMinesViewScoresDialog dialog(this);
    dialog.setWindowIcon(QIcon(":/icons_rsc/icons/libremines.svg"));
    dialog.setScores(scores);
    dialog.exec();
}

void LibreMinesGui::SLOT_toggleFullScreen()
{
    if(isFullScreen())
    {
        this->showNormal();
        this->showMaximized();
    }
    else
    {
        this->showFullScreen();
    }
}

void LibreMinesGui::SLOT_saveMinefieldAsImage()
{
    if(controller.active)
        qApp->restoreOverrideCursor();

    QString picturesDirPAth = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if(picturesDirPAth.isEmpty())
        picturesDirPAth = QDir::currentPath();

    QString fileName = "libremines_screeshoot" + QDateTime::currentDateTime().toString(Qt::ISODate).replace(':', '_') + ".png";

    QString fullFileName = QFileDialog::getSaveFileName(
                           this,
                           tr("Save Minefield as image"),
                           picturesDirPAth + '/' + fileName,
                           tr("Image (*.bmp *.jpg *png *.jpeg)"));

    if(fullFileName.isEmpty())
        return;

    QImage image(widgetBoardContents->size(), QImage::Format_RGBA64);
    QPainter painter(&image);
    widgetBoardContents->render(&painter);
    image.save(fullFileName);

    if(controller.active)
        qApp->setOverrideCursor(QCursor(Qt::BlankCursor));
}

void LibreMinesGui::vSetApplicationTheme(const QString& theme)
{
    static bool firstTimeHere = true;

    if(theme.compare("default", Qt::CaseInsensitive) == 0)
    {
        if(!firstTimeHere)
        {
            QMessageBox::information(this, "LibreMines",
                                     "Please reset the application to apply this change");
        }
    }
    else if(theme.compare("FusionDark", Qt::CaseInsensitive) == 0)
    {
        qApp->setStyleSheet("");
        qApp->setStyle(QStyleFactory::create ("Fusion"));
        QPalette palette;
        palette.setColor(QPalette::BrightText,      Qt::red);
        palette.setColor(QPalette::WindowText,      Qt::white);
        palette.setColor(QPalette::ToolTipBase,     Qt::white);
        palette.setColor(QPalette::ToolTipText,     Qt::white);
        palette.setColor(QPalette::Text,            Qt::white);
        palette.setColor(QPalette::ButtonText,      Qt::white);
        palette.setColor(QPalette::HighlightedText, Qt::black);
        palette.setColor(QPalette::Window,          QColor (53, 53, 53));
        palette.setColor(QPalette::Base,            QColor (25, 25, 25));
        palette.setColor(QPalette::AlternateBase,   QColor (53, 53, 53));
        palette.setColor(QPalette::Button,          QColor (53, 53, 53));
        palette.setColor(QPalette::Link,            QColor (42, 130, 218));
        palette.setColor(QPalette::Highlight,       QColor (42, 130, 218));

        qApp->setPalette(palette);
    }
    else if(theme.compare("FusionLight", Qt::CaseInsensitive) == 0)
    {
        qApp->setStyleSheet("");
        qApp->setStyle(QStyleFactory::create ("Fusion"));
        QPalette palette;
        palette.setColor(QPalette::BrightText,      Qt::cyan);
        palette.setColor(QPalette::WindowText,      Qt::black);
        palette.setColor(QPalette::ToolTipBase,     Qt::black);
        palette.setColor(QPalette::ToolTipText,     Qt::black);
        palette.setColor(QPalette::Text,            Qt::black);
        palette.setColor(QPalette::ButtonText,      Qt::black);
        palette.setColor(QPalette::HighlightedText, Qt::white);
        palette.setColor(QPalette::Window,          QColor (202, 202, 202));
        palette.setColor(QPalette::Base,            QColor (228, 228, 228));
        palette.setColor(QPalette::AlternateBase,   QColor (202, 202, 202));
        palette.setColor(QPalette::Button,          QColor (202, 202, 202));
        palette.setColor(QPalette::Link,            QColor (213, 125, 37));
        palette.setColor(QPalette::Highlight,       QColor (42, 130, 218));

        qApp->setPalette(palette);
    }
    else if(QStyleFactory::keys().contains(theme))
    {
        qApp->setStyleSheet("");
        qApp->setPalette(QPalette());
        qApp->setStyle(QStyleFactory::create(theme));
    }
    else
    {
        qApp->setPalette(QPalette());
        qApp->setStyle("");

        QString prefix;

        if(theme.compare("ConsoleStyle", Qt::CaseInsensitive) == 0)
            prefix = ":/qss/ConsoleStyle.qss";
        else if(theme.compare("NeonButtons", Qt::CaseInsensitive) == 0)
            prefix = ":/qss/NeonButtons.qss";
        else if(theme.compare("QDarkStyle", Qt::CaseInsensitive) == 0)
            prefix = ":/qdarkstyle/dark/style.qss";
        else if(theme.compare("QDarkStyleLight", Qt::CaseInsensitive) == 0)
            prefix = ":/qdarkstyle/light/style.qss";
        else if(theme.compare("BreezeDark", Qt::CaseInsensitive) == 0)
            prefix = ":/breeze/dark.qss";
        else if(theme.compare("BreezeLight", Qt::CaseInsensitive) == 0)
            prefix = ":/breeze/light.qss";


        QFile fileQSS(prefix);

        if (!fileQSS.exists())
        {
            qWarning() << "Unable to set stylesheet, file not found";
        }
        else
        {
            fileQSS.open(QFile::ReadOnly | QFile::Text);
            QTextStream ts(&fileQSS);
            qApp->setStyleSheet(ts.readAll());
        }
    }

    firstTimeHere = false;
}

void LibreMinesGui::vSetMinefieldTheme(const QString &theme)
{
    QString prefix;
    if(theme.compare("ClassicLight", Qt::CaseInsensitive) == 0)
    {
        prefix = ":/minefield_icons/classic_light/";
    }
    else if(theme.compare("ClassicDark", Qt::CaseInsensitive) == 0)
    {
        prefix = ":/minefield_icons/classic_dark/";
    }
    else if(theme.compare("TwEmoji", Qt::CaseInsensitive) == 0)
    {
        prefix = ":/minefield_icons/twemoji/";
    }
    else
    {
        qWarning() << "Minefield selected " << theme << " does not exist";
        return;
    }

    pmZero.reset( new QPixmap(  QIcon(prefix + "0.svg").pixmap(cellLength, cellLength)  ) );
    pmOne.reset( new QPixmap(  QIcon(prefix + "1.svg").pixmap(cellLength, cellLength)  ) );
    pmTwo.reset( new QPixmap(  QIcon(prefix + "2.svg").pixmap(cellLength, cellLength)  ) );
    pmThree.reset( new QPixmap(  QIcon(prefix + "3.svg").pixmap(cellLength, cellLength)  ) );
    pmFour.reset( new QPixmap(  QIcon(prefix + "4.svg").pixmap(cellLength, cellLength)  ) );
    pmFive.reset( new QPixmap(  QIcon(prefix + "5.svg").pixmap(cellLength, cellLength)  ) );
    pmSix.reset( new QPixmap(  QIcon(prefix + "6.svg").pixmap(cellLength, cellLength)  ) );
    pmSeven.reset( new QPixmap(  QIcon(prefix + "7.svg").pixmap(cellLength, cellLength)  ) );
    pmEight.reset( new QPixmap(  QIcon(prefix + "8.svg").pixmap(cellLength, cellLength)  ) );
    pmFlag.reset( new QPixmap(  QIcon(prefix + "flag.svg").pixmap(cellLength, cellLength)  ) );
    pmNoFlag.reset( new QPixmap(  QIcon(prefix + "no_flag.svg").pixmap(cellLength, cellLength)  ) );
    pmMine.reset( new QPixmap(  QIcon(prefix + "mine.svg").pixmap(cellLength, cellLength)  ) );
    pmBoom.reset( new QPixmap(  QIcon(prefix + "boom.svg").pixmap(cellLength, cellLength)  ) );
    pmWrongFlag.reset( new QPixmap(  QIcon(prefix + "wrong_flag.svg").pixmap(cellLength, cellLength)  ) );
}

void LibreMinesGui::vSetFacesReaction(const QString &which)
{
    if(which.compare("Disable", Qt::CaseInsensitive) == 0)
    {
        pmDizzyFace.reset( new QPixmap() );
        pmGrimacingFace.reset( new QPixmap() );
        pmGrinningFace.reset( new QPixmap() );
        pmOpenMouthFace.reset( new QPixmap() );
        pmSmillingFace.reset( new QPixmap() );
    }
    else
    {
        QString prefix = ":/facesreaction/faces_reaction/open-emoji-color/";
        if(which.compare("OpenEmojiColored", Qt::CaseInsensitive) == 0)
            prefix = ":/facesreaction/faces_reaction/open-emoji-color/";
        else if(which.compare("OpenEmojiBlack", Qt::CaseInsensitive) == 0)
            prefix = ":/facesreaction/faces_reaction/open-emoji-black/";
        else if(which.compare("OpenEmojiWhite", Qt::CaseInsensitive) == 0)
            prefix = ":/facesreaction/faces_reaction/open-emoji-white/";
        else if(which.compare("TwEmojiColored", Qt::CaseInsensitive) == 0)
            prefix = ":/facesreaction/faces_reaction/twemoji-color/";
        else
        {
            qWarning() << "Faces reaction option: \"" << qPrintable(which) << "\" will not be handled";
        }

        const int length = labelFaceReactionInGame->width();

        pmDizzyFace.reset( new QPixmap( QIcon(prefix + "dizzy_face.svg").pixmap(length, length) ));
        pmGrimacingFace.reset( new QPixmap( QIcon(prefix + "grimacing_face.svg").pixmap(length, length) ));
        pmGrinningFace.reset( new QPixmap( QIcon(prefix + "grinning_face.svg").pixmap(length, length) ));
        pmOpenMouthFace.reset( new QPixmap( QIcon(prefix + "open_mouth_face.svg").pixmap(length, length) ));
        pmSmillingFace.reset( new QPixmap( QIcon(prefix + "smilling_face.svg").pixmap(length, length) ));
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

    scrollAreaBoard->ensureVisible(x*cellLength + cellLength/2, y*cellLength + cellLength/2,
                                   cellLength/2 + 1, cellLength/2 + 1);
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

void LibreMinesGui::vKeyboardControllerCenterCurrentCell()
{
    const uchar x = controller.currentX;
    const uchar y = controller.currentY;
    scrollAreaBoard->ensureVisible(x*cellLength + cellLength/2, y*cellLength + cellLength/2,
                                   cellLength/2 + scrollAreaBoard->width()/2, cellLength/2 + scrollAreaBoard->height()/2);

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
            else if(terms.at(0).compare("ApplicationStyle", Qt::CaseInsensitive) == 0 ||
                    terms.at(0).compare("ApplicationTheme", Qt::CaseInsensitive) == 0)
            {
                if(terms.size() != 2)
                    continue;

                preferences->setOptionApplicationStyle(terms.at(1));
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
                if(terms.size() == 7)
                {
                    preferences->setOptionKeyboardControllerKeys(
                                {
                                    terms.at(1).toInt(nullptr, 16),
                                    terms.at(2).toInt(nullptr, 16),
                                    terms.at(3).toInt(nullptr, 16),
                                    terms.at(4).toInt(nullptr, 16),
                                    terms.at(5).toInt(nullptr, 16),
                                    terms.at(6).toInt(nullptr, 16)
                                });
                }
                else if(terms.size() == 8)
                {
                    preferences->setOptionKeyboardControllerKeys(
                                {
                                    terms.at(1).toInt(nullptr, 16),
                                    terms.at(2).toInt(nullptr, 16),
                                    terms.at(3).toInt(nullptr, 16),
                                    terms.at(4).toInt(nullptr, 16),
                                    terms.at(5).toInt(nullptr, 16),
                                    terms.at(6).toInt(nullptr, 16),
                                    terms.at(7).toInt(nullptr, 16)
                                });
                }
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
            else if(terms.at(0).compare("MinimumCellLength", Qt::CaseInsensitive) == 0)
            {
                if(terms.size() != 2)
                    continue;

                preferences->setOptionMinimumCellLength(terms.at(1).toInt());
            }
            else if(terms.at(0).compare("MaximumCellLength", Qt::CaseInsensitive) == 0)
            {
                if(terms.size() != 2)
                    continue;

                preferences->setOptionMaximumCellLength(terms.at(1).toInt());
            }
            else if(terms.at(0).compare("FacesReaction", Qt::CaseInsensitive) == 0)
            {
                if(terms.size() != 2)
                    continue;

                preferences->setOptionFacesReaction(terms.at(1));
            }
            else if(terms.at(0).compare("ProgressBar", Qt::CaseInsensitive) == 0)
            {
                if(terms.size() != 2)
                    continue;

                preferences->setOptionProgressBar(terms.at(1));
            }
            else if(terms.at(0).compare("MinefieldGenerationAnimation", Qt::CaseInsensitive) == 0)
            {
                if(terms.size() != 2)
                    continue;

                preferences->setOptionMinefieldGenerationAnimation(terms.at(1));
            }
            else if(terms.at(0).compare("AskToSaveMatchScoreBehaviour", Qt::CaseInsensitive) == 0)
            {
                if(terms.size() != 2)
                    continue;

                preferences->setOptionAskToSaveMatchScoreBehaviour(terms.at(1).toUInt());
            }

        }
    }


    QScopedPointer<QFile> fileLanguage( new QFile(destDir.absoluteFilePath("libreminesDefaultLanguage.txt")) );

    fileLanguage->open(QIODevice::ReadOnly);

    QTextStream stream(fileLanguage.get());
    QString language;
    stream >> language;

    preferences->setOptionLanguage(language);

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
    {
        QTextStream stream(fileLastSession.get());

        stream << "FirstCellClean" << ' ' << (preferences->optionFirstCellClean() ? "On" : "Off") << '\n'
               << "ApplicationStyle" << ' ' << preferences->optionApplicationStyle() << '\n'
               << "ClearNeighborCellsWhenClickedOnShowedCell" << ' ' << (preferences->optionCleanNeighborCellsWhenClickedOnShowedCell() ? "On" : "Off") << '\n'
               << "Username" << ' ' << preferences->optionUsername() << '\n'
               << "CustomizedPercentageOfMines" << ' ' << sbCustomizedPercentageMines->value() << '\n'
               << "CustomizedX" << ' ' << sbCustomizedX->value() << '\n'
               << "CustomizedY" << ' ' << sbCustomizedY->value() << '\n'
               << "KeyboardControllerKeys" << ' ' << preferences->optionKeyboardControllerKeysString() << '\n'
               << "MinefieldTheme" << ' ' << preferences->optionMinefieldTheme() << '\n'
               << "CustomizedMinesInPercentage" << ' ' << (cbCustomizedMinesInPercentage->isChecked() ? "On" : "Off") << '\n'
               << "WhenCtrlIsPressed" << ' ' << preferences->optionWhenCtrlIsPressed() << '\n'
               << "MinimumCellLength" << ' ' << preferences->optionMinimumCellLength() << '\n'
               << "MaximumCellLength" << ' ' << preferences->optionMaximumCellLength() << '\n'
               << "FacesReaction" << ' ' << preferences->optionFacesReaction() << '\n'
               << "ProgressBar" << ' ' << (preferences->optionProgressBar() ? "On" : "Off") << '\n'
               << "MinefieldGenerationAnimation" << ' ' << preferences->optionMinefieldGenerationAnimationString() << '\n'
               << "AskToSaveMatchScoreBehaviour" << ' ' << (uchar)preferences->optionAskToSaveMatchScoreBehaviour() << '\n';
    }

    {
        QScopedPointer<QFile> fileLanguage( new QFile(destDir.absoluteFilePath("libreminesDefaultLanguage.txt")) );
        fileLanguage->open(QIODevice::WriteOnly);

        QTextStream stream(fileLanguage.get());
        stream << preferences-> optionsLanguage();
    }
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
    controller.keyCenterCell = keys.at(6);

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
#ifdef Q_OS_WINDOWS
    {
        preferences->setOptionUsername(qgetenv("USERNAME"));
    }
#else
    {
        preferences->setOptionUsername(qgetenv("USER"));
    }
#endif
}
