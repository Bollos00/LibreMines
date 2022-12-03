/*****************************************************************************
 * LibreMines                                                                *
 * Copyright (C) 2020-2022  Bruno Bollos Correa                              *
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

#include "common.h"
#include "libreminespreferencesdialog.h"
#include "libreminesgameengine.h"
#include "qlabel_adapted.h"
#include "qpushbutton_adapted.h"
#include "minefieldtheme.h"
#include "soundeffects.h"

/**
 * @brief
 *
 */
class LibreMinesGui : public QMainWindow
{
    Q_OBJECT

private:
    class CellGui
    {
    public:
        CellGui();

        QPushButton_adapted *button; /**< TODO: describe */
        QLabel_adapted *label; /**< TODO: describe */
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
                  ushort i_nMines_);

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
    void vSetFacesReaction(const QString& which);

    void vKeyboardControllerSetCurrentCell(const uchar x, const uchar y);
    void vKeyboardControllUnsetCurrentCell();
    void vKeyboardControllerMoveLeft();
    void vKeyboardControllerMoveRight();
    void vKeyboardControllerMoveDown();
    void vKeyboardControllerMoveUp();
    void vKeyboardControllerCenterCurrentCell();

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

    void SLOT_showCell(const uchar _X, const uchar _Y);
    void SLOT_endGameScore(LibreMinesScore score,
                           int iCorrectFlags,
                           int iWrongFlags,
                           int iUnlockedCells,
                           double dFlagsPerSecond,
                           double dCellsPerSecond,
                           bool ignorePreferences=false);
    void SLOT_currentTime(const ushort time);
    void SLOT_minesLeft(const ushort minesLeft);
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

    void SLOT_toggleFullScreen();

    void SLOT_saveMinefieldAsImage();

Q_SIGNALS:
    void SIGNAL_cleanCell(const uchar _X, const uchar _Y);
    void SIGNAL_cleanNeighborCells(const uchar _X, const uchar _Y);
    void SIGNAL_addOrRemoveFlag(const uchar _X, const uchar _Y);
    void SIGNAL_stopGame();

private:

    QScopedPointer<LibreMinesGameEngine> gameEngine;
    std::vector< std::vector<CellGui> > principalMatrix; /**< TODO: describe */

    int iLimitHeightField; /**< TODO: describe */
    int iLimitWidthField; /**< TODO: describe */

    int cellLength; /**< TODO: describe */

    GAME_DIFFICULTY difficult; /**< TODO: describe */

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

    QLabel *labelYouWonYouLost; /**< TODO: describe */
    QLabel *labelStatisLastMatch; /**< TODO: describe */

    QMenu* menuOptions;
    QAction* actionPreferences;
    QAction* actionHighScores;
    QAction* actionToggleFullScreen;

    QMenu* menuHelp;
    QAction* actionAbout;
    QAction* actionAboutQt;
    QAction* actionGitHubHomePage;

    MinefieldTheme fieldTheme;

    QScopedPointer<QPixmap> pmDizzyFace;
    QScopedPointer<QPixmap> pmGrimacingFace;
    QScopedPointer<QPixmap> pmGrinningFace;
    QScopedPointer<QPixmap> pmOpenMouthFace;
    QScopedPointer<QPixmap> pmSleepingFace;
    QScopedPointer<QPixmap> pmSmillingFace;

    KeyboardController controller;

    LibreMinesPreferencesDialog* preferences;

    QDir dirAppData;

    QScopedPointer<SoundEffects> sound;
};
#endif // LIBREMINESGUI_H
