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


#ifndef LIBREMINESGAMEENGINE_H
#define LIBREMINESGAMEENGINE_H

#include <QPointer>
#include <QElapsedTimer>

#include "common.h"
#include "libreminesscore.h"

class LibreMinesGameEngine: public QObject
{
    Q_OBJECT

public:
    class CellGameEngine
    {
    public:

        CellGameEngine();

        CellValue value;
        bool isHidden;
        FlagState flagState;
    };

public:
    LibreMinesGameEngine();
    void vNewGame(const uchar _X,
                  const uchar _Y,
                  short i_nMines_,
                  const uchar i_X_Clean = 255,
                  const uchar i_Y_Clean = 255);

    const std::vector< std::vector<CellGameEngine> >& getPrincipalMatrix()const;
    uchar rows()const;
    uchar lines()const;
    short mines()const;
    ushort cellsToUnlock()const;
    ushort hiddenCells()const;
    bool isGameActive()const;

    void setFirstCellClean(const bool x);
    void setUseQuestionMark(const bool x);

private:
    void vResetPrincipalMatrix();

    /**
     * @brief Clear the cell in the #_X, #_Y position.
     * Create the condition of game lost if the cell is a MINE.
     * Clear all neighbor cells if the cell is ZERO
     *
     * @param recursive - Indicates this cell is being cleared along with another cell.
     */
    bool bCleanCell(const uchar _X, const uchar _Y, const bool recursive=true);
    void vGameLost(const uchar _X, const uchar _Y);
    void vGameWon();

    void vGenerateEndGameScore(qint64 iTimeInNs = -1, bool ignorePreferences=false);

    std::vector< std::vector<CellGameEngine> > principalMatrix; /**< TODO: describe */

    uchar iX; /**< TODO: describe */
    uchar iY; /**< TODO: describe */

    short nMines; /**< TODO: describe */
    short iMinesLeft; /**< TODO: describe */
    ushort iHiddenCells; /**< TODO: describe */
    ushort iCellsToUnlock; /**< TODO: describe */

    QScopedPointer<QTimer> timerTimeInGame; /**< TODO: describe */
    QElapsedTimer elapsedPreciseTimeInGame;

    bool bFirst; /**< TODO: describe */
    bool bFirstCellClean; /**< TODO: describe */

    bool bUseQuestionMark;

    bool bGameActive;

Q_SIGNALS:
    void SIGNAL_showCell(const uchar _X, const uchar _Y, const bool recursive);
    void SIGNAL_endGameScore(LibreMinesScore score,
                             int iCorrectFlags,
                             int iWrongFlags,
                             int iUnlockedCells,
                             double dFlagsPerSecond,
                             double dCellsPerSecond,
                             bool ignorePreferences=false);
    void SIGNAL_currentTime(const ushort);
    void SIGNAL_minesLeft(const short);
    void SIGNAL_flagCell(const uchar _X, const uchar _Y);
    void SIGNAL_questionCell(const uchar _X, const uchar _Y);
    void SIGNAL_unflagCell(const uchar _X, const uchar _Y);
    void SIGNAL_gameWon();
    void SIGNAL_gameLost(const uchar _X, const uchar _Y);
    void SIGNAL_remakeGame();

public Q_SLOTS:
    void SLOT_cleanCell(const uchar _X, const uchar _Y);
    void SLOT_changeFlagState(const uchar _X, const uchar _Y);
    void SLOT_stop();
    void SLOT_startTimer();
    void SLOT_cleanNeighborCells(const uchar _X, const uchar _Y);
    void SLOT_generateEndGameScoreAgain();

private Q_SLOTS:
    void SLOT_UpdateTime();

};

#endif // LIBREMINESGAMEENGINE_H
