/**
 * @file extrathemes.h
 * @brief Extra theme validation and discovery for LibreMines
 * @author Bruno Bollos Correa
 * @date 2021-04-10
 * 
 * This file defines utilities for validating and discovering additional
 * themes for LibreMines beyond the built-in themes. It provides functions
 * to validate theme directories and enumerate available external themes
 * for both minefield and face reaction components.
 */

#ifndef EXTRATHEMES_H
#define EXTRATHEMES_H

#include <QString>

/**
 * @brief Namespace for extra theme management utilities
 * 
 * ExtraThemes provides functionality for working with external theme
 * packages that extend the visual customization options beyond the
 * built-in themes. It handles theme validation, discovery, and
 * enumeration for both minefield graphics and face reaction images.
 */
namespace ExtraThemes
{
    /**
     * @brief Types of themes that can be validated and loaded
     * 
     * Enumeration of the different theme categories supported by
     * the extra themes system.
     */
    enum Type
    {
        Minefield,      ///< Themes for minefield cell graphics (numbers, mines, flags)
        FacesReaction   ///< Themes for emoji face reactions to game events
    };

    /**
     * @brief Validate a theme directory or package
     * 
     * Checks whether the specified theme path contains all required
     * files for the given theme type and is properly formatted.
     * 
     * @param themePath Full path to the theme directory or package
     * @param type Type of theme to validate (Minefield or FacesReaction)
     * @return true if the theme is valid and can be loaded, false otherwise
     */
    bool isValidTheme(const QString& themePath, Type type);
    
    /**
     * @brief Validate a theme by directory and name
     * 
     * Checks whether a theme with the specified name exists in the
     * given directory and contains all required files for the theme type.
     * 
     * @param dirName Directory containing theme subdirectories
     * @param themeName Name of the specific theme to validate
     * @param type Type of theme to validate (Minefield or FacesReaction)
     * @return true if the theme is valid and can be loaded, false otherwise
     */
    bool isValidTheme(const QString& dirName, const QString& themeName, Type type);
    
    /**
     * @brief Get list of available extra themes
     * 
     * Scans the system for additional themes of the specified type
     * and returns a list of valid theme names that can be loaded.
     * 
     * @param type Type of themes to search for (Minefield or FacesReaction)
     * @return List of valid theme names available for the specified type
     */
    QList<QString> getExtraThemes(Type type);
}

#endif // EXTRATHEMES_H
