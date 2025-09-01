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
#include "libreminesapptheme.h"
#include "libreminespreferencessaver.h"

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
    difficult(GameDifficulty::NONE ),
    preferences( new LibreMinesPreferencesDialog(this) ),
    dirAppData( QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) ),
    sound( new SoundEffects() )
{
    // this->resize(800, 600);
    this->setMinimumSize(QSize(700, 500));

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
    controller = KeyboardController();

    // Load configuration file and set the theme
    LibreMinesPreferencesSaver::vLastSessionLoadConfigurationFile
    (
        *preferences, dirAppData,
        *sbCustomizedPercentageMines, *sbCustomizedX, *sbCustomizedY,
        *cbCustomizedMinesInPercentage
    );
    vUpdatePreferences();
    fieldTheme.vSetMinefieldTheme(preferences->optionMinefieldTheme(), 0);
    facesReac.vSetFacesReactionTheme(preferences->optionFacesReaction(), 0);

    // Necessary for some reason
    QTimer::singleShot(100, [this]()
    { vSetApplicationTheme(preferences->optionApplicationStyle()); });

    bMinefieldBeingCreated = false;

    connect(this, &LibreMinesGui::SIGNAL_setSoundEffectVolume,
            sound.get(), &SoundEffects::SLOT_setVolume);
    connect(this, &LibreMinesGui::SIGNAL_playSoundEffect,
            sound.get(), &SoundEffects::SLOT_playSound);
}

LibreMinesGui::~LibreMinesGui()
{
    vResetPrincipalMatrix();

    // save the current user preferences into a specific file
    LibreMinesPreferencesSaver::vLastSessionSaveConfigurationFile
    (
        *preferences, dirAppData,
        *sbCustomizedPercentageMines, *sbCustomizedX, *sbCustomizedY,
        *cbCustomizedMinesInPercentage
    );
}

