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
 *****************************************************************************/

#include "facesreactionpreview.h"
#include "ui_facesreactionpreview.h"

FacesReactionPreview::FacesReactionPreview(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FacesReactionPreview)
    , iFaceLength(0)
{
    ui->setupUi(this);

    listLbFaces.append(ui->lbGameLost);
    listLbFaces.append(ui->lbUnhiddenCellPressed);
    listLbFaces.append(ui->lbGameWon);
    listLbFaces.append(ui->lbHiddenCellPressed);
    listLbFaces.append(ui->lbGameBeingGenerated);
    listLbFaces.append(ui->lbDefault);
}

FacesReactionPreview::~FacesReactionPreview()
{
    delete ui;
}

void FacesReactionPreview::vSetFacesReactionTheme(const QString& theme, int length)
{
    iFaceLength = length;

    QString themeKey = theme;
    if (theme.compare("Open Emoji Colored", Qt::CaseInsensitive) == 0)
    {
        themeKey = "OpenEmojiColored";
    }

    facesReaction.vSetFacesReactionTheme(themeKey, iFaceLength);

    setFixedHeight(iFaceLength);

    for (QLabel* lb: listLbFaces)
    {
        lb->setFixedSize(iFaceLength, iFaceLength);
        lb->setText("");
    }

    ui->lbGameLost->setPixmap(facesReaction.getPixmapFromGameEvent(FacesReaction::GAME_LOST));
    ui->lbUnhiddenCellPressed->setPixmap(facesReaction.getPixmapFromGameEvent(FacesReaction::UNHIDDEN_CELL_PRESSED));
    ui->lbGameWon->setPixmap(facesReaction.getPixmapFromGameEvent(FacesReaction::GAME_WON));
    ui->lbHiddenCellPressed->setPixmap(facesReaction.getPixmapFromGameEvent(FacesReaction::HIDDEN_CELL_PRESSED));
    ui->lbGameBeingGenerated->setPixmap(facesReaction.getPixmapFromGameEvent(FacesReaction::GAME_BEING_GENERATED));
    ui->lbDefault->setPixmap(facesReaction.getPixmapFromGameEvent(FacesReaction::DEFAULT));
}
