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


#ifndef COMMON_H
#define COMMON_H

#include <QLabel>

/**
 * @brief
 *
 */
enum CELL_STATE: char{
    MINE = -1,
    ZERO = 0,
    ONE = 1,
    TWO = 2,
    THREE = 3,
    FOUR = 4,
    FIVE = 5,
    SIX = 6,
    SEVEN = 7,
    EIGHT = 8
};

/**
 * @brief
 *
 */
enum GAME_DIFFICULTY: char{
    NONE,
    EASY,
    MEDIUM,
    HARD,
    CUSTOMIZED
};


class Vector2Dshort
{
public:
    Vector2Dshort();
    Vector2Dshort(const qint16 _x, const qint16 _y);

    float length() const;
    float distanceToPoint(const Vector2Dshort& point) const;

    short x;
    short y;


};

struct KeyboardController
{
    bool ctrlPressed;
    bool active;
    uchar currentX;
    uchar currentY;
};



#endif // COMMON_H
