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


#include <QPushButton>
#include <QLineEdit>

#include "libreminesscoresdialog.h"

LibreMinesScoresDialog::LibreMinesScoresDialog(QWidget* parent, int rowsCount):
    QDialog(parent),
    table(nullptr),
    QPB_Save(nullptr),
    QPB_Cancel(nullptr),
    QLE_username(nullptr),
    saveEditableScore(false)
{
    table = new QTableWidget(this);
    QPB_Save = new QPushButton("Save", this);
    QPB_Cancel = new QPushButton("Cancel", this);
    QLE_username = new QLineEdit(this);

    table->setEnabled(true);

    table->setRowCount(rowsCount);
    table->setColumnCount(3);

    QStringList tableHorizontalHeader;
    tableHorizontalHeader << "User" << "Game Completed (%)" << "Time (secs)";
    table->setHorizontalHeaderLabels(tableHorizontalHeader);

    QStringList tableVerticalHeader;
    tableVerticalHeader.append("1st");

    if(rowsCount >= 2)
        tableVerticalHeader.append("2nd");
    if(rowsCount >= 3)
        tableVerticalHeader.append("3rd");

    for(int i=4; i<=rowsCount; ++i)
    {
        tableVerticalHeader.append(QString::number(i) + "th");
    }
    table->setVerticalHeaderLabels(tableVerticalHeader);

    table->setColumnWidth(0, 100);
    table->setColumnWidth(1, 200);
    table->setColumnWidth(2, 100);

    table->setGeometry(0, 0, 450, 600);
    QPB_Save->setGeometry(table->x() + table->width() + 15, 200, 200, 50);
    QPB_Cancel->setGeometry(QPB_Save->x(), QPB_Save->y() + QPB_Save->height() + 15, 200, 50);
    QLE_username->setGeometry(QPB_Cancel->x(), QPB_Cancel->y() + QPB_Cancel->height() + 15, 200, 50);
    this->setFixedSize(700, 600);
    table->setFixedSize(table->size());
}

void LibreMinesScoresDialog::setScores(QList<LibreMinesScore> scores,
                                       const LibreMinesScore*const editableScore,
                                       const int editableScorePosition)
{
    if(editableScore)
    {
        Q_ASSERT(editableScorePosition != -1);
        scores.insert(editableScorePosition, *editableScore);
        QLE_username->setText(editableScore->username);
    }

    // use iterators instead?
    for(int i=0; i<scores.size(); ++i)
    {
        QTableWidgetItem* user = new QTableWidgetItem(scores.at(i).username);
        QTableWidgetItem* gameCompleted = new QTableWidgetItem( QString::number(scores.at(i).dPercentageGameCompleted, 'f', 2) );
        QTableWidgetItem* time = new QTableWidgetItem( QString::number(scores.at(i).iTimeInNs*1e-9, 'f', 3) );

        user->setFlags(Qt::NoItemFlags);
        gameCompleted->setFlags(Qt::NoItemFlags);
        time->setFlags(Qt::NoItemFlags);

        table->setItem(i, 0, user);
        table->setItem(i, 1, gameCompleted);
        table->setItem(i, 2, time);
    }

    if(editableScore)
    {
        QTableWidgetItem* editableUser = table->item(editableScorePosition, 0);
        editableUser->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEditable);
        editableUser->setSelected(true);

        table->setCurrentItem(editableUser);

        connect(QLE_username, &QLineEdit::textChanged,
                [editableUser](const QString& text){ editableUser->setText(text); });
        connect(QPB_Save, &QPushButton::released,
                [this]{ this->saveEditableScore = true;  this->close(); });
        connect(QPB_Cancel, &QPushButton::released,
                this, &QDialog::close);

        QLE_username->setFocus();
    }
}

bool LibreMinesScoresDialog::bSaveEditableScore() const
{
    return saveEditableScore;
}

QString LibreMinesScoresDialog::stringUserName() const
{
    return QLE_username->text();
}
