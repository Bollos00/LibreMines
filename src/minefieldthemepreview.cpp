/*****************************************************************************
 * LibreMines                                                                *
 * Copyright (C) 2020-2026  Bruno Bollos Correa                              *
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

#include "minefieldthemepreview.h"
#include "ui_minefieldthemepreview.h"

MinefieldThemePreview::MinefieldThemePreview(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MinefieldThemePreview)
{
    ui->setupUi(this);

    listLbCells.append(ui->lbNoFlag);
    listLbCells.append(ui->lbFlag);
    listLbCells.append(ui->lbQuestionFlag);
    listLbCells.append(ui->lbZero);
    listLbCells.append(ui->lbZero);
    listLbCells.append(ui->lbZero);
    listLbCells.append(ui->lbOne);
    listLbCells.append(ui->lbTwo);
    listLbCells.append(ui->lbThree);
    listLbCells.append(ui->lbFour);
    listLbCells.append(ui->lbFive);
    listLbCells.append(ui->lbSix);
    listLbCells.append(ui->lbSeven);
    listLbCells.append(ui->lbEight);
    listLbCells.append(ui->lbMine);
    listLbCells.append(ui->lbBoom);
    listLbCells.append(ui->lbWrongFlag);
}

MinefieldThemePreview::~MinefieldThemePreview()
{
    delete ui;
}

void MinefieldThemePreview::vSetMinefieldTheme(const QString& theme, int lenght)
{
    iCellLength = lenght;

    QString themeKey = theme;
    if (theme.compare("Classic Dark", Qt::CaseInsensitive) == 0)
    {
        themeKey = "ClassicDark";
    }
    else if (theme.compare("Classic Light", Qt::CaseInsensitive) == 0)
    {
        themeKey = "ClassicLight";
    }

    minefieldTheme.vSetMinefieldTheme(themeKey, iCellLength);

    setFixedHeight(iCellLength);

    for (QLabel_adapted* lb: listLbCells)
    {
        lb->setFixedSize(iCellLength, iCellLength);
        lb->setText("");
    }

    // ui->lbThemeName->setText(theme);
    ui->lbNoFlag->setPixmapCached(minefieldTheme.getPixmapButton(FlagState::NoFlag));
    ui->lbFlag->setPixmapCached(minefieldTheme.getPixmapButton(FlagState::HasFlag));
    ui->lbQuestionFlag->setPixmapCached(minefieldTheme.getPixmapButton(FlagState::Question));
    ui->lbZero->setPixmapCached(minefieldTheme.getPixmapFromCellValue(CellValue::ZERO));
    ui->lbOne->setPixmapCached(minefieldTheme.getPixmapFromCellValue(CellValue::ONE));
    ui->lbTwo->setPixmapCached(minefieldTheme.getPixmapFromCellValue(CellValue::TWO));
    ui->lbThree->setPixmapCached(minefieldTheme.getPixmapFromCellValue(CellValue::THREE));
    ui->lbFour->setPixmapCached(minefieldTheme.getPixmapFromCellValue(CellValue::FOUR));
    ui->lbFive->setPixmapCached(minefieldTheme.getPixmapFromCellValue(CellValue::FIVE));
    ui->lbSix->setPixmapCached(minefieldTheme.getPixmapFromCellValue(CellValue::SIX));
    ui->lbSeven->setPixmapCached(minefieldTheme.getPixmapFromCellValue(CellValue::SEVEN));
    ui->lbEight->setPixmapCached(minefieldTheme.getPixmapFromCellValue(CellValue::EIGHT));
    ui->lbMine->setPixmapCached(minefieldTheme.getPixmapFromCellValue(CellValue::MINE));
    ui->lbBoom->setPixmapCached(minefieldTheme.getPixmapBoom());
    ui->lbWrongFlag->setPixmapCached(minefieldTheme.getPixmapWrongFlag());
}
