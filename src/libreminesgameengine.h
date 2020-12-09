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


#ifndef LIBREMINESGAMEENGINE_H
#define LIBREMINESGAMEENGINE_H

#include <QPointer>

#include "common.h"

class LibreMinesGameEngine
{
public:
    class CellGameEngine
    {
    public:
        CellGameEngine();

        CELL_STATE state;
        bool isHidden;
        bool hasFlag;
    };

public:
    LibreMinesGameEngine();
    void vNewGame(const uchar _X,
                  const uchar _Y,
                  ushort i_nMines_,
                  const uchar i_X_Clean = 255,
                  const uchar i_Y_Clean = 255);

    void vResetPrincipalMatrix();
    void vCleanCell(const uchar _X, const uchar _Y);
    void vGameLost(const uchar _X, const uchar _Y);
    void vGameWon();
    void vStartTimer();
    void vGenerateEndGameStatics();

public:
    std::vector< std::vector<CellGameEngine> > principalMatrix; /**< TODO: describe */

    uchar iX; /**< TODO: describe */
    uchar iY; /**< TODO: describe */

    ushort nMines; /**< TODO: describe */
    ushort iTimeInSeconds; /**< TODO: describe */
    ushort iMinesLeft; /**< TODO: describe */
    ushort iHiddenCells; /**< TODO: describe */
    ushort iCellsToUnlock; /**< TODO: describe */

    QScopedPointer<QTimer> timerTimeInGame; /**< TODO: describe */

    bool bFirst; /**< TODO: describe */
    bool bFirstCellClean; /**< TODO: describe */

    bool bGameActive;

Q_SIGNALS:
    void SIGNAL_showCell(const uchar _X, const uchar _Y);
    void SIGNAL_endGameStatics(const QString&);
    void SIGNAL_currentTime(const ushort);
    void SIGNAL_minesLeft(const ushort);

public Q_SLOTS:
    void SLOT_cleanCell(const uchar _X, const uchar _Y);
    void SLOT_addOrRemoveFlag(const uchar _X, const uchar _Y);

private Q_SLOTS:
    void SLOT_UpdateTime();

};

#endif // LIBREMINESGAMEENGINE_H
