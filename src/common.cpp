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


#include "common.h"

#include <cmath>

Vector2Dshort::Vector2Dshort():
    x( 0 ),
    y( 0 )
{

}

Vector2Dshort::Vector2Dshort(const qint16 _x, const qint16 _y):
    x(_x),
    y(_y)
{

}

float Vector2Dshort::length() const
{
    return sqrtf(static_cast<float>(x*x + y*y));
}

float Vector2Dshort::distanceToPoint(const Vector2Dshort &point) const
{
    return Vector2Dshort(point.x - x, point.y - y).length();
}
