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


#include <QApplication>
#include <QCommandLineParser>

#include "libreminesgui.h"

struct InitializeOptions
{
    InitializeOptions():
        heightMainWindow(0),
        widthMainWindow(0),
        maximumCellLength(0)
    {};

    InitializeOptions(int h, int w, int l):
        heightMainWindow(h),
        widthMainWindow(w),
        maximumCellLength(l)
    {};

    int heightMainWindow = -1;
    int widthMainWindow = -1;
    int maximumCellLength = 999999999;
};

InitializeOptions getOptions(const QStringList& args)
{
    int h = -1;
    int w = -1;
    int l = 999999999;

    QCommandLineParser parser;
    parser.setApplicationDescription("A Qt5 based Minesweeper game");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions({
                {"width", "Width of the Main Widget - in pixels", "width", "-1"},
                {"heigth", "Heigth of the Main Widget - in pixels", "heigth", "-1"},
                {"length", "Maximum length of each Cell - in pixels", "length", "999999999"}});

    parser.process(args);

    QStringList results = parser.positionalArguments();

    w = parser.value("width").toInt();
    h = parser.value("heigth").toInt();
    l = parser.value("length").toInt();
    return{h, w, l};
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("libremines");
    a.setApplicationVersion("1.1.0");

    InitializeOptions ops = getOptions(a.arguments());

    LibreMinesGui* w = new LibreMinesGui(nullptr, ops.heightMainWindow, ops.widthMainWindow, ops.maximumCellLength);

    w->show();
    return a.exec();
}
