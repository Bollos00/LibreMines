#ifndef LIBREMINESAPPTHEME_H
#define LIBREMINESAPPTHEME_H

#include <QString>

namespace LibreMinesAppTheme
{

///
/// \brief Change the application theme according to the parameter _theme_
/// \param
/// \return True if the app needs to restart to apply the changes
///
bool bSetApplicationTheme(const QString& theme);
}

#endif // LIBREMINESAPPTHEME_H
