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

#include "extrathemes.h"

#include <QDir>
#include <QDebug>
#include <QStandardPaths>
#include <QCoreApplication>

bool ExtraThemes::isValidTheme(const QString& themePath, Type type)
{
    QDir themeDir(themePath);

    // Files expected on the theme directory
    QStringList entries;

    switch(type)
    {
        case ExtraThemes::Minefield:
            entries = QStringList({"0", "1", "2", "3", "4", "5", "6", "7", "8",
                                   "boom", "flag", "mine", "no_flag", "wrong_flag", "question_flag"});
            break;
        case ExtraThemes::FacesReaction:
            entries = QStringList({"dizzy_face", "grimacing_face", "grinning_face",
                                   "open_mouth_face", "sleeping_face", "smilling_face" });
            break;
    }
    for(QString& v : entries)
    {
        v.append(".svg");
    }

    for(const QString& fileName :
         themeDir.entryList(QDir::Files))
    {
        if(entries.contains(fileName))
            entries.removeOne(fileName);
    }

    return entries.isEmpty();
}

bool ExtraThemes::isValidTheme(const QString& dirPath, const QString& themeName, Type type)
{
    QDir themeDir(dirPath);

    if(!themeDir.exists() || !themeDir.cd(themeName))
        return false;

    return ExtraThemes::isValidTheme(themeDir.path(), type);
}


QList<QString> ExtraThemes::getExtraThemes(Type type)
{
    const QString themeDir = type == Minefield ? "minefield_extra_themes" : "facesreaction_extra_themes";

    QList<QString> extraThemes;

    // Build list of paths to search
    QStringList searchPaths = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
    
#ifdef Q_OS_LINUX
    // Add AppImage paths if we're running as AppImage
    QString appImagePath = qgetenv("APPIMAGE");
    if(!appImagePath.isEmpty())
    {
        QString executableDir = QCoreApplication::applicationDirPath();
        searchPaths << executableDir + "/../share/libremines";
    }
#endif

    // Search all paths for themes
    for(const QString& basePath : searchPaths)
    {
        QDir dir(basePath);

        // Continue if the directory does not exist or can't navigate to theme dir
        if(!dir.exists() || !dir.cd(themeDir))
            continue;

        for(const QString& themeName : dir.entryList(QDir::AllDirs | QDir::NoDot | QDir::NoDotDot))
        {
            // Add theme if valid and not already found
            if(ExtraThemes::isValidTheme(dir.path(), themeName, type) && !extraThemes.contains(themeName))
            {
                extraThemes.append(themeName);
            }
        }
    }

    // Remove duplicates
    for(int i=0; i<extraThemes.size(); ++i)
    {
        for(int j=i+1; j<extraThemes.size(); ++j)
        {
            if(extraThemes[i] == extraThemes[j])
            {
                qWarning() << "Duplicated extra theme:" << extraThemes[j];
                extraThemes.removeAt(j);
                j--;
            }
        }
    }

    return extraThemes;
}
