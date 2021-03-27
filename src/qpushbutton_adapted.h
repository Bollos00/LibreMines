/*****************************************************************************
 * LibreMines                                                                *
 * Copyright (C) 2020-2021  Bruno Bollos Correa                              *
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


#ifndef QPUSHBUTTON_ADAPTED_H
#define QPUSHBUTTON_ADAPTED_H

#include <QPushButton>

/**
 * @brief
 *
 */
class QPushButton_adapted : public QPushButton
{
    Q_OBJECT

public:
    /**
     * @brief
     *
     * @param parent
     */
    explicit QPushButton_adapted(QWidget *parent = nullptr);

protected:
    void mouseReleaseEvent(QMouseEvent* e);

Q_SIGNALS:
    /**
     * @brief
     *
     * @param
     */
    void SIGNAL_released(const QMouseEvent*const event);
};

#endif // QPUSHBUTTON_ADAPTED_H
