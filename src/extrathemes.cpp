#include "extrathemes.h"

#include <QDir>
#include <QDebug>
#include <QStandardPaths>

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

    for(const QString& path :
         QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation))
    {
        QDir dir(path);

        // Continue if the directory does not exist
        if(!dir.exists() || !dir.cd(themeDir))
            continue;

        for(const QString& themeName :
             dir.entryList(QDir::AllDirs | QDir::NoDot | QDir::NoDotDot))
        {
            // Add the new theme to the check box if it has a valid format
            if(ExtraThemes::isValidTheme(dir.path(), themeName, type))
            {
                extraThemes.append(themeName);
            }
        }
    }

    for(int i=0; i<extraThemes.size(); ++i)
    {
        for(int j=i+1; j<extraThemes.size(); ++j)
        {
            if(extraThemes[i] == extraThemes[j])
            {
                qWarning() << "Duplicated Minefield extra theme: " << extraThemes[j];
                extraThemes.removeAt(j);
                j--;
            }
        }
    }

    return extraThemes;
}
