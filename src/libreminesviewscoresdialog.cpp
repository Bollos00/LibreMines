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


#include "libreminesviewscoresdialog.h"


LibreMinesViewScoresDialog::LibreMinesViewScoresDialog(QWidget *parent):
    QDialog(parent)
{
    this->setWindowTitle("High Scores");
    this->resize(500, 800);

    layoutGlobal = new QVBoxLayout(this);
    tabWidget = new QTabWidget(this);

    for(QString n : {tr("Easy"), tr("Medium"), tr("Hard"), tr("Easy (No Guess)"), tr("Medium (No Guess)"), tr("Hard (No Guess)")})
        vCreateTab(n);

    layoutGlobal->addWidget(tabWidget);
}

void LibreMinesViewScoresDialog::setScores(const QList<LibreMinesScore>& scores)
{
    for(const LibreMinesScore& s : scores)
    {
        if(s.gameDifficulty == GameDifficulty::EASY)
        {
            tabs[tr("Easy")].scores.append(s);
        }
        else if(s.gameDifficulty == GameDifficulty::MEDIUM)
        {
            tabs[tr("Medium")].scores.append(s);
        }
        else if(s.gameDifficulty == GameDifficulty::HARD)
        {
            tabs[tr("Hard")].scores.append(s);
        }
        else if(s.gameDifficulty == GameDifficulty::EASY_NOGUESS)
        {
            tabs[tr("Easy (No Guess)")].scores.append(s);
        }
        else if(s.gameDifficulty == GameDifficulty::MEDIUM_NOGUESS)
        {
            tabs[tr("Medium (No Guess)")].scores.append(s);
        }
        else if(s.gameDifficulty == GameDifficulty::HARD_NOGUESS)
        {
            tabs[tr("Hard (No Guess)")].scores.append(s);
        }
        else if(s.gameDifficulty == GameDifficulty::CUSTOMIZED)
        {
            QString strDifficulty;
            strDifficulty += QString::number(s.width) + " x ";
            strDifficulty += QString::number(s.height) + " : ";
            strDifficulty += QString::number(s.mines);

            if(!tabs.contains(strDifficulty))
            {
                vCreateTab(strDifficulty);
            }

            tabs[strDifficulty].scores.append(s);
        }
        else if(s.gameDifficulty == GameDifficulty::CUSTOMIZED_NOGUESS)
        {
            QString strDifficulty;
            strDifficulty += QString::number(s.width) + " x ";
            strDifficulty += QString::number(s.height) + " : ";
            strDifficulty += QString::number(s.mines) + " (No Guess)";

            if(!tabs.contains(strDifficulty))
            {
                vCreateTab(strDifficulty);
            }

            tabs[strDifficulty].scores.append(s);
        }
    }

    QStringList tableHorizontalHeader;
    tableHorizontalHeader << tr("User") << tr("Game Completed (%)") << tr("Time (secs)");

    for(const QString& key : tabs.keys())
    {
        LibreMinesScore::sort(tabs[key].scores);

        A& tab = tabs[key];
        tab.table->setRowCount(tab.scores.size());
        tab.table->setColumnCount(3);
        tab.table->setHorizontalHeaderLabels(tableHorizontalHeader);
        tab.table->setColumnWidth(0, 100);
        tab.table->setColumnWidth(1, 200);
        tab.table->setColumnWidth(2, 100);


        QStringList tableVerticalHeader;
        tableVerticalHeader.append(tr("1st"));

        if(tab.scores.size() >= 2)
            tableVerticalHeader.append(tr("2nd"));
        if(tab.scores.size() >= 3)
            tableVerticalHeader.append(tr("3rd"));

        for(int i=4; i<=tab.scores.size(); ++i)
        {
            tableVerticalHeader.append(tr("%1th").arg(i));
        }
        tab.table->setVerticalHeaderLabels(tableVerticalHeader);

        for(int i=0; i<tab.scores.size(); ++i)
        {
            QTableWidgetItem* user = new QTableWidgetItem(tab.scores.at(i).username);
            QTableWidgetItem* gameCompleted = new QTableWidgetItem( QString::number(tab.scores.at(i).dPercentageGameCompleted, 'f', 2) );
            QTableWidgetItem* time = new QTableWidgetItem( QString::number(tab.scores.at(i).iTimeInNs*1e-9, 'f', 3) );

            user->setFlags(Qt::NoItemFlags);
            gameCompleted->setFlags(Qt::NoItemFlags);
            time->setFlags(Qt::NoItemFlags);

            tab.table->setItem(i, 0, user);
            tab.table->setItem(i, 1, gameCompleted);
            tab.table->setItem(i, 2, time);
        }
    }
}

void LibreMinesViewScoresDialog::vCreateTab(const QString &tabName)
{
    tabs[tabName] = {new QWidget(), nullptr, {}};
    tabs[tabName].table = new QTableWidget(tabs[tabName].widget);
    tabWidget->addTab(tabs[tabName].widget, tabName);
    QVBoxLayout* layoutTab = new QVBoxLayout(tabs[tabName].widget);
    layoutTab->addWidget(tabs[tabName].table);

}
