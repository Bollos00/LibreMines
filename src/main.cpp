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


#include <QApplication>
#include <QCommandLineParser>
#include <QTranslator>

#include "libreminesgui.h"
#include "libreminesconfig.h"

struct InitializeOptions
{
    InitializeOptions():
        heightMainWindow(0),
        widthMainWindow(0)
    {}

    InitializeOptions(int h, int w):
        heightMainWindow(h),
        widthMainWindow(w)
    {}

    int heightMainWindow = -1;
    int widthMainWindow = -1;
};

// This functions will check if the current application local dir is different of
//  "~/.local/share/libremines". This will occur when the user upgrade the application
//  from version 1.9.1 (or before) to the current 2.0.0 (or older).
//  In case the directories are different and that are user settings or high scores,
//  it will import those files by copying to the new data dir location.
void autoImportScoresAndSettingsFromLocalShare()
{
    QDir oldFlatpakDataDir = QDir( QDir::homePath() + "/.local/share/libremines");

    QDir newFlatpakDataDir = QDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));

    if(!oldFlatpakDataDir.exists() ||
        oldFlatpakDataDir.absolutePath() == newFlatpakDataDir.absolutePath())
    {
        return;
    }

    qDebug() << "oldFlatpakDataDir: " << oldFlatpakDataDir.absolutePath();
    qDebug() << "newFlatpakDataDir: " << newFlatpakDataDir.absolutePath();

    if(!newFlatpakDataDir.exists())
    {
        qDebug() << newFlatpakDataDir.absolutePath() << " does not exist.";
        if(newFlatpakDataDir.mkpath("."))
        {
            qDebug() << newFlatpakDataDir.absolutePath() << " successsfully created.";
        }
        else
        {
            qDebug() << "Failed to create " << newFlatpakDataDir.absolutePath()
                     << ". Ending here!";
            return;
        }
    }


    QFile oldSettings(oldFlatpakDataDir.absoluteFilePath("libreminesLastSession.txt"));

    if(oldSettings.exists())
    {
        QString newSettingsStr = newFlatpakDataDir.absoluteFilePath("libreminesLastSession.txt");
        if(!QFile(newSettingsStr).exists())
        {
            if(oldSettings.copy(newSettingsStr))
            {
                qDebug() << oldSettings.fileName() << " was automatticaly copied to " << newSettingsStr
                         << " to preserve user preferences.";
            }
            else
            {
                qDebug() << "Failed to copy " << oldSettings.fileName() << " to " << newSettingsStr;

            }
        }
    }

    QFile oldHighScores(oldFlatpakDataDir.absoluteFilePath("scoresLibreMines"));

    if(oldHighScores.exists())
    {
        QString newHighScoresStr = newFlatpakDataDir.absoluteFilePath("scoresLibreMines");
        if(!QFile(newHighScoresStr).exists())
        {
            if(oldHighScores.copy(newHighScoresStr))
            {
                qDebug() << oldHighScores.fileName() << " was automatticaly copied to " << newHighScoresStr
                         << " to preserve user high scores history.";
            }
            else
            {
                qDebug() << "Failed to copy " << oldHighScores.fileName() << " to " << newHighScoresStr;

            }
        }
    }

    QFile oldLanguage(oldFlatpakDataDir.absoluteFilePath("libreminesDefaultLanguage.txt"));

    if(oldLanguage.exists())
    {
        QString newLanguageStr = newFlatpakDataDir.absoluteFilePath("libreminesDefaultLanguage.txt");
        if(!QFile(newLanguageStr).exists())
        {
            if(oldLanguage.copy(newLanguageStr))
            {
                qDebug() << oldLanguage.fileName() << " was automatticaly copied to " << newLanguageStr
                         << " user language preference.";
            }
            else
            {
                qDebug() << "Failed to copy " << oldLanguage.fileName() << " to " << newLanguageStr;

            }
        }
    }
}

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
                {"height", "Height of the Main Widget - in pixels", "height", "-1"}
                      });

    parser.process(args);

    QStringList results = parser.positionalArguments();

    w = parser.value("width").toInt();
    h = parser.value("height").toInt();
    return{h, w};
}

void loadLanguagePreference()
{
    QDir destDir = QDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));

    // Create the app data directory in case it does not exist
    if(!destDir.exists())
    {
        QDir().mkpath(destDir.path());
        return;
    }

    QScopedPointer<QFile> fileLanguage( new QFile(destDir.absoluteFilePath("libreminesDefaultLanguage.txt")) );

    if(fileLanguage->open(QIODevice::ReadOnly))
    {
        QTextStream stream(fileLanguage.get());
        QString language;
        stream >> language;

        QTranslator* translator = new QTranslator();

        if(!language.isNull() && translator->load(":/translations/libremines_" + language + ".qm"))
            qApp->installTranslator(translator);
        else
            delete translator;
    }
    else
    {
        // On the first launch, set the language to the user preference
        QLocale usrLocale = QLocale::system();
        if(usrLocale.language() == QLocale::Portuguese)
        {
            QTranslator* translator = new QTranslator();
            if(translator->load(":/translations/libremines_pt_BR.qm"))
                qApp->installTranslator(translator);
            else
                delete translator;
        }
//        else if(usrLocale.language() == QLocale::Spanish)
//        {
//            QTranslator* translator = new QTranslator();
//            translator->load(":/translations/libremines_es.qm");
//            qApp->installTranslator(translator);
//        }
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName(LIBREMINES_PROJECT_NAME);
    a.setApplicationVersion(LIBREMINES_PROJECT_VERSION);

    InitializeOptions ops = getOptions(a.arguments());

    autoImportScoresAndSettingsFromLocalShare();

    loadLanguagePreference();

    LibreMinesGui* w = new LibreMinesGui(nullptr, ops.heightMainWindow, ops.widthMainWindow);

    w->show();
    return a.exec();
}
