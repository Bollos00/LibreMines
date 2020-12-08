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


#ifndef LIBREMINESGUI_H
#define LIBREMINESGUI_H

#include <QDesktopWidget>
#include <QSpinBox>
#include <QTimer>
#include <QLCDNumber>
#include <QCheckBox>
#include <QMainWindow>

#include "common.h"


/**
 * @brief
 *
 */
class LibreMines : public QMainWindow
{
    Q_OBJECT


public:
    /**
     * @brief
     *
     * @param parent
     */
    LibreMines(QWidget *parent = nullptr);
    /**
     * @brief
     *
     */
    ~LibreMines();

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

private:
    void vNewGame(const uchar _X,
                  const uchar _Y,
                  ushort i_nMines_,
                  const uchar i_X_Clean = 255,
                  const uchar i_Y_Clean = 255);
    /**
     * @brief
     *
     */

    void vResetPrincipalMatrix();
    /**
     * @brief
     *
     */
    void vConfigureInterface();
    /**
     * @brief
     *
     */
    void vHideInterface();
    /**
     * @brief
     *
     */
    void vShowInterface();
    /**
     * @brief
     *
     * @param _X
     * @param _Y
     */
    void vCleanCell(const uchar _X, const uchar _Y);
    /**
     * @brief
     *
     */
    void vAjustInterfaceInGame();
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
    /**
     * @brief
     *
     * @param _X
     * @param _Y
     */
    void vAddOrRemoveFlag(const uchar _X, const uchar _Y);
    /**
     * @brief
     *
     * @param _X
     * @param _Y
     */
    void vGameLost(const uchar _X, const uchar _Y);
    /**
     * @brief
     *
     */
    void vGameWon();
    /**
     * @brief
     *
     */
    void vStartTimer();
    /**
     * @brief
     *
     */
    void vGenerateStatics();

    void vConfigureDarkMode(const bool bDark);

    void vKeyboardControllerSetCurrentCell(const uchar x, const uchar y);
    void vKeyboardControllUnsetCurrentCell();
    void vKeyboardControllerMoveLeft();
    void vKeyboardControllerMoveRight();
    void vKeyboardControllerMoveDown();
    void vKeyboardControllerMoveUp();
    void vKeyboardControllerUpdateCurrentCell();

private slots:
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
    void SLOT_Quit();
    /**
     * @brief
     *
     */
    void SLOT_Restart();

    /**
     * @brief
     *
     */
    void SLOT_UpdateFirstCellClean();

    ///
    /// \brief SLOT_DarkMode
    ///
    void SLOT_DarkMode();

    /**
     * @brief
     *
     */
    void SLOT_UpdateTime();

    /**
     * @brief
     *
     * @param e
     */
    void SLOT_OnButtonClicked(const QMouseEvent* e);

private:

    std::vector< std::vector<Cell> > principalMatrix; /**< TODO: describe */
    int iLimitHeight; /**< TODO: describe */
    int iLimitWidth; /**< TODO: describe */

    uchar iX; /**< TODO: describe */
    uchar iY; /**< TODO: describe */
    ushort nMines; /**< TODO: describe */
    ushort iTimeInSeconds; /**< TODO: describe */
    ushort iMinesLeft; /**< TODO: describe */
    ushort iHiddenCells; /**< TODO: describe */
    int fm; /**< TODO: describe */

    ushort iCellsToUnlock; /**< TODO: describe */

    bool bFirst; /**< TODO: describe */
    bool bFirstCellClean; /**< TODO: describe */

    GAME_DIFFICULTY difficult; /**< TODO: describe */

    QPushButton *buttonEasy; /**< TODO: describe */
    QPushButton *buttonMedium; /**< TODO: describe */
    QPushButton *buttonHard; /**< TODO: describe */

    QPushButton *buttonCustomizedNewGame; /**< TODO: describe */

    QSpinBox *sbCustomizedX; /**< TODO: describe */
    QSpinBox *sbCustomizedY; /**< TODO: describe */
    QSpinBox *sbCustomizednMines; /**< TODO: describe */

    QLabel *labelCustomizedX; /**< TODO: describe */
    QLabel *labelCustomizedY; /**< TODO: describe */
    QLabel *labelCustomized_nMines; /**< TODO: describe */

    QCheckBox *cbFirstCellClean; /**< TODO: describe */
    QCheckBox *cbDarkModeEnabled;

    QTimer *timerTimeInGame; /**< TODO: describe */
    QLabel *labelTimerInGame; /**< TODO: describe */
    QLCDNumber *lcd_numberMinesLeft; /**< TODO: describe */
    QPushButton *buttonRestartInGame; /**< TODO: describe */
    QPushButton *buttonQuitInGame; /**< TODO: describe */

    QLabel *labelYouWonYouLost; /**< TODO: describe */
    QLabel *labelStatisLastMatch; /**< TODO: describe */

    QImage *imgZero; /**< TODO: describe */
    QImage *imgOne; /**< TODO: describe */
    QImage *imgTwo; /**< TODO: describe */
    QImage *imgThree; /**< TODO: describe */
    QImage *imgFour; /**< TODO: describe */
    QImage *imgFive; /**< TODO: describe */
    QImage *imgSix; /**< TODO: describe */
    QImage *imgSeven; /**< TODO: describe */
    QImage *imgEight; /**< TODO: describe */
    QImage *imgFlag; /**< TODO: describe */
    QImage *imgNoFlag; /**< TODO: describe */
    QImage *imgMine; /**< TODO: describe */
    QImage *imgBoom; /**< TODO: describe */
    QImage *imgWrongFlag; /**< TODO: describe */

    KeyboardController controller;
    bool bGameOn;
};
#endif // LIBREMINESGUI_H
