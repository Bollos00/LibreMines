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


#ifndef LIBREMINESPREFERENCESDIALOG_H
#define LIBREMINESPREFERENCESDIALOG_H

#include <QDialog>

namespace Ui {
class LibreMinesPreferencesDialog;
}

class LibreMinesPreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LibreMinesPreferencesDialog(QWidget *parent = nullptr);
    ~LibreMinesPreferencesDialog();

    bool optionFirstCellClean()const;
    bool optionCleanNeighborCellsWhenClickedOnShowedCell()const;
    QString optionTheme()const;
    QString optionUsername()const;

    void setOptionFirstCellClean(const QString& option);
    void setOptionCleanNeighborCellsWhenClickedOnShowedCell(const QString& option);
    void setOptionTheme(const QString& theme);
    void setOptionUsername(const QString& username);

protected:
    void closeEvent(QCloseEvent* e);
    void hideEvent(QHideEvent* e);
    void showEvent(QShowEvent* e);

private:
    Ui::LibreMinesPreferencesDialog *ui;

Q_SIGNALS:
    void SIGNAL_optionChanged(const QString& name, const QString& value);
    void SIGNAL_visibilityChanged(const bool visible);
};

#endif // LIBREMINESPREFERENCESDIALOG_H
