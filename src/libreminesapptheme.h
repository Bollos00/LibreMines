/**
 * @file libreminesapptheme.h
 * @brief Application-wide theme management for LibreMines
 * @author Bruno Bollos Correa
 * @date 2020-2025
 * 
 * This file defines the LibreMinesAppTheme namespace which provides
 * functionality for managing the overall application theme including
 * color schemes, widget styling, and visual appearance that affects
 * the entire user interface beyond just the game grid.
 */

#ifndef LIBREMINESAPPTHEME_H
#define LIBREMINESAPPTHEME_H

#include <QString>

/**
 * @brief Namespace for application-wide theme management
 * 
 * LibreMinesAppTheme provides utilities for setting and managing the
 * overall visual theme of the LibreMines application. This includes
 * system integration with platform themes, dark/light mode support,
 * and custom styling that affects all UI elements.
 * 
 * The theme system is separate from the minefield graphics themes,
 * focusing instead on the application window, menus, dialogs, and
 * overall visual consistency with the operating system.
 */
namespace LibreMinesAppTheme
{
    /**
     * @brief Set the application theme
     * 
     * Changes the overall visual theme of the LibreMines application
     * to match the specified theme name. This may include changing
     * color schemes, widget styles, and other visual elements that
     * affect the entire user interface.
     * 
     * Some theme changes may require an application restart to take
     * full effect, particularly those that involve system-level
     * theme integration or fundamental styling changes.
     * 
     * @param theme Name of the theme to apply (e.g., "Dark", "Light", "System")
     * @return true if the application needs to be restarted for changes to take full effect, false if changes are immediate
     */
    bool bSetApplicationTheme(const QString& theme);
}

#endif // LIBREMINESAPPTHEME_H
