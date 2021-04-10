/*****************************************************************************
 * LibreMines                                                                *
 * Copyright (C) 2020-2021  Bruno Bollos Correa                              *
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
#include "libreminesconfig.h"

#ifdef QT_NO_DEBUG_OUTPUT
#undef QT_NO_DEBUG_OUTPUT
#endif

struct InitializeOptions
{
    InitializeOptions():
        heightMainWindow(0),
        widthMainWindow(0)
    {};

    InitializeOptions(int h, int w):
        heightMainWindow(h),
        widthMainWindow(w)
    {};

    int heightMainWindow = -1;
    int widthMainWindow = -1;
};

InitializeOptions getOptions(const QStringList& args)
{
    int h = -1;
    int w = -1;

    QCommandLineParser parser;
    parser.setApplicationDescription(LIBREMINES_PROJECT_DESCRIPTION);
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions({
                {"width", "Width of the Main Widget - in pixels", "width", "-1"},
                {"heigth", "Heigth of the Main Widget - in pixels", "heigth", "-1"}
                      });

    parser.process(args);

    QStringList results = parser.positionalArguments();

    w = parser.value("width").toInt();
    h = parser.value("heigth").toInt();
    return{h, w};
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName(LIBREMINES_PROJECT_NAME);
    a.setApplicationVersion(LIBREMINES_PROJECT_VERSION);

//    a.setOverrideCursor(QCursor(QPixmap("/home/robofei/Repository/LibreMines/share/icons/libremines.svg").
//                                scaled(100, 100, Qt::KeepAspectRatio),
//                                50, 50));

    InitializeOptions ops = getOptions(a.arguments());

    LibreMinesGui* w = new LibreMinesGui(nullptr, ops.heightMainWindow, ops.widthMainWindow);

    w->show();
    return a.exec();
}
