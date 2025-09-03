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


#ifndef LIBREMINESGUI_H
#define LIBREMINESGUI_H

#include <QSpinBox>
#include <QTimer>
#include <QLCDNumber>
#include <QCheckBox>
#include <QMainWindow>
#include <QGridLayout>
#include <QScrollArea>
#include <QProgressBar>
#include <QVBoxLayout>

#include "common.h"
#include "libreminespreferencesdialog.h"
#include "libreminesgameengine.h"
#include "qlabel_adapted.h"
#include "qpushbutton_adapted.h"
#include "minefieldtheme.h"
#include "soundeffects.h"
#include "facesreaction.h"
#include "keyboardcontroller.h"

/**
 * @file libreminesgui.h
 * @brief Main GUI class for the LibreMines application
 * @author Bruno Bollos Correa
 * @date 2020-12-12
 * 
 * This file defines the LibreMinesGui class which serves as the main window
 * and primary user interface for the LibreMines minesweeper game. It manages
 * the complete visual interface including the game grid, menus, dialogs,
 * and all user interactions with the game.
 */

/**
 * @brief Main window and user interface for LibreMines
 * 
 * LibreMinesGui is the central GUI class that manages the complete user
 * interface for the LibreMines minesweeper game. It coordinates between
 * the game engine, visual themes, user preferences, and all interactive
 * elements to provide a comprehensive gaming experience.
 * 
 * Key responsibilities include:
 * - Managing the minefield grid display and interaction
 * - Handling menu systems and dialog integration
 * - Coordinating with the game engine for game logic
 * - Managing themes, sounds, and visual effects
 * - Providing keyboard accessibility through KeyboardController
 * - Score management and high score displays
 * - User preferences and settings integration
 * 
 * The class uses a combination of adapted Qt widgets and custom components
 * to create an interactive minesweeper interface that supports both mouse
 * and keyboard navigation, multiple themes, and comprehensive accessibility
 * features.
 * 
 * @since v1.0
 */
class LibreMinesGui : public QMainWindow
{
    Q_OBJECT

    friend class KeyboardController;
private:
    /**
     * @brief GUI representation of a single minefield cell
     * 
     * CellGui encapsulates the visual components needed to display and
     * interact with a single cell in the minesweeper grid. Each cell
     * contains both a button (for hidden state) and a label (for revealed
     * state), allowing smooth transitions between game states.
     * 
     * The class manages the dual-widget approach where buttons represent
     * hidden cells that can be clicked to reveal, and labels represent
     * revealed cells that can be used for chord clicking operations.
     * 
     * @since v1.0
     */
    class CellGui
    {
    public:
        /**
         * @brief Default constructor for cell GUI components
         * 
         * Initializes the cell with null button and label pointers.
         * The actual widgets are created and assigned by the parent
         * LibreMinesGui class during grid setup.
         */
        CellGui();

        QPushButton_adapted *button; ///< Button widget for hidden cell state (clickable for revealing)
        QLabel_adapted *label;       ///< Label widget for revealed cell state (displays value, clickable for chording)
    };

public:
    /**
     * @brief
     *
     * @param parent
     */
    LibreMinesGui(QWidget *parent = nullptr, const int thatWidth = -1, const int thatHeight = -1);
    /**
     * @brief
     *
     */
    ~LibreMinesGui();

    /**
     * @brief
     *
     * @param _X
     * @param _Y
     * @param i_nMines_
     * @param i_X_Clean
     * @param i_Y_Clean
     */

protected:
    bool eventFilter(QObject* object, QEvent* event);

    void resizeEvent(QResizeEvent* e);

    void closeEvent(QCloseEvent* e);
private:
    void vNewGame(const uchar _X,
                  const uchar _Y,
                  short i_nMines_);

    void vAttributeAllCells();
    void vResetPrincipalMatrix();
    /**
     * @brief
     *
     */
    void vCreateGUI(const int width, const int height);
    /**
     * @brief
     *
     */
    void vHideMainMenu();
    /**
     * @brief
     *
     */
    void vShowMainMenu();
    /**
     * @brief
     *
     */
    void vAdjustMainMenu();
    void vAdjustInterfaceInGame();
    /**
     * @brief
     *
     */
    void vHideInterfaceInGame();
    /**
     * @brief
     *
     */
    void vShowInterfaceInGame();

    void vSetApplicationTheme(const QString& theme);

    void vUpdatePreferences();

private Q_SLOTS:
    /**
     * @brief
     *
     */
    void SLOT_Easy();
    /**
     * @brief
     *
     */
    void SLOT_Medium();
    /**
     * @brief
     *
     */
    void SLOT_Hard();
    /**
     * @brief
     *
     */
    void SLOT_Customized();

    /**
     * @brief
     *
     */
    void SLOT_QuitGame();
    /**
     * @brief
     *
     */
    void SLOT_RestartGame();

