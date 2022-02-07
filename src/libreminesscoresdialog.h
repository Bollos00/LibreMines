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


#ifndef LIBREMINESSCORESDIALOG_H
#define LIBREMINESSCORESDIALOG_H

#include <QDialog>
#include <QTableWidget>

#include "libreminesscore.h"

class LibreMinesScoresDialog : public QDialog
{
    Q_OBJECT
public:
    LibreMinesScoresDialog(QWidget *parent, int rowsCount);
    void setScores(QList<LibreMinesScore> scores,
                   const LibreMinesScore *const editableScore = nullptr,
                   const int editableScorePosition = -1);

    bool bSaveEditableScore()const;
    QString stringUserName()const;

private:
    QTableWidget* table;
    QPushButton* buttonSave;
    QPushButton* buttonCancel;
    QLineEdit* lineEditUsername;

    bool saveEditableScore;
};

#endif // LIBREMINESSCORESDIALOG_H
