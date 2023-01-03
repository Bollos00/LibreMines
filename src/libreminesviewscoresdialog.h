/*****************************************************************************
 * LibreMines                                                                *
 * Copyright (C) 2020-2023  Bruno Bollos Correa                              *
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


#ifndef LIBREMINESVIEWSCORESDIALOG_H
#define LIBREMINESVIEWSCORESDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QMap>
#include <QTableWidget>

#include "libreminesscore.h"

class LibreMinesViewScoresDialog : public QDialog
{
    Q_OBJECT
public:
    LibreMinesViewScoresDialog(QWidget *parent);

    void setScores(const QList<LibreMinesScore>& scores);

private:

    void vCreateTab(const QString& tabName);

    QTabWidget* tabWidget;

    struct A{ QWidget* widget; QTableWidget* table; QList<LibreMinesScore> scores; };
    QMap<QString, A> tabs;

    QVBoxLayout* layoutGlobal;

};

#endif // LIBREMINESVIEWSCORESDIALOG_H