    /**
     * @brief
     *
     * @param e
     */
    void SLOT_OnCellButtonReleased(const QMouseEvent*const e);
    void SLOT_OnCellButtonClicked(const QMouseEvent*const e);

    void SLOT_onCellLabelReleased(const QMouseEvent*const e);
    void SLOT_onCellLabelClicked(const QMouseEvent*const e);

    void SLOT_showCell(const uchar _X, const uchar _Y, const bool recursive);
    void SLOT_endGameScore(LibreMinesScore score,
                           int iCorrectFlags,
                           int iWrongFlags,
                           int iUnlockedCells,
                           double dFlagsPerSecond,
                           double dCellsPerSecond,
                           bool ignorePreferences=false);
    void SLOT_currentTime(const ushort time);
    void SLOT_minesLeft(const short minesLeft);
    void SLOT_flagCell(const uchar _X, const uchar _Y);
    void SLOT_QuestionCell(const uchar _X, const uchar _Y);
    void SLOT_unflagCell(const uchar _X, const uchar _Y);

    void SLOT_remakeGame();
    void SLOT_gameWon();
    void SLOT_gameLost(const uchar _X, const uchar _Y);

    void SLOT_optionChanged(const QString& name, const QString& value);

    void SLOT_quitApplication();

    void SLOT_showAboutDialog();

    void SLOT_showHighScores();

    void SLOT_importHighScores();

    void SLOT_exportHighScores();

    void SLOT_toggleFullScreen();

    void SLOT_saveMinefieldAsImage();

Q_SIGNALS:
    void SIGNAL_cleanCell(const uchar _X, const uchar _Y);
    void SIGNAL_cleanNeighborCells(const uchar _X, const uchar _Y);
    void SIGNAL_addOrRemoveFlag(const uchar _X, const uchar _Y);
    void SIGNAL_stopGame();
    void SIGNAL_playSoundEffect(SoundEffects::SoundType t);
    void SIGNAL_setSoundEffectVolume(const int v);

private:

    QScopedPointer<LibreMinesGameEngine> gameEngine;
    std::vector< std::vector<CellGui> > principalMatrix; /**< TODO: describe */

    int iLimitHeightField; /**< TODO: describe */
    int iLimitWidthField; /**< TODO: describe */

    int cellLength; /**< TODO: describe */

    GameDifficulty difficult; /**< TODO: describe */

    QPushButton *buttonEasy; /**< TODO: describe */
    QPushButton *buttonMedium; /**< TODO: describe */
    QPushButton *buttonHard; /**< TODO: describe */

    QPushButton *buttonCustomizedNewGame; /**< TODO: describe */

    QSpinBox *sbCustomizedX; /**< TODO: describe */
    QSpinBox *sbCustomizedY; /**< TODO: describe */
    QSpinBox *sbCustomizedPercentageMines; /**< TODO: describe */
    QSpinBox* sbCustomizedNumbersOfMines;

    QLabel *labelCustomizedX; /**< TODO: describe */
    QLabel *labelCustomizedY; /**< TODO: describe */
    QLabel *labelCustomizedPercentageMines; /**< TODO: describe */
    QLabel* labelCustomizedNumbersOfMines;

    QCheckBox* cbCustomizedMinesInPercentage;

    QLabel* labelFaceReactionInGame;
    QLabel *labelTimerInGame; /**< TODO: describe */
    QLCDNumber *lcd_numberMinesLeft; /**< TODO: describe */
    QProgressBar* progressBarGameCompleteInGame;
    QPushButton *buttonRestartInGame; /**< TODO: describe */
    QPushButton *buttonQuitInGame; /**< TODO: describe */
    QPushButton* buttonSaveMinefieldAsImage;
    QPushButton* buttonSaveScore;

    QScrollArea* scrollAreaBoard;
    QWidget* widgetBoardContents;
    QGridLayout* layoutBoard;

    QScrollArea* scrollAreaEndGameResults;
    QWidget* widgetEndGameResultsContents;
    QVBoxLayout* layoutEndGameResults;
    QLabel *labelYouWonYouLost; /**< TODO: describe */
    QLabel *labelStatsLastMatch; /**< TODO: describe */

    QMenu* menuOptions;
    QAction* actionPreferences;
    QAction* actionToggleFullScreen;

    QMenu* menuHighScores;
    QAction* actionShowHighScores;
    QAction* actionImportHighScores;
    QAction* actionExportHighScores;

    QMenu* menuHelp;
    QAction* actionAbout;
    QAction* actionAboutQt;
    QAction* actionGitHubHomePage;

    MinefieldTheme fieldTheme;
    FacesReaction facesReac;

    KeyboardController controller;

    LibreMinesPreferencesDialog* preferences;

    QDir dirAppData;

    QScopedPointer<SoundEffects> sound;

    bool bMinefieldBeingCreated;
};
#endif // LIBREMINESGUI_H
