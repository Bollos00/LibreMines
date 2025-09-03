/**
 * @file libreminespreferencessaver.h
 * @brief Preferences persistence and configuration file management
 * @author Bruno Bollos Correa
 * @date 2020-12-17
 * 
 * This file defines utilities for saving and loading user preferences
 * and game configuration settings. It handles the persistence of all
 * customizable options between application sessions, ensuring that
 * user preferences are maintained across restarts.
 */

#ifndef LIBREMINESPREFERENCESSAVER_H
#define LIBREMINESPREFERENCESSAVER_H

#include "libreminespreferencesdialog.h"

class QSpinBox;
class QCheckBox;

/**
 * @brief Namespace for preferences persistence utilities
 * 
 * LibreMinesPreferencesSaver provides functionality for saving and loading
 * user preferences to and from configuration files. It handles the complete
 * serialization of all user settings including game options, visual themes,
 * keyboard controls, and custom game parameters.
 * 
 * The persistence system uses platform-appropriate configuration directories
 * and file formats to ensure reliable storage and retrieval of user preferences
 * across different operating systems and user environments.
 */
namespace LibreMinesPreferencesSaver
{
    /**
     * @brief Load configuration from the last session
     * 
     * Reads the saved configuration file and restores all user preferences
     * and custom game settings to their last known state. This includes
     * both dialog preferences and game-specific parameters like custom
     * board dimensions and mine percentages.
     * 
     * @param preferences Reference to preferences dialog to populate with loaded settings
     * @param dirAppData Application data directory containing configuration files
     * @param sbCustomizedPercentageMines Spin box for custom mine percentage setting
     * @param sbCustomizedX Spin box for custom board width setting
     * @param sbCustomizedY Spin box for custom board height setting
     * @param cbCustomizedMinesInPercentage Checkbox for percentage-based mine setting
     */
    void vLastSessionLoadConfigurationFile(
        LibreMinesPreferencesDialog& preferences, const QDir& dirAppData,
        QSpinBox& sbCustomizedPercentageMines, QSpinBox& sbCustomizedX, QSpinBox& sbCustomizedY,
        QCheckBox& cbCustomizedMinesInPercentage
    );

    /**
     * @brief Save current configuration for future sessions
     * 
     * Writes all current user preferences and custom game settings to the
     * configuration file for restoration in future application sessions.
     * This ensures that all user customizations persist across application
     * restarts.
     * 
     * @param preferences Const reference to preferences dialog containing current settings
     * @param dirAppData Application data directory for storing configuration files
     * @param sbCustomizedPercentageMines Spin box containing custom mine percentage setting
     * @param sbCustomizedX Spin box containing custom board width setting
     * @param sbCustomizedY Spin box containing custom board height setting
     * @param cbCustomizedMinesInPercentage Checkbox containing percentage-based mine setting
     */
    void vLastSessionSaveConfigurationFile(
        const LibreMinesPreferencesDialog& preferences, const QDir& dirAppData,
        const QSpinBox& sbCustomizedPercentageMines, const QSpinBox& sbCustomizedX,
        const QSpinBox& sbCustomizedY,
        const QCheckBox& cbCustomizedMinesInPercentage
    );
};

#endif // LIBREMINESPREFERENCESSAVER_H
