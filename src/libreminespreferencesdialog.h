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

/**
 * @file libreminespreferencesdialog.h
 * @brief User preferences and settings dialog for LibreMines
 * @author Bruno Bollos Correa
 * @date 2020-12-17
 * 
 * This file defines the LibreMinesPreferencesDialog class and related
 * enumerations for managing user preferences and configuration options.
 * It provides a comprehensive settings interface for customizing game
 * behavior, appearance, controls, and other user preferences.
 */

#ifndef LIBREMINESPREFERENCESDIALOG_H
#define LIBREMINESPREFERENCESDIALOG_H

#include <QDialog>
#include <QtCore>

namespace Ui {
class LibreMinesPreferencesDialog;
}

/**
 * @brief Namespace for LibreMines-specific enumerations and types
 * 
 * Contains enumerations and type definitions specific to LibreMines
 * configuration and preferences that are used throughout the application.
 */
namespace LibreMines
{
    /**
     * @brief Keyboard navigation behavior when Ctrl key is pressed
     * 
     * Defines how the keyboard controller behaves when the Ctrl key
     * is held down during navigation, affecting movement speed and distance.
     */
    enum WhenCtrlIsPressedOptions : uchar
    {
        GoToTheEdge = 0,    ///< Move cursor to the edge of the grid
        Jump3Cells = 1,     ///< Move cursor 3 cells at a time
        Jump5Cells = 2,     ///< Move cursor 5 cells at a time
        Jump10Cells = 3     ///< Move cursor 10 cells at a time
    };

    /**
     * @brief Animation settings for minefield generation
     * 
     * Controls the visual animation played during minefield generation,
     * allowing users to balance visual feedback with performance.
     */
    enum MinefieldGenerationAnimation : uchar
    {
        AnimationOn = 0,      ///< Full animation during generation
        AnimationLimited = 1, ///< Limited animation for faster generation  
        AnimationOff = 2      ///< No animation for fastest generation
    };

    /**
     * @brief Options for automatic score saving behavior
     * 
     * Defines when the application should automatically save or prompt
     * to save game scores. Can be combined using bitwise operations.
     */
    enum AskToSaveMatchScore : uchar
    {
        SaveNever              = 0x00, ///< Never save scores automatically
        SaveWhenNewHighScore   = 0x01, ///< Save when achieving a new high score
        SaveWhenGameCompleted  = 0x02, ///< Save when successfully completing any game
        SaveAlways             = 0x04  ///< Always save scores regardless of outcome
    };
}
Q_DECLARE_FLAGS(AskToSaveMatchScore, LibreMines::AskToSaveMatchScore)
Q_DECLARE_OPERATORS_FOR_FLAGS(AskToSaveMatchScore)

/**
 * @brief Comprehensive preferences and settings dialog for LibreMines
 * 
 * LibreMinesPreferencesDialog provides a user interface for configuring
 * all aspects of the LibreMines application including game behavior,
 * visual appearance, keyboard controls, sound settings, and user preferences.
 * 
 * The dialog is organized into multiple sections covering:
 * - Game behavior (first cell safety, neighbor clearing, etc.)
 * - Visual themes and appearance settings
 * - Keyboard navigation and accessibility options  
 * - Audio volume and sound effect settings
 * - Score saving and user account preferences
 * - Language and localization options
 * 
 * All settings are automatically persisted when changed and emit signals
 * to notify other parts of the application when preferences are updated.
 * The dialog integrates with the LibreMinesPreferencesSaver for reliable
 * configuration storage across application sessions.
 * 
 * @since v1.1.0
 */
class LibreMinesPreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LibreMinesPreferencesDialog(QWidget *parent = nullptr);
    ~LibreMinesPreferencesDialog();

    bool optionFirstCellClean()const;
    bool optionCleanNeighborCellsWhenClickedOnShowedCell()const;
    bool optionProgressBar()const;
    QString optionApplicationStyle()const;
    QString optionMinefieldTheme()const;
    QString optionFacesReaction()const;
    QString optionUsername()const;
    uchar optionWhenCtrlIsPressed()const;
    int optionMinimumCellLength()const;
    int optionMaximumCellLength()const;
    QString optionsLanguage()const;
    uchar optionMinefieldGenerationAnimation()const;
    QString optionMinefieldGenerationAnimationString()const;
    AskToSaveMatchScore optionAskToSaveMatchScoreBehaviour()const;
    int optionSoundVolume()const;
    bool optionUseQuestionMark()const;
    bool optionNoGuessMode()const;

    void setOptionFirstCellClean(const QString& option);
    void setOptionCleanNeighborCellsWhenClickedOnShowedCell(const QString& option);
    void setOptionProgressBar(const QString& option);
    void setOptionApplicationStyle(const QString& option);
    void setOptionMinefieldTheme(const QString& option);
    void setOptionFacesReaction(const QString& option);
    void setOptionUsername(const QString& username);
    void setOptionWhenCtrlIsPressed(const uchar option);
    void setOptionMinimumCellLength(const int option);
    void setOptionMaximumCellLength(const int option);
    void setOptionLanguage(const QString& option);
    void setOptionMinefieldGenerationAnimation(const uchar option);
    void setOptionMinefieldGenerationAnimation(const QString& option);
    void setOptionAskToSaveMatchScoreBehaviour(const uchar option);
    void setOptionSoundVolume(const int option);
    void setOptionUseQuestionMark(const QString& option);
    void setOptionNoGuessMode(const QString& option);

    QList<int> optionKeyboardControllerKeys()const;
    QString optionKeyboardControllerKeysString()const;
    void setOptionKeyboardControllerKeys(const QList<int>& keys);

protected:
    void closeEvent(QCloseEvent* e);
    void hideEvent(QHideEvent* e);
    void showEvent(QShowEvent* e);

private:
    Ui::LibreMinesPreferencesDialog *ui;
    bool updateLanguageDialog;

private Q_SLOTS:
    void SLOT_updateCellLengthParameters();
    void SLOT_updateLanguage();

    void on_cbSoundMute_stateChanged(int arg1);

    void on_sliderSoundVolume_valueChanged(int value);

    void on_sliderSoundVolume_sliderReleased();

    void on_cbFirstCellClean_checkStateChanged(const Qt::CheckState &arg1);

Q_SIGNALS:
    void SIGNAL_optionChanged(const QString& name, const QString& value);
    void SIGNAL_visibilityChanged(const bool visible);

    void SIGNAL_setSoundEffectVolume(const int v, const bool play);
};

#endif // LIBREMINESPREFERENCESDIALOG_H
