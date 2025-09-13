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
 * @file libreminesviewscoresdialog.h
 * @brief High scores viewing dialog with tabbed interface
 * @author Bruno Bollos Correa
 * @date 2021-01-02
 * 
 * This file defines the LibreMinesViewScoresDialog class which provides
 * a comprehensive interface for viewing high scores organized by difficulty
 * level. The dialog uses a tabbed interface to separate scores by game
 * difficulty and board configuration.
 */

#ifndef LIBREMINESVIEWSCORESDIALOG_H
#define LIBREMINESVIEWSCORESDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QMap>
#include <QTableWidget>

#include "libreminesscore.h"

/**
 * @brief Dialog for viewing high scores organized by difficulty
 * 
 * LibreMinesViewScoresDialog provides a tabbed interface for viewing
 * high scores categorized by game difficulty level and board configuration.
 * Each tab represents a different difficulty level (Easy, Medium, Hard)
 * or custom board configuration, showing the best scores achieved for
 * that particular game type.
 * 
 * The dialog automatically organizes scores by their difficulty level
 * and board dimensions, creating separate tabs for different game
 * configurations. This allows players to compare their performance
 * across different game types and track their progress in each category.
 * 
 * @since v1.2.1
 */
class LibreMinesViewScoresDialog : public QDialog
{
    Q_OBJECT
public:
    /**
     * @brief Constructor for the view scores dialog
     * 
     * Creates a scores viewing dialog with a tabbed interface. Tabs will
     * be created dynamically based on the scores provided to the dialog.
     * 
     * @param parent Parent widget for Qt's parent-child memory management
     */
    LibreMinesViewScoresDialog(QWidget *parent);

    /**
     * @brief Set the scores to display in the dialog
     * 
     * Populates the dialog with the provided scores, automatically organizing
     * them into tabs based on difficulty level and board configuration.
     * Creates separate tabs for each unique game type found in the scores.
     * 
     * @param scores List of LibreMinesScore objects to display and organize
     */
    void setScores(const QList<LibreMinesScore>& scores);

private:
    /**
     * @brief Create a new tab for a specific difficulty/configuration
     * 
     * Creates a new tab with the specified name and sets up the table
     * widget for displaying scores of that particular game type.
     * 
     * @param tabName Name to display on the tab (e.g., "Easy", "Medium", "Custom 20x20")
     */
    void vCreateTab(const QString& tabName);

    QTabWidget* tabWidget; ///< Tab widget containing all difficulty-specific score tables

    /**
     * @brief Structure containing tab-specific components
     * 
     * Groups the widget, table, and scores for each tab to maintain
     * organization and easy access to tab-specific data.
     */
    struct A{ 
        QWidget* widget;              ///< Container widget for the tab content
        QTableWidget* table;          ///< Table widget for displaying scores
        QList<LibreMinesScore> scores; ///< Scores specific to this tab
    };
    QMap<QString, A> tabs; ///< Map of tab names to their corresponding components

    QVBoxLayout* layoutGlobal; ///< Main layout for the dialog
};

#endif // LIBREMINESVIEWSCORESDIALOG_H
