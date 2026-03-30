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

#include "libreminesapptheme.h"

#include <QApplication>
#include <QPalette>
#include <QMessageBox>
#include <QStyleFactory>
#include <QDebug>
#include <QFile>

bool LibreMinesAppTheme::bSetApplicationTheme(const QString& theme)
{
    static bool firstTimeHere = true;

    bool restartNeeded = false;

    if(theme.compare("default", Qt::CaseInsensitive) == 0)
    {
        if(!firstTimeHere)
        {
            restartNeeded = true;
        }
    }
    else if(theme.compare("FusionDark", Qt::CaseInsensitive) == 0)
    {
        qApp->setStyleSheet("");
        qApp->setStyle(QStyleFactory::create ("Fusion"));
        QPalette palette;
        palette.setColor(QPalette::BrightText,      Qt::red);
        palette.setColor(QPalette::WindowText,      Qt::white);
        palette.setColor(QPalette::ToolTipBase,     Qt::white);
        palette.setColor(QPalette::ToolTipText,     Qt::white);
        palette.setColor(QPalette::Text,            Qt::white);
        palette.setColor(QPalette::ButtonText,      Qt::white);
        palette.setColor(QPalette::HighlightedText, Qt::black);
        palette.setColor(QPalette::Window,          QColor (53, 53, 53));
        palette.setColor(QPalette::Base,            QColor (25, 25, 25));
        palette.setColor(QPalette::AlternateBase,   QColor (53, 53, 53));
        palette.setColor(QPalette::Button,          QColor (53, 53, 53));
        palette.setColor(QPalette::Link,            QColor (42, 130, 218));
        palette.setColor(QPalette::Highlight,       QColor (42, 130, 218));

        qApp->setPalette(palette);
    }
    else if(theme.compare("FusionLight", Qt::CaseInsensitive) == 0)
    {
        qApp->setStyleSheet("");
        qApp->setStyle(QStyleFactory::create ("Fusion"));
        QPalette palette;
        palette.setColor(QPalette::BrightText,      Qt::cyan);
        palette.setColor(QPalette::WindowText,      Qt::black);
        palette.setColor(QPalette::ToolTipBase,     Qt::black);
        palette.setColor(QPalette::ToolTipText,     Qt::black);
        palette.setColor(QPalette::Text,            Qt::black);
        palette.setColor(QPalette::ButtonText,      Qt::black);
        palette.setColor(QPalette::HighlightedText, Qt::white);
        palette.setColor(QPalette::Window,          QColor (202, 202, 202));
        palette.setColor(QPalette::Base,            QColor (228, 228, 228));
        palette.setColor(QPalette::AlternateBase,   QColor (202, 202, 202));
        palette.setColor(QPalette::Button,          QColor (202, 202, 202));
        palette.setColor(QPalette::Link,            QColor (213, 125, 37));
        palette.setColor(QPalette::Highlight,       QColor (42, 130, 218));

        qApp->setPalette(palette);
    }
    else if(QStyleFactory::keys().contains(theme))
    {
        qApp->setStyleSheet("");
        qApp->setPalette(QPalette());
        qApp->setStyle(QStyleFactory::create(theme));
    }
    else
    {

    }

    firstTimeHere = false;

    return restartNeeded;
}
