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


#include "qpushbutton_adapted.h"
#include <QMouseEvent>

QPushButton_adapted::QPushButton_adapted(QWidget *parent, const uchar x, const uchar y) :
    QPushButton(parent),
    xCell(x),
    yCell(y)
{
    setFocusPolicy(Qt::ClickFocus);
}

uchar QPushButton_adapted::getXCell()
{
    return xCell;
}

uchar QPushButton_adapted::getYCell()
{
    return yCell;
}

void QPushButton_adapted::setIconCached(const QIcon& icn)
{
    // Store normal icon
    normalIcon = icn;
    
    // Store reversed icon
    QPixmap pm = icn.pixmap(this->size());
    QImage img = pm.toImage();
    img.invertPixels();
    invertedIcon = QIcon(QPixmap::fromImage(img));
    
    // Set the normal icon initially
    setIcon(normalIcon);
    isInverted = false;
}

void QPushButton_adapted::mouseReleaseEvent(QMouseEvent *e)
{
    // Cached pixmap swap
    if (isInverted) {
        setIcon(normalIcon);
        isInverted = false;
    }

    Q_EMIT SIGNAL_released(e);
}

void QPushButton_adapted::mousePressEvent(QMouseEvent *e)
{
    // Cached pixmap swap
    if (!isInverted) {
        setIcon(invertedIcon);
        isInverted = true;
    }

    Q_EMIT SIGNAL_clicked(e);
}
