#ifndef EXTRATHEMES_H
#define EXTRATHEMES_H

#include <QString>

namespace ExtraThemes
{
enum Type
{
    Minefield,
    FacesReaction
};

bool isValidTheme(const QString& themePath, Type type);
bool isValidTheme(const QString& dirName, const QString& themeName, Type type);
QList<QString> getExtraThemes(Type type);
}

#endif // EXTRATHEMES_H
