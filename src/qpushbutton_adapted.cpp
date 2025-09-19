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
#include <QPainter>
#include <QStyleOptionButton>
#include <QStyle>

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

void QPushButton_adapted::setIconInverted()
{
    // Cached pixmap swap
    if (!isInverted) {
        setIcon(invertedIcon);
        isInverted = true;
    }
}

void QPushButton_adapted::setIconNormal()
{
    // Cached pixmap swap
    if (isInverted) {
        setIcon(normalIcon);
        isInverted = false;
    }
}

void QPushButton_adapted::mouseReleaseEvent(QMouseEvent *e)
{
    setIconNormal();

    Q_EMIT SIGNAL_released(e);
}

void QPushButton_adapted::mousePressEvent(QMouseEvent *e)
{
    setIconInverted();

    Q_EMIT SIGNAL_clicked(e);
}

void QPushButton_adapted::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    
    // Get the current icon (normal or inverted)
    QIcon currentIcon = isInverted ? invertedIcon : normalIcon;
    
    // Draw icon to fill entire button area
    if (!currentIcon.isNull()) {
        QRect iconRect = rect();
        currentIcon.paint(&painter, iconRect);
    }
}
