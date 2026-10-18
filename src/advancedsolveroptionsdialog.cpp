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

#include "advancedsolveroptionsdialog.h"
#include "ui_advancedsolveroptionsdialog.h"

#include <QCheckBox>
#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QSpinBox>

AdvancedSolverOptionsDialog::AdvancedSolverOptionsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AdvancedSolverOptionsDialog)
{
    ui->setupUi(this);

    vSetupStrategyCheckBoxes();

    connect(ui->pbResetDefaults, &QPushButton::clicked,
            this, &AdvancedSolverOptionsDialog::vResetToDefaults);

    connect(ui->buttonBox, &QDialogButtonBox::rejected,
            this, &AdvancedSolverOptionsDialog::close);

    // Initialize the controls with the solver defaults
    setOptionSolverStrategies(LibreMinesSolver::SolverStrategy::AllStrategies);
    setOptionMaxEnumerationClues(LibreMinesSolver::DefaultMaxEnumerationClues);
}

AdvancedSolverOptionsDialog::~AdvancedSolverOptionsDialog()
{
    delete ui;
}

LibreMinesSolver::SolverStrategy AdvancedSolverOptionsDialog::optionSolverStrategies() const
{
    quint64 strategies = static_cast<quint64>(LibreMinesSolver::SolverStrategy::None);

    for (const QPair<QCheckBox*, LibreMinesSolver::SolverStrategy>& pair : strategyCheckBoxes)
    {
        if (pair.first->isChecked())
        {
            strategies |= static_cast<quint64>(pair.second);
        }
    }

    return static_cast<LibreMinesSolver::SolverStrategy>(strategies);
}

void AdvancedSolverOptionsDialog::setOptionSolverStrategies(const LibreMinesSolver::SolverStrategy strategies)
{
    const quint64 strategyMask = static_cast<quint64>(strategies);

    for (const QPair<QCheckBox*, LibreMinesSolver::SolverStrategy>& pair : strategyCheckBoxes)
    {
        pair.first->setChecked((strategyMask & static_cast<quint64>(pair.second)) != 0);
    }

    // Keep the tri-state group headers in sync with the new selection
    vUpdateGroupHeader(ui->cbAllBasicPatterns, basicPatternCheckBoxes);
    vUpdateGroupHeader(ui->cbAllAdvancedPatterns, advancedPatternCheckBoxes);
}

int AdvancedSolverOptionsDialog::optionMaxEnumerationClues() const
{
    return ui->sbMaxEnumerationClues->value();
}

void AdvancedSolverOptionsDialog::setOptionMaxEnumerationClues(const int clues)
{
    ui->sbMaxEnumerationClues->setValue(qMax(0, clues));
}

void AdvancedSolverOptionsDialog::closeEvent(QCloseEvent* e)
{
    Q_UNUSED(e);
    this->hide();
}

void AdvancedSolverOptionsDialog::hideEvent(QHideEvent* e)
{
    Q_UNUSED(e);
    Q_EMIT SIGNAL_visibilityChanged(false);
}

void AdvancedSolverOptionsDialog::showEvent(QShowEvent* e)
{
    Q_UNUSED(e);
    Q_EMIT SIGNAL_visibilityChanged(true);
}

