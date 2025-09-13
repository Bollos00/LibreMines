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
 * @file libreminesscoresdialog.h
 * @brief Score display and editing dialog for LibreMines
 * @author Bruno Bollos Correa
 * @date 2020-12-17
 * 
 * This file defines the LibreMinesScoresDialog class which provides
 * a user interface for displaying high scores and allowing users to
 * edit their username for score entries. The dialog presents scores
 * in a table format with options for saving edited entries.
 */

#ifndef LIBREMINESSCORESDIALOG_H
#define LIBREMINESSCORESDIALOG_H

#include <QDialog>
#include <QTableWidget>

#include "libreminesscore.h"

/**
 * @brief Dialog for displaying and editing game scores
 * 
 * LibreMinesScoresDialog provides a table-based interface for viewing
 * high scores and editing score entries. It allows users to modify
 * usernames associated with their scores and save the changes back
 * to the score database.
 * 
 * The dialog can operate in two modes:
 * - Read-only mode for viewing existing scores
 * - Edit mode for modifying a specific score entry (typically a new score)
 * 
 * When in edit mode, the dialog provides input controls for changing
 * the username and save/cancel buttons for confirming or discarding
 * changes.
 * 
 * @since v1.1.0
 */
class LibreMinesScoresDialog : public QDialog
{
    Q_OBJECT
public:
    /**
     * @brief Constructor for the scores dialog
     * 
     * Creates a scores dialog with a table widget configured to display
     * the specified number of score entries.
     * 
     * @param parent Parent widget for Qt's parent-child memory management
     * @param rowsCount Number of score rows to display in the table
     */
    LibreMinesScoresDialog(QWidget *parent, int rowsCount);
    
    /**
     * @brief Set the scores to display in the dialog
     * 
     * Populates the dialog with the provided list of scores. Optionally
     * allows one score to be editable, typically for a newly achieved score.
     * 
     * @param scores List of LibreMinesScore objects to display
     * @param editableScore Pointer to the score that can be edited (nullptr for read-only)
     * @param editableScorePosition Position in the table of the editable score (-1 if none)
     */
    void setScores(QList<LibreMinesScore> scores,
                   const LibreMinesScore *const editableScore = nullptr,
                   const int editableScorePosition = -1);

    /**
     * @brief Check if the user chose to save the edited score
     * 
     * Returns true if the user clicked the save button to confirm changes
     * to an editable score entry.
     * 
     * @return true if the edited score should be saved, false otherwise
     */
    bool bSaveEditableScore()const;
    
    /**
     * @brief Get the username entered by the user
     * 
     * Returns the username text that the user entered in the username
     * field for the editable score entry.
     * 
     * @return QString containing the entered username
     */
    QString stringUserName()const;

private:
    QTableWidget* table;          ///< Table widget for displaying scores
    QPushButton* buttonSave;      ///< Button to save changes to editable score
    QPushButton* buttonCancel;    ///< Button to cancel changes and close dialog
    QLineEdit* lineEditUsername;  ///< Input field for editing username

    bool saveEditableScore;       ///< Whether the user chose to save the edited score
};

#endif // LIBREMINESSCORESDIALOG_H
