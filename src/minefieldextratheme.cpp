#include "minefieldextratheme.h"

#include <QDir>
#include <QDebug>


bool MinefieldExtraTheme::isValidTheme(const QString& themePath)
{
    QDir themeDir(themePath);

    // Files expected on the theme directory
    QStringList entries({ "0", "1", "2", "3", "4", "5", "6", "7", "8",
                          "boom", "flag", "mine", "no_flag", "wrong_flag"});
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

bool MinefieldExtraTheme::isValidTheme(const QString& dirPath, const QString& themeName)
{
    QDir themeDir(dirPath);

    if(!themeDir.exists() || !themeDir.cd(themeName))
        return false;

    return MinefieldExtraTheme::isValidTheme(themeDir.path());
}