bool LibreMinesGui::eventFilter(QObject* object, QEvent* event)
{
    Q_UNUSED(object)

    // Just handle those two events
    if(event->type() != QEvent::KeyPress &&
       event->type() != QEvent::KeyRelease)
        return false;

    // If the game is not running, do not deal with the event
    if(!gameEngine || !gameEngine->isGameActive())
        return false;

    // If the GUI is not ready
    if(principalMatrix.empty() ||
       !principalMatrix[0][0].button ||
       !principalMatrix[0][0].button->isEnabled())
        return false;

    return controller.handleEvent(event, this);
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
                             short i_nMines_)
{
    bMinefieldBeingCreated = true;
    
    vAdjustInterfaceInGame();
    vShowInterfaceInGame();

    // Reset the controller attributes
    controller = KeyboardController();
    controller.setKeys(preferences->optionKeyboardControllerKeys());

    qApp->restoreOverrideCursor();

    if(!controller.isValid())
    {
        QMessageBox::warning(this, tr("Keyboard Controller is invalid"),
                             tr("Dear user, your Keyboard Controller preferences"
                             " are invalid. Therefore, you will not be able to play with the keyboard.\n"
                             "To fix it, go to (Main Menu > Options > Preferences) and edit your preferences."));
    }

    // Create a new matrix
    principalMatrix = std::vector<std::vector<CellGui>> (_X, std::vector<CellGui>(_Y));

    buttonQuitInGame->setEnabled(false);
    buttonRestartInGame->setEnabled(false);
    buttonSaveMinefieldAsImage->setEnabled(false);
    buttonSaveScore->hide();
    scrollAreaEndGameResults->hide();

    // Create the game engine instance
    gameEngine.reset(new LibreMinesGameEngine());

    gameEngine->setFirstCellClean(preferences->optionFirstCellClean());
    gameEngine->setUseQuestionMark(preferences->optionUseQuestionMark());
    gameEngine->vNewGame(_X, _Y, i_nMines_);

    // Set the length of each cell
    cellLength = qMin(iLimitWidthField/_X, iLimitHeightField/_Y);

    if(cellLength < preferences->optionMinimumCellLength())
        cellLength = preferences->optionMinimumCellLength();
    else if(cellLength > preferences->optionMaximumCellLength())
        cellLength = preferences->optionMaximumCellLength();


    // Update the pixmaps
    fieldTheme.vSetMinefieldTheme(preferences->optionMinefieldTheme(), cellLength);
    // Update faces reaction
    facesReac.vSetFacesReactionTheme(preferences->optionFacesReaction(),
                                              labelFaceReactionInGame->width());

    widgetBoardContents->setGeometry(0, 0, _X*cellLength, _Y*cellLength);

    labelFaceReactionInGame->setPixmap(facesReac.getPixmapFromGameEvent(FacesReaction::GAME_BEING_GENERATED));


    const bool bCleanNeighborCellsWhenClickedOnShowedLabel =
            preferences->optionCleanNeighborCellsWhenClickedOnShowedCell();

    qApp->processEvents();

    // Create each cell
    for(uchar j=0; j<_Y; j++)
    {
        for (uchar i=0; i<_X; i++)
        {
            CellGui& cell = principalMatrix[i][j];

//            cell.label = new QLabel_adapted(this);
//            cell.button = new QPushButton_adapted(this);

            cell.label = new QLabel_adapted(widgetBoardContents, i, j);
            cell.button = new QPushButton_adapted(widgetBoardContents, i, j);

            layoutBoard->addWidget(cell.label, j, i);
            layoutBoard->addWidget(cell.button, j, i);

            cell.label->resize(cellLength, cellLength);
            cell.label->setPixmap(fieldTheme.getPixmapFromCellValue(CellValue::ZERO));
            cell.label->setScaledContents(true);
            cell.label->show();

            cell.button->resize(cellLength, cellLength);
            cell.button->setIcon(QIcon(fieldTheme.getPixmapButton(FlagState::NoFlag)));
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
    connect(gameEngine.get(), &LibreMinesGameEngine::SIGNAL_questionCell,
            this, &LibreMinesGui::SLOT_QuestionCell);
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
            gameEngine.get(), &LibreMinesGameEngine::SLOT_changeFlagState);
    connect(this, &LibreMinesGui::SIGNAL_stopGame,
            gameEngine.get(), &LibreMinesGameEngine::SLOT_stop);

    connect(buttonSaveScore, &QPushButton::released,
            gameEngine.get(), &LibreMinesGameEngine::SLOT_generateEndGameScoreAgain);


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

    labelFaceReactionInGame->setPixmap(facesReac.getPixmapFromGameEvent(FacesReaction::DEFAULT));

    bMinefieldBeingCreated = false;

    vAdjustInterfaceInGame();
}

void LibreMinesGui::vAttributeAllCells()
{
    for(uchar j=0; j<gameEngine->lines(); ++j)
    {
        for(uchar i=0; i<gameEngine->rows(); ++i)
        {
            CellGui& cell = principalMatrix[i][j];

            cell.button->setEnabled(true);

            cell.label->setPixmap(fieldTheme.getPixmapFromCellValue
                                  (gameEngine->getPrincipalMatrix()[i][j].value));

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
    actionShowHighScores = new QAction(this);
    actionImportHighScores = new QAction(this);
    actionExportHighScores = new QAction(this);
    actionToggleFullScreen = new QAction(this);
    actionAbout = new QAction(this);
    actionAboutQt = new QAction(this);
    actionGitHubHomePage = new QAction(this);

    QMenuBar* menuBarGlobal = new QMenuBar(this);

    menuBarGlobal->setGeometry(0, 0, this->width(), 100);

    menuOptions = new QMenu(menuBarGlobal);
    menuHighScores = new QMenu(menuBarGlobal);
    menuHelp = new QMenu(menuBarGlobal);

    this->setMenuBar(menuBarGlobal);
    this->setStatusBar(new QStatusBar(this));

    menuBarGlobal->addAction(menuOptions->menuAction());
    menuBarGlobal->addAction(menuHighScores->menuAction());
    menuBarGlobal->addAction(menuHelp->menuAction());

    menuOptions->addActions({actionPreferences, actionToggleFullScreen});
    menuHighScores->addActions({actionShowHighScores, actionImportHighScores, actionExportHighScores});
    menuHelp->addActions({actionAbout, actionAboutQt, actionGitHubHomePage});

    menuOptions->setTitle(tr("Options"));
    menuHighScores->setTitle(tr("High Scores"));
    menuHelp->setTitle(tr("Help"));

    actionPreferences->setText(tr("Preferences..."));
    actionToggleFullScreen->setText(tr("Toggle Full Screen"));
    actionToggleFullScreen->setShortcut(QKeySequence(Qt::Key_F11));

    actionShowHighScores->setText(tr("Show High Scores..."));
    actionImportHighScores->setText(tr("Import High Scores..."));
    actionExportHighScores->setText(tr("Export High Scores..."));

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
    buttonSaveScore = new QPushButton(centralWidget());

    scrollAreaBoard = new QScrollArea(centralWidget());
    widgetBoardContents = new QWidget();    
    layoutBoard = new QGridLayout();
    layoutBoard->setSpacing(0);

    scrollAreaEndGameResults = new QScrollArea(centralWidget());
    widgetEndGameResultsContents = new QWidget();
    layoutEndGameResults = new QVBoxLayout();

    widgetBoardContents->setLayout(layoutBoard);
    widgetBoardContents->setFocusPolicy(Qt::NoFocus);
    scrollAreaBoard->setWidget(widgetBoardContents);
    scrollAreaBoard->setFocusPolicy(Qt::NoFocus);

    widgetEndGameResultsContents->setLayout(layoutEndGameResults);
    widgetEndGameResultsContents->setFocusPolicy(Qt::NoFocus);
    scrollAreaEndGameResults->setWidget(widgetEndGameResultsContents);
    scrollAreaEndGameResults->setFocusPolicy(Qt::NoFocus);
    scrollAreaEndGameResults->setStyleSheet("QScrollArea { background: transparent; }");

    labelYouWonYouLost = new QLabel(widgetEndGameResultsContents);
    labelStatsLastMatch = new QLabel(widgetEndGameResultsContents);

    layoutEndGameResults->addWidget(labelYouWonYouLost);
    layoutEndGameResults->addWidget(labelStatsLastMatch);

    labelTimerInGame->setFont(QFont("Liberation Sans", 40));
    labelTimerInGame->setNum(0);
    lcd_numberMinesLeft->setDecMode();
    lcd_numberMinesLeft->display(0);
    labelFaceReactionInGame->setScaledContents(true);
    progressBarGameCompleteInGame->setTextVisible(false);
    buttonRestartInGame->setText(tr("Restart"));
    buttonRestartInGame->setMinimumSize(0, 35);
    buttonQuitInGame->setText(tr("Quit"));
    buttonQuitInGame->setMinimumSize(0, 35);
    buttonSaveMinefieldAsImage->setText(tr("Save Minefield as Image"));
    buttonSaveMinefieldAsImage->setMinimumSize(0, 35);
    buttonSaveScore->setText(tr("Save Score"));
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
    sbCustomizedX->setMinimum(5);
    sbCustomizedX->setMaximum(100);
    sbCustomizedX->setValue(20);

    sbCustomizedY = new QSpinBox(centralWidget());
    sbCustomizedY->setMinimum(5);
    sbCustomizedY->setMaximum(100);
    sbCustomizedY->setValue(20);


    sbCustomizedPercentageMines = new QSpinBox(centralWidget());
    sbCustomizedPercentageMines->setMinimum(0);
    sbCustomizedPercentageMines->setMaximum(100);
    sbCustomizedPercentageMines->setValue(20);
    sbCustomizedPercentageMines->setSuffix("%");

    sbCustomizedNumbersOfMines = new QSpinBox(centralWidget());
    sbCustomizedNumbersOfMines->setMinimum(0);
    sbCustomizedNumbersOfMines->setMaximum(sbCustomizedX->value() * sbCustomizedY->value());
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

    connect(actionShowHighScores, &QAction::triggered,
            this, &LibreMinesGui::SLOT_showHighScores);

    connect(actionImportHighScores, &QAction::triggered,
            this, &LibreMinesGui::SLOT_importHighScores);

    connect(actionExportHighScores, &QAction::triggered,
            this, &LibreMinesGui::SLOT_exportHighScores);

    connect(actionToggleFullScreen, &QAction::triggered,
            this, &LibreMinesGui::SLOT_toggleFullScreen);


    connect(actionAbout, &QAction::triggered,
            this, &LibreMinesGui::SLOT_showAboutDialog);

    connect(actionAboutQt, &QAction::triggered,
            [this](){ QMessageBox::aboutQt(this, "About Qt"); });

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

    // lambda function
    auto updateCustomizedNumberOfMinesMaximum
    {
        [this]()
        {
            sbCustomizedNumbersOfMines->setMaximum(sbCustomizedX->value() * sbCustomizedY->value());
            sbCustomizedNumbersOfMines->setValue(sbCustomizedX->value() * sbCustomizedY->value() * sbCustomizedPercentageMines->value() / 100);
            sbCustomizedNumbersOfMines->setSuffix(
                " / " +QString::number(sbCustomizedX->value()*sbCustomizedY->value()));
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
    setTabOrder(buttonSaveMinefieldAsImage, buttonSaveScore);

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

    // cbCustomizedMinesInPercentage->setStyleSheet(
    //             "QCheckBox::indicator "
    //             "{"
    //             "    width: " + QString::number(cbCustomizedMinesInPercentage->width()) + "px;"
    //             "    height: " + QString::number(cbCustomizedMinesInPercentage->width()) + "px;"
    //             "}");
}

void LibreMinesGui::SLOT_Easy()
{
    vHideMainMenu();
    vNewGame(8, 8, 10);

    difficult = GameDifficulty::EASY;
}

void LibreMinesGui::SLOT_Medium()
{
    vHideMainMenu();
    vNewGame(16, 16, 40);

    difficult = GameDifficulty::MEDIUM;

}

void LibreMinesGui::SLOT_Hard()
{
    vHideMainMenu();
    vNewGame(30, 16, 99);

    difficult = GameDifficulty::HARD;

}

void LibreMinesGui::SLOT_Customized()
{
    vHideMainMenu();
    vNewGame(sbCustomizedX->value(), sbCustomizedY->value(), sbCustomizedNumbersOfMines->value());

    difficult = GameDifficulty::CUSTOMIZED;

}

void LibreMinesGui::vAdjustInterfaceInGame()
{
    const int w = centralWidget()->width();
    const int h = centralWidget()->height();


    iLimitWidthField = 8*w/10;
    iLimitHeightField = h;

    scrollAreaBoard->setGeometry(0, 0, iLimitWidthField, iLimitHeightField);

    labelFaceReactionInGame->setGeometry(
        85*w /100, h/50, w/10, w/10
        );

    facesReac.vSetFacesReactionTheme(preferences->optionFacesReaction(),
                                     labelFaceReactionInGame->width());

    labelTimerInGame->setGeometry(
        81*w /100, labelFaceReactionInGame->y() + labelFaceReactionInGame->height(),
        18*w /100, h /14
        );
    lcd_numberMinesLeft->setGeometry(
        labelTimerInGame->x(), labelTimerInGame->y()+labelTimerInGame->height(),
        labelTimerInGame->width(), h /7
        );
    progressBarGameCompleteInGame->setGeometry(
        lcd_numberMinesLeft->x(), lcd_numberMinesLeft->y()+lcd_numberMinesLeft->height(),
        lcd_numberMinesLeft->width(), h /20
        );
    buttonRestartInGame->setGeometry(
        progressBarGameCompleteInGame->x(),
        progressBarGameCompleteInGame->y()+progressBarGameCompleteInGame->height() + h/200,
        progressBarGameCompleteInGame->width()/2 - h/400, h/20
        );
    buttonQuitInGame->setGeometry(
        buttonRestartInGame->x() + buttonRestartInGame->width() + h/200, buttonRestartInGame->y(),
        buttonRestartInGame->width(), buttonRestartInGame->height()
        );

    if(progressBarGameCompleteInGame->width() > 200)
    {
        buttonSaveMinefieldAsImage->setGeometry(
            buttonRestartInGame->x(),
            buttonRestartInGame->y() + buttonRestartInGame->height() + h/200,
            progressBarGameCompleteInGame->width(), progressBarGameCompleteInGame->height()
            );
        buttonSaveMinefieldAsImage->setText(tr("Save Minefield as Image"));
    }
    else
    {
        buttonSaveMinefieldAsImage->setGeometry(
            buttonRestartInGame->x(),
            buttonRestartInGame->y() + buttonRestartInGame->height() + h/200,
            progressBarGameCompleteInGame->width(), 2*progressBarGameCompleteInGame->height()
            );
        buttonSaveMinefieldAsImage->setText(tr("Save Minefield\nas Image"));
    }

    buttonSaveScore->setGeometry(
        buttonSaveMinefieldAsImage->x(),
        buttonSaveMinefieldAsImage->y() + buttonSaveMinefieldAsImage->height() + h/200,
        buttonSaveMinefieldAsImage->width(), buttonSaveMinefieldAsImage->height()
        );

    scrollAreaEndGameResults->setGeometry(
        QRect(QPoint(buttonSaveScore->x(),
                     buttonSaveScore->y() + buttonSaveScore->height() + h/200),
              QPoint(buttonSaveScore->x() + buttonSaveScore->width(), centralWidget()->rect().bottom()))
        );

    labelYouWonYouLost->adjustSize();
    labelStatsLastMatch->adjustSize();
    widgetEndGameResultsContents->adjustSize();

    // labelYouWonYouLost->setGeometry(
    //     buttonSaveScore->x(), buttonSaveScore->y()+buttonSaveScore->height()*1.1,
    //     lcd_numberMinesLeft->width(), lcd_numberMinesLeft->height()
    //     );
    // labelStatsLastMatch->setGeometry(
    //     labelYouWonYouLost->x(), labelYouWonYouLost->y() + labelYouWonYouLost->height(),
    //     labelYouWonYouLost->width(), h/5
    //     );

    QFont lFont = labelTimerInGame->font();
    lFont.setPixelSize(qMin(labelTimerInGame->width(), labelTimerInGame->height()));
    labelTimerInGame->setFont(lFont);


    int _X = principalMatrix.size();
    if(gameEngine.isNull() || _X ==0 || bMinefieldBeingCreated)
        return;

    int _Y = principalMatrix[0].size();
    // Recalculate the length of each cell
    int oldCellLength = cellLength;
    cellLength = qMin((iLimitWidthField-5)/_X, (iLimitHeightField-5)/_Y);

    if(cellLength < preferences->optionMinimumCellLength())
        cellLength = preferences->optionMinimumCellLength();
    else if(cellLength > preferences->optionMaximumCellLength())
        cellLength = preferences->optionMaximumCellLength();

    if(cellLength == oldCellLength)
        return;

    fieldTheme.vSetMinefieldTheme(preferences->optionMinefieldTheme(), cellLength);

    widgetBoardContents->setGeometry(
        0, 0, _X*cellLength, _Y*cellLength);

    for(uchar j=0; j<_Y; j++)
    {
        for (uchar i=0; i<_X; i++)
        {
            CellGui& cell = principalMatrix[i][j];
            const LibreMinesGameEngine::CellGameEngine& cellGE =
                gameEngine->getPrincipalMatrix()[i][j];

            cell.label->resize(cellLength, cellLength);
            cell.label->setPixmap(fieldTheme.getPixmapFromCellValue(cellGE.value));

            cell.button->resize(cellLength, cellLength);
            cell.button->setIcon(QIcon(fieldTheme.getPixmapButton(cellGE.flagState)));
            cell.button->setIconSize(QSize(cellLength, cellLength));
        }
    }
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
    buttonSaveScore->hide();
    labelYouWonYouLost->hide();
    labelStatsLastMatch->hide();
    widgetBoardContents->hide();
    scrollAreaBoard->hide();
    widgetEndGameResultsContents->hide();
    scrollAreaEndGameResults->hide();
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
    labelStatsLastMatch->show();
    widgetBoardContents->show();
    scrollAreaBoard->show();
    widgetEndGameResultsContents->show();
    scrollAreaEndGameResults->show();
}

void LibreMinesGui::vSetApplicationTheme(const QString& theme)
{
    if(LibreMinesAppTheme::bSetApplicationTheme(theme))
    {
        // Create a message if a restart is needed
        QMessageBox::information(this, "LibreMines",
                                 "Please reset the application to apply this change");
    }
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

    labelStatsLastMatch->setText(" ");
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

    labelStatsLastMatch->setText(" ");

    Q_EMIT SIGNAL_stopGame();

    vResetPrincipalMatrix();
    vHideInterfaceInGame();
    vShowMainMenu();

    difficult = GameDifficulty::NONE;

    gameEngine.reset();
}

void LibreMinesGui::SLOT_OnCellButtonReleased(const QMouseEvent *const e)
{
    if(!gameEngine->isGameActive() || controller.isActive())
        return;

    labelFaceReactionInGame->setPixmap(facesReac.getPixmapFromGameEvent(FacesReaction::DEFAULT));

    // if the button is released outside its area do not treat the event
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if(e->position().x() >= cellLength || e->position().x() < 0 ||
        e->position().y() >= cellLength || e->position().y() < 0)
#else
    if(e->localPos().x() >= cellLength || e->localPos().x() < 0 ||
       e->localPos().y() >= cellLength || e->localPos().y() < 0)
#endif
    {
        return;
    }

    QPushButton_adapted *buttonClicked = (QPushButton_adapted *) sender();

    switch (e->button())
    {
        case Qt::RightButton:
            Q_EMIT SIGNAL_addOrRemoveFlag(buttonClicked->getXCell(), buttonClicked->getYCell());
            return;

        case Qt::LeftButton:
            Q_EMIT SIGNAL_cleanCell(buttonClicked->getXCell(), buttonClicked->getYCell());
            return;

        default:
            return;
    }
}

void LibreMinesGui::SLOT_OnCellButtonClicked(const QMouseEvent *const e)
{
    if(!gameEngine->isGameActive() || controller.isActive())
        return;

    if(e->button() != Qt::LeftButton)
        return;

    labelFaceReactionInGame->setPixmap(facesReac.getPixmapFromGameEvent(FacesReaction::HIDDEN_CELL_PRESSED));
}

void LibreMinesGui::SLOT_onCellLabelReleased(const QMouseEvent *const e)
{
    if(!gameEngine->isGameActive() || controller.isActive())
        return;

    labelFaceReactionInGame->setPixmap(facesReac.getPixmapFromGameEvent(FacesReaction::DEFAULT));

    // if the button is released outside its area do not treat the event
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if(e->position().x() >= cellLength || e->position().x() < 0 ||
        e->position().y() >= cellLength || e->position().y() < 0)
#else
    if(e->localPos().x() >= cellLength || e->localPos().x() < 0 ||
       e->localPos().y() >= cellLength || e->localPos().y() < 0)
#endif
    {
        return;
    }

    QLabel_adapted *buttonClicked = (QLabel_adapted *) sender();

    switch (e->button())
    {
        case Qt::LeftButton:
            Q_EMIT SIGNAL_cleanNeighborCells(buttonClicked->getXCell(), buttonClicked->getYCell());
            return;

        default:
            return;
    }
}

void LibreMinesGui::SLOT_onCellLabelClicked(const QMouseEvent *const e)
{
    Q_UNUSED(e)

    if(!gameEngine->isGameActive() || controller.isActive())
        return;

    labelFaceReactionInGame->setPixmap(facesReac.getPixmapFromGameEvent(FacesReaction::UNHIDDEN_CELL_PRESSED));

}

void LibreMinesGui::SLOT_showCell(const uchar _X, const uchar _Y, const bool recursive)
{
    principalMatrix[_X][_Y].button->hide();

    controller.refresh(_X, _Y, this);

    if(!recursive)
    {
        Q_EMIT SIGNAL_playSoundEffect(SoundEffects::RELEASE_CELL);
    }
}

void LibreMinesGui::SLOT_endGameScore(LibreMinesScore score,
                                      int iCorrectFlags,
                                      int iWrongFlags,
                                      int iUnlockedCells,
                                      double dFlagsPerSecond,
                                      double dCellsPerSecond,
                                      bool ignorePreferences)
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

    labelStatsLastMatch->setText(QS_Statics);


    labelYouWonYouLost->adjustSize();
    labelStatsLastMatch->adjustSize();
    widgetEndGameResultsContents->adjustSize();
    scrollAreaEndGameResults->show();

    // Update the timer label to avoid this being shown different from statistics time (#83).
    labelTimerInGame->setNum(static_cast<int>(score.iTimeInNs*1e-9));

    score.gameDifficulty = difficult;
    score.username = preferences->optionUsername();
    if(score.username.isEmpty())
    {
#ifdef Q_OS_WINDOWS
        score.username = qgetenv("USERNAME");
#else
        score.username = qgetenv("USER");
#endif
    }

    // Save the score of the current game on the file scoresLibreMines on
    //  the "~/.local/share/libremines/" directory. If the file does not
    //  exist, a new one will be created.
    if(score.dPercentageGameCompleted != 0)
    {
        QScopedPointer<QFile> fileScores( new QFile(dirAppData.absoluteFilePath("scoresLibreMines")) );

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
            if(score.gameDifficulty == GameDifficulty::EASY)
                strGameDiffuclty = tr("Easy");
            else if(score.gameDifficulty == GameDifficulty::MEDIUM)
                strGameDiffuclty = tr("Medium");
            else if(score.gameDifficulty == GameDifficulty::HARD)
                strGameDiffuclty = tr("Hard");
            else if(score.gameDifficulty == GameDifficulty::CUSTOMIZED)
            {
                strGameDiffuclty = tr("Customized ") + QString::number(score.width) +
                                   " x " + QString::number(score.heigth) + " : " +
                                   QString::number(score.mines);
            }

            AskToSaveMatchScore behaviour = preferences->optionAskToSaveMatchScoreBehaviour();

            if(ignorePreferences ||
               behaviour == LibreMines::SaveAlways ||
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

            fileScores.reset( new QFile(dirAppData.absoluteFilePath("scoresLibreMines")) );

            if(fileScores->exists())
                fileScores->open(QIODevice::Append);
            else
                fileScores->open(QIODevice::WriteOnly);

            QDataStream stream(fileScores.get());

            stream.setVersion(QDataStream::Qt_5_12);

            stream << score;
        }

        buttonSaveScore->setVisible(!saveScore);
    }

}

void LibreMinesGui::SLOT_currentTime(const ushort time)
{
    labelTimerInGame->setNum(time);

//    Q_EMIT(sound->SIGNAL_clockTick());
}

void LibreMinesGui::SLOT_minesLeft(const short minesLeft)
{
    lcd_numberMinesLeft->display(minesLeft);
}

void LibreMinesGui::SLOT_flagCell(const uchar _X, const uchar _Y)
{
    if(principalMatrix[_X][_Y].button->isHidden())
        qDebug(Q_FUNC_INFO);
    else
    {
        principalMatrix[_X][_Y].button->setIcon(QIcon(fieldTheme.getPixmapButton(FlagState::HasFlag)));
        principalMatrix[_X][_Y].button->setIconSize(QSize(cellLength, cellLength));
    }

    controller.refresh(_X, _Y, this);

    Q_EMIT SIGNAL_playSoundEffect(SoundEffects::FLAG_CELL);
}


void LibreMinesGui::SLOT_QuestionCell(const uchar _X, const uchar _Y)
{
    if(principalMatrix[_X][_Y].button->isHidden())
        qDebug(Q_FUNC_INFO);
    else
    {
        principalMatrix[_X][_Y].button->setIcon(QIcon(fieldTheme.getPixmapQuestion()));
        principalMatrix[_X][_Y].button->setIconSize(QSize(cellLength, cellLength));
    }

    controller.refresh(_X, _Y, this);

    Q_EMIT SIGNAL_playSoundEffect(SoundEffects::FLAG_CELL);
}

void LibreMinesGui::SLOT_unflagCell(const uchar _X, const uchar _Y)
{
    if(principalMatrix[_X][_Y].button->isHidden())
        qDebug(Q_FUNC_INFO);
    else
    {
        principalMatrix[_X][_Y].button->setIcon(QIcon(fieldTheme.getPixmapButton(FlagState::NoFlag)));
        principalMatrix[_X][_Y].button->setIconSize(QSize(cellLength, cellLength));
    }

    controller.refresh(_X, _Y, this);

    Q_EMIT SIGNAL_playSoundEffect(SoundEffects::FLAG_CELL);
}

void LibreMinesGui::SLOT_remakeGame()
{
    vAttributeAllCells();
}

void LibreMinesGui::SLOT_gameWon()
{
    switch (difficult)
    {
        case GameDifficulty::NONE:
            break;
        case GameDifficulty::EASY:
            labelYouWonYouLost->setText(tr("You Win!") + '\n' + tr("Difficulty: EASY"));
            break;
        case GameDifficulty::MEDIUM:
            labelYouWonYouLost->setText(tr("You Win!") + '\n' + tr("Difficulty: MEDIUM"));
            break;
        case GameDifficulty::HARD:
            labelYouWonYouLost->setText(tr("You Win!") + '\n' + tr("Difficulty: HARD"));
            break;
        case GameDifficulty::CUSTOMIZED:
            labelYouWonYouLost->setText(tr("You Win!") +'\n' + tr("Difficulty: CUSTOM\n") +
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

    controller.deactivate(this);

    labelFaceReactionInGame->setPixmap(facesReac.getPixmapFromGameEvent(FacesReaction::GAME_WON));

    Q_EMIT SIGNAL_playSoundEffect(SoundEffects::GAME_WON);
}

void LibreMinesGui::SLOT_gameLost(const uchar _X, const uchar _Y)
{
    switch (difficult)
    {
        case GameDifficulty::NONE:
            break;
        case GameDifficulty::EASY:
            labelYouWonYouLost->setText(tr("You Lost") + '\n' + tr("Difficulty: EASY"));
            break;
        case GameDifficulty::MEDIUM:
            labelYouWonYouLost->setText(tr("You Lost") + '\n' + tr("Difficulty: MEDIUM"));
            break;
        case GameDifficulty::HARD:
            labelYouWonYouLost->setText(tr("You Lost") + '\n' + tr("Difficulty: HARD"));
            break;
        case GameDifficulty::CUSTOMIZED:
            labelYouWonYouLost->setText(tr("You Lost") + '\n' + tr("Difficulty: CUSTOM\n") +
                                        QString::number(gameEngine->rows()) +
                                        "x" +
                                        QString::number(gameEngine->lines()) +
                                        " : " +
                                        QString::number(gameEngine->mines()) +
                                        tr(" Mines"));
    }


    principalMatrix[_X][_Y].label->setPixmap(fieldTheme.getPixmapBoom());

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
                if(cellGE.value == CellValue::MINE &&
                   cellGE.flagState == FlagState::NoFlag)
                {
                    cellGui.button->hide();
                }
                else if (cellGE.value != CellValue::MINE &&
                         cellGE.flagState != FlagState::NoFlag)
                {
                    cellGui.button->hide();
                    cellGui.label->setPixmap(fieldTheme.getPixmapWrongFlag());
                }
            }
        }
    }

    controller.deactivate(this);

    labelFaceReactionInGame->setPixmap(facesReac.getPixmapFromGameEvent(FacesReaction::GAME_LOST));
    Q_EMIT SIGNAL_playSoundEffect(SoundEffects::GAME_LOST);
}

void LibreMinesGui::SLOT_optionChanged(const QString &name, const QString &value)
{
    if(name.compare("ApplicationTheme", Qt::CaseInsensitive) == 0)
    {
        vSetApplicationTheme(value);
    }
    else if(name.compare("MinefieldTheme", Qt::CaseInsensitive) == 0)
    {
       fieldTheme.vSetMinefieldTheme(value, cellLength);
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
            tr("Copyright (C) 2020-2025  Bruno Bollos Correa\n"
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

    QMessageBox::about(this, "About LibreMines", text);
}

void LibreMinesGui::SLOT_showHighScores()
{
    QScopedPointer<QFile> fileScores( new QFile(dirAppData.absoluteFilePath("scoresLibreMines")) );

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

void LibreMinesGui::SLOT_importHighScores()
{
    QString importFileName = QFileDialog::getOpenFileName(
        this, tr("Import high scores from file"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
    );

    if(!importFileName.isEmpty())
    {
        QScopedPointer<QFile> fileImport( new QFile(importFileName) );


        fileImport->open(QIODevice::ReadOnly);

        QDataStream stream(fileImport.get());
        stream.setVersion(QDataStream::Qt_5_12);

        bool fileOk = true;

        while(!stream.atEnd())
        {
            stream.startTransaction();
            LibreMinesScore s;
            stream >> s;
            if(!stream.commitTransaction())
            {
                QMessageBox::critical(this, tr("Invalid high scores file"),
                                      tr("Error parsing the data of the file!"));
                fileOk = false;
                break;
            }
        }

        if(fileOk)
        {
            // Backup the old scores file if it exists
            QString backupScorePath = dirAppData.absoluteFilePath(
                "scoresLibreMines." + QString::number(QDateTime::currentSecsSinceEpoch()) + ".bkp");

            QFile::rename(dirAppData.absoluteFilePath("scoresLibreMines"), backupScorePath);

            if(fileImport->copy(dirAppData.absoluteFilePath("scoresLibreMines")))
            {
                QMessageBox::information(this, tr("High scores import complete"),
                                         tr("Operation completed!") + '\n' +
                                             tr("Old high score file has been saved to") +
                                             backupScorePath);
            }
        }
    }

}

void LibreMinesGui::SLOT_exportHighScores()
{
    QString backupFileName = QFileDialog::getSaveFileName(
        this, tr("Backup high scores to file"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/scoresLibreMines.bkp"
    );

    if(!backupFileName.isEmpty())
    {
        QScopedPointer<QFile> fileScores( new QFile(dirAppData.absoluteFilePath("scoresLibreMines")) );
        if(fileScores && fileScores->exists() && fileScores->copy(backupFileName))
        {
            QMessageBox::information(this, tr("High scores backup complete"),
                                    tr("Successfully backup high scores to \"") + backupFileName + '\"');
        }

    }
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
    if(controller.isActive())
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

    if(controller.isActive())
        qApp->setOverrideCursor(QCursor(Qt::BlankCursor));
}

void LibreMinesGui::vUpdatePreferences()
{
    const QList<int> keys = preferences->optionKeyboardControllerKeys();

    controller.setKeys(keys);

    if(preferences->optionUsername().isEmpty())
    {
#ifdef Q_OS_WINDOWS
        preferences->setOptionUsername(qgetenv("USERNAME"));
#else
        preferences->setOptionUsername(qgetenv("USER"));
#endif
    }

    sound->SLOT_setVolume(preferences->optionSoundVolume());
}
