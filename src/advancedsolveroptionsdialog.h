/*****************************************************************************
 * LibreMines                                                                *
 * Copyright (C) 2020-2026  Bruno Bollos Correa                              *
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
 *****************************************************************************/

/**
 * @file advancedsolveroptionsdialog.h
 * @brief Dialog to tune the deduction strategies used by the No Guess solver
 * @author Bruno Bollos Correa
 * @date 2026-10-18
 *
 * This file defines the AdvancedSolverOptionsDialog class, a dialog that lets
 * the user choose which deduction strategies the LibreMinesSolver may apply
 * when generating a "No Guess" puzzle, together with the clue limit used by
 * the constraint enumeration tier. The dialog is opened from the preferences
 * dialog and its values are read when a new game is started.
 *
 * @see LibreMinesSolver
 * @see LibreMinesPreferencesDialog
 */

#ifndef ADVANCEDSOLVEROPTIONSDIALOG_H
#define ADVANCEDSOLVEROPTIONSDIALOG_H

#include "libreminessolver.h"

#include <QDialog>
#include <QPair>
#include <QVector>

class QCheckBox;

namespace Ui { class AdvancedSolverOptionsDialog; }

/**
 * @brief Dialog for configuring the No Guess solver strategies
 *
 * AdvancedSolverOptionsDialog provides one checkbox per deduction strategy
 * supported by LibreMinesSolver plus a spin box that limits how many clues a
 * single enumeration may consider. The basic and advanced pattern groups each
 * have a tri-state header checkbox so every pattern of the group can be
 * enabled or disabled at once.
 *
 * The selections are exposed as a bit mask (LibreMinesSolver::SolverStrategy)
 * and as an integer clue limit; the dialog does not apply the values itself,
 * it only stores and returns them.
 *
 * @see LibreMinesSolver::SolverStrategy
 * @see LibreMinesPreferencesDialog
 * @since v3.0.0
 */
class AdvancedSolverOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Construct the advanced solver options dialog
     *
     * Builds the widget tree, registers every strategy checkbox and initializes
     * the controls with the solver defaults (all strategies enabled and
     * LibreMinesSolver::DefaultMaxEnumerationClues clues).
     *
     * @param parent Parent widget that owns this dialog (optional)
     */
    explicit AdvancedSolverOptionsDialog(QWidget *parent = nullptr);

    /**
     * @brief Destructor
     *
     * Releases the UI resources allocated by the dialog.
     */
    ~AdvancedSolverOptionsDialog();

    /**
     * @brief Get the enabled solver strategies
     *
     * @return Bit mask combining every strategy whose checkbox is checked
     */
    LibreMinesSolver::SolverStrategy optionSolverStrategies() const;

    /**
     * @brief Set the enabled solver strategies
     *
     * Checks the checkbox of every strategy contained in the mask and clears
     * the checkbox of every other strategy. The group header checkboxes are
     * updated to match the resulting selection.
     *
     * @param strategies Bit mask of the strategies to enable
     */
    void setOptionSolverStrategies(const LibreMinesSolver::SolverStrategy strategies);

    /**
     * @brief Get the maximum number of clues considered by one enumeration
     *
     * @return Clue limit; 0 means "no limit"
     */
    int optionMaxEnumerationClues() const;

    /**
     * @brief Set the maximum number of clues considered by one enumeration
     *
     * @param clues Clue limit; 0 disables the limit, negative values are treated as 0
     */
    void setOptionMaxEnumerationClues(const int clues);

    /**
     * @brief Get the number of parallel solver threads
     *
     * @return Solver count, or LibreMinesGameEngine::AutoSolversCount for
     *         automatic sizing based on the board mine density
     */
    int optionSolversCount() const;

    /**
     * @brief Set the number of parallel solver threads
     *
     * @param n Solver count, or LibreMinesGameEngine::AutoSolversCount (0) for
     *          automatic sizing; values above the CPU core count are clamped
     */
    void setOptionSolversCount(const int n);

protected:
    /**
     * @brief Handle dialog close event
     *
     * Hides the dialog instead of destroying it so its options survive between
     * openings.
     *
     * @param e Pointer to the close event
     */
    void closeEvent(QCloseEvent* e);

    /**
     * @brief Handle dialog hide event
     *
     * Emits SIGNAL_visibilityChanged(false) so the owner can re-enable itself.
     *
     * @param e Pointer to the hide event
     */
    void hideEvent(QHideEvent* e);

    /**
     * @brief Handle dialog show event
     *
     * Emits SIGNAL_visibilityChanged(true) so the owner can disable itself
     * while the options are being edited.
     *
     * @param e Pointer to the show event
     */
    void showEvent(QShowEvent* e);

private:
    Ui::AdvancedSolverOptionsDialog *ui;

    QVector<QPair<QCheckBox*, LibreMinesSolver::SolverStrategy>> strategyCheckBoxes;  ///< Every strategy checkbox with its flag
    QVector<QCheckBox*> basicPatternCheckBoxes;      ///< Checkboxes of the basic pattern group
    QVector<QCheckBox*> advancedPatternCheckBoxes;   ///< Checkboxes of the advanced pattern group

    /**
     * @brief Collect the checkboxes that map to a solver strategy
     *
     * Fills strategyCheckBoxes, basicPatternCheckBoxes and
     * advancedPatternCheckBoxes and connects the signals used to keep the
     * tri-state group headers in sync with their children.
     */
    void vSetupStrategyCheckBoxes();

    /**
     * @brief Check or uncheck a group header and all of its children
     *
     * @param header Tri-state checkbox shown as the group header
     * @param children Checkboxes controlled by the header
     * @param checked true to check every checkbox, false to uncheck them
     */
    void vSetGroupChecked(QCheckBox* header, const QVector<QCheckBox*>& children, const bool checked);

    /**
     * @brief Update a group header to reflect the state of its children
     *
     * Sets the header to Checked when every child is checked, to Unchecked when
     * none is checked and to PartiallyChecked otherwise.
     *
     * @param header Tri-state checkbox shown as the group header
     * @param children Checkboxes controlled by the header
     */
    void vUpdateGroupHeader(QCheckBox* header, const QVector<QCheckBox*>& children) const;

    /**
     * @brief Check or uncheck every strategy checkbox
     *
     * @param checked true to enable all strategies, false to disable them
     */
    void vSetAllStrategiesChecked(const bool checked);

    /**
     * @brief Restore the solver defaults
     *
     * Enables every strategy and resets the clue limit to
     * LibreMinesSolver::DefaultMaxEnumerationClues.
     */
    void vResetToDefaults();

Q_SIGNALS:
    /**
     * @brief Emitted when the dialog's visibility changes
     *
     * @param visible true when the dialog is being shown, false when hidden
     */
    void SIGNAL_visibilityChanged(const bool visible);
};

#endif // ADVANCEDSOLVEROPTIONSDIALOG_H
