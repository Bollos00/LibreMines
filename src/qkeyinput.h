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


#ifndef QKEYINPUT_H
#define QKEYINPUT_H

#include <QLineEdit>
#include <QObject>

class QKeyInput : public QLineEdit
{
    Q_OBJECT
public:
    QKeyInput(QWidget* parent = nullptr);

    int currentKey()const;

    bool setKey(const int k);

    bool isValid()const;

    static QString getTextFromKey(const int k);

protected:
    void keyPressEvent(QKeyEvent* e);

private:
    int key;
};

#endif // QKEYINPUT_H