void AdvancedSolverOptionsDialog::vSetupStrategyCheckBoxes()
{
    strategyCheckBoxes =
    {
        {ui->cbPattern12X,       LibreMinesSolver::SolverStrategy::Pattern12X},
        {ui->cbPattern11X,       LibreMinesSolver::SolverStrategy::Pattern11X},
        {ui->cbPattern13X,       LibreMinesSolver::SolverStrategy::Pattern13X},
        {ui->cbPattern14X,       LibreMinesSolver::SolverStrategy::Pattern14X},
        {ui->cbPattern23X,       LibreMinesSolver::SolverStrategy::Pattern23X},
        {ui->cbPattern24X,       LibreMinesSolver::SolverStrategy::Pattern24X},
        {ui->cbPattern34X,       LibreMinesSolver::SolverStrategy::Pattern34X},
        {ui->cbPattern22X,       LibreMinesSolver::SolverStrategy::Pattern22X},
        {ui->cbPattern33X,       LibreMinesSolver::SolverStrategy::Pattern33X},
        {ui->cbPattern1X1,       LibreMinesSolver::SolverStrategy::Pattern1X1},
        {ui->cbPattern2X2,       LibreMinesSolver::SolverStrategy::Pattern2X2},
        {ui->cbPattern222Corner, LibreMinesSolver::SolverStrategy::Pattern222Corner},
        {ui->cbPattern131Corner, LibreMinesSolver::SolverStrategy::Pattern131Corner},
        {ui->cbPairRule,         LibreMinesSolver::SolverStrategy::PairRule},
        {ui->cbEnumeration,      LibreMinesSolver::SolverStrategy::Enumeration},
        {ui->cbMineCounting,     LibreMinesSolver::SolverStrategy::MineCounting}
    };

    basicPatternCheckBoxes =
    {
        ui->cbPattern12X,
        ui->cbPattern11X,
        ui->cbPattern13X,
        ui->cbPattern14X
    };

    advancedPatternCheckBoxes =
    {
        ui->cbPattern23X,
        ui->cbPattern24X,
        ui->cbPattern34X,
        ui->cbPattern22X,
        ui->cbPattern33X,
        ui->cbPattern1X1,
        ui->cbPattern2X2,
        ui->cbPattern222Corner,
        ui->cbPattern131Corner
    };

    // Clicking a group header selects or clears the whole group
    connect(ui->cbAllBasicPatterns, &QCheckBox::clicked,
            this, [this]()
    {
        bool allChecked = true;
        for (const QCheckBox* cb : basicPatternCheckBoxes)
        {
            if (!cb->isChecked())
            {
                allChecked = false;
                break;
            }
        }
        vSetGroupChecked(ui->cbAllBasicPatterns, basicPatternCheckBoxes, !allChecked);
    });

    connect(ui->cbAllAdvancedPatterns, &QCheckBox::clicked,
            this, [this]()
    {
        bool allChecked = true;
        for (const QCheckBox* cb : advancedPatternCheckBoxes)
        {
            if (!cb->isChecked())
            {
                allChecked = false;
                break;
            }
        }
        vSetGroupChecked(ui->cbAllAdvancedPatterns, advancedPatternCheckBoxes, !allChecked);
    });

    // Any child change updates the matching group header
    for (QCheckBox* cb : basicPatternCheckBoxes)
    {
        connect(cb, &QCheckBox::clicked,
                this, [this]()
        {
            vUpdateGroupHeader(ui->cbAllBasicPatterns, basicPatternCheckBoxes);
        });
    }

    for (QCheckBox* cb : advancedPatternCheckBoxes)
    {
        connect(cb, &QCheckBox::clicked,
                this, [this]()
        {
            vUpdateGroupHeader(ui->cbAllAdvancedPatterns, advancedPatternCheckBoxes);
        });
    }
}

void AdvancedSolverOptionsDialog::vSetGroupChecked(QCheckBox* header,
                                                   const QVector<QCheckBox*>& children,
                                                   const bool checked)
{
    for (QCheckBox* cb : children)
    {
        cb->setChecked(checked);
    }

    vUpdateGroupHeader(header, children);
}

void AdvancedSolverOptionsDialog::vUpdateGroupHeader(QCheckBox* header,
                                                     const QVector<QCheckBox*>& children) const
{
    int nChecked = 0;
    for (const QCheckBox* cb : children)
    {
        if (cb->isChecked())
        {
            ++nChecked;
        }
    }

    if (nChecked == 0)
    {
        header->setCheckState(Qt::Unchecked);
    }
    else if (nChecked == children.size())
    {
        header->setCheckState(Qt::Checked);
    }
    else
    {
        header->setCheckState(Qt::PartiallyChecked);
    }
}

void AdvancedSolverOptionsDialog::vResetToDefaults()
{
    setOptionSolverStrategies(LibreMinesSolver::SolverStrategy::AllStrategies);
    setOptionMaxEnumerationClues(LibreMinesSolver::DefaultMaxEnumerationClues);
}
