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


#ifndef LIBREMINESSCORE_H
#define LIBREMINESSCORE_H

#include "common.h"

class LibreMinesScore
{
public:
    LibreMinesScore();

    operator QString() const;

    qint64 iTimeInNs;
    GameDifficulty gameDifficulty;
    uchar width;
    uchar heigth;
    ushort mines;
    bool bGameCompleted;
    double dPercentageGameCompleted;
    QString username;

    static bool bFirstIsBetter(const LibreMinesScore& first, const LibreMinesScore& second);
    static void sort(QList<LibreMinesScore> &l);
};

QDataStream& operator<< (QDataStream& stream, const LibreMinesScore& score);
QDataStream& operator>> (QDataStream& stream, LibreMinesScore& score);
QDebug operator<<(QDebug debug, const LibreMinesScore& score);

#endif // LIBREMINESSCORE_H
