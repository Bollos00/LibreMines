/*****************************************************************************
 * LibreMines                                                                *
 * Copyright (C) 2020-2021  Bruno Bollos Correa                              *
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


#include "libreminespreferencesdialog.h"
#include "ui_libreminespreferencesdialog.h"


LibreMinesPreferencesDialog::LibreMinesPreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LibreMinesPreferencesDialog)
{
    ui->setupUi(this);

    ui->comboBoxApplicationTheme->addItems({"Fusion Dark", "Fusion Light"});
    ui->comboBoxMinefieldTheme->addItems({"Classic Dark", "Classic Light"});
    ui->comboBoxFacesReaction->addItems({"Open Emoji Colored", "Open Emoji Black", "Open Emoji White", "Disable"});
    ui->comboBoxWhenCtrlIsPressed->addItems({"Go to the Edge", "Jump 3 Cells", "Jump 5 Cells", "Jump 10 Cells"});


    // Space character is not allowed. This will remove every space character of the line edit
//    ui->lineEditUsername->setValidator(new QRegExpValidator(QRegExp("^(?i)[a-z][a-z0-9]*$")));
    connect(ui->lineEditUsername, &QLineEdit::textChanged,
            [this](QString text)
    { ui->lineEditUsername->setText(text.remove(" ", Qt::CaseInsensitive)); });

    connect(ui->comboBoxApplicationTheme, &QComboBox::currentTextChanged,
            [this](QString text)
    {
        text.remove(" ", Qt::CaseInsensitive);
        Q_EMIT SIGNAL_optionChanged("ApplicationTheme", text);
    });

    connect(ui->comboBoxMinefieldTheme, &QComboBox::currentTextChanged,
            [this](QString text)
    {
        text.remove(" ", Qt::CaseInsensitive);
        Q_EMIT SIGNAL_optionChanged("MinefieldTheme", text);
    });

    connect(ui->sbMinimumCellLength, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &LibreMinesPreferencesDialog::SLOT_updateCellLengthParameters);
    connect(ui->sbMaximumCellLength, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &LibreMinesPreferencesDialog::SLOT_updateCellLengthParameters);

    ui->keyInputMoveLeft->setKey(Qt::Key_A);
    ui->keyInputMoveUp->setKey(Qt::Key_W);
    ui->keyInputMoveRight->setKey(Qt::Key_D);
    ui->keyInputMoveDown->setKey(Qt::Key_S);
    ui->keyInputReleaseCell->setKey(Qt::Key_O);
    ui->keyInputFlagCell->setKey(Qt::Key_P);

}

LibreMinesPreferencesDialog::~LibreMinesPreferencesDialog()
{
    delete ui;
}

bool LibreMinesPreferencesDialog::optionFirstCellClean() const
{
    return ui->cbFirstCellClean->isChecked();
}

bool LibreMinesPreferencesDialog::optionCleanNeighborCellsWhenClickedOnShowedCell() const
{
    return ui->cbCleanNeighborCellsWhenClickedOnShowedCell->isChecked();
}

bool LibreMinesPreferencesDialog::optionProgressBar() const
{
    return ui->cbProgressBarInGame->isChecked();
}

QString LibreMinesPreferencesDialog::optionApplicationTheme() const
{
    QString s = ui->comboBoxApplicationTheme->currentText();
    s.remove(" ", Qt::CaseInsensitive);
    return s;
}

QString LibreMinesPreferencesDialog::optionMinefieldTheme() const
{
    QString s = ui->comboBoxMinefieldTheme->currentText();
    s.remove(" ", Qt::CaseInsensitive);
    return s;
}

QString LibreMinesPreferencesDialog::optionFacesReaction() const
{
    QString s = ui->comboBoxFacesReaction->currentText();
    s.remove(" ", Qt::CaseInsensitive);
    return s;
}

QString LibreMinesPreferencesDialog::optionUsername() const
{
    return ui->lineEditUsername->text();
}

uchar LibreMinesPreferencesDialog::optionWhenCtrlIsPressed() const
{
    return ui->comboBoxWhenCtrlIsPressed->currentIndex();
}

int LibreMinesPreferencesDialog::optionMinimumCellLength() const
{
    return ui->sbMinimumCellLength->value();
}

int LibreMinesPreferencesDialog::optionMaximumCellLength() const
{
    return ui->sbMaximumCellLength->value();
}

void LibreMinesPreferencesDialog::setOptionFirstCellClean(const QString &option)
{
    ui->cbFirstCellClean->setChecked(option.compare("On", Qt::CaseInsensitive) == 0);
}

void LibreMinesPreferencesDialog::setOptionCleanNeighborCellsWhenClickedOnShowedCell(const QString &option)
{
    ui->cbCleanNeighborCellsWhenClickedOnShowedCell->setChecked(option.compare("On", Qt::CaseInsensitive) == 0);
}

void LibreMinesPreferencesDialog::setOptionProgressBar(const QString &option)
{
    ui->cbProgressBarInGame->setChecked(option.compare("On", Qt::CaseInsensitive) == 0);
}

void LibreMinesPreferencesDialog::setOptionApplicationTheme(const QString &option)
{
    QString s;
    if(option.compare("FusionDark", Qt::CaseInsensitive) == 0){ s = "Fusion Dark"; }
    else if(option.compare("FusionLight", Qt::CaseInsensitive) == 0){ s = "Fusion Light"; }

    ui->comboBoxApplicationTheme->setCurrentText(s);
}

void LibreMinesPreferencesDialog::setOptionMinefieldTheme(const QString &option)
{
    QString s;
    if(option.compare("ClassicLight", Qt::CaseInsensitive) == 0){ s = "Classic Light"; }
    else if(option.compare("ClassicDark", Qt::CaseInsensitive) == 0){ s = "Classic Dark"; }

    ui->comboBoxMinefieldTheme->setCurrentText(s);
}

void LibreMinesPreferencesDialog::setOptionFacesReaction(const QString &option)
{
    QString s = "Disable";
    if(option.compare("OpenEmojiColored", Qt::CaseInsensitive) == 0){ s = "Open Emoji Colored"; }
    else if(option.compare("OpenEmojiBlack", Qt::CaseInsensitive) == 0){ s = "Open Emoji Black"; }
    else if(option.compare("OpenEmojiWhite", Qt::CaseInsensitive) == 0){ s = "Open Emoji White"; }

    ui->comboBoxFacesReaction->setCurrentText(s);
}

void LibreMinesPreferencesDialog::setOptionUsername(const QString &username)
{
    ui->lineEditUsername->setText(username);
}

void LibreMinesPreferencesDialog::setOptionWhenCtrlIsPressed(const uchar option)
{
    if(ui->comboBoxWhenCtrlIsPressed->count() > option)
        ui->comboBoxWhenCtrlIsPressed->setCurrentIndex(option);
}

void LibreMinesPreferencesDialog::setOptionMinimumCellLength(const int option)
{
    ui->sbMinimumCellLength->setValue(option);
}

void LibreMinesPreferencesDialog::setOptionMaximumCellLength(const int option)
{
    ui->sbMaximumCellLength->setValue(option);
}

QList<int> LibreMinesPreferencesDialog::optionKeyboardControllerKeys() const
{
    return
    {
        ui->keyInputMoveLeft->currentKey(),
        ui->keyInputMoveUp->currentKey(),
        ui->keyInputMoveRight->currentKey(),
        ui->keyInputMoveDown->currentKey(),
        ui->keyInputReleaseCell->currentKey(),
        ui->keyInputFlagCell->currentKey()
    };
}

QString LibreMinesPreferencesDialog::optionKeyboardControllerKeysString() const
{
    QString s;

    s += QString::number(ui->keyInputMoveLeft->currentKey(), 16);
    s += ' ';
    s += QString::number(ui->keyInputMoveUp->currentKey(), 16);
    s += ' ';
    s += QString::number(ui->keyInputMoveRight->currentKey(), 16);
    s += ' ';
    s += QString::number(ui->keyInputMoveDown->currentKey(), 16);
    s += ' ';
    s += QString::number(ui->keyInputReleaseCell->currentKey(), 16);
    s += ' ';
    s += QString::number(ui->keyInputFlagCell->currentKey(), 16);

    return s;
}

void LibreMinesPreferencesDialog::setOptionKeyboardControllerKeys(const QList<int> &keys)
{
    ui->keyInputMoveLeft->setKey(keys.at(0));
    ui->keyInputMoveUp->setKey(keys.at(1));
    ui->keyInputMoveRight->setKey(keys.at(2));
    ui->keyInputMoveDown->setKey(keys.at(3));
    ui->keyInputReleaseCell->setKey(keys.at(4));
    ui->keyInputFlagCell->setKey(keys.at(5));
}

void LibreMinesPreferencesDialog::closeEvent(QCloseEvent *e)
{
    Q_UNUSED(e);
    this->hide();
}

void LibreMinesPreferencesDialog::hideEvent(QHideEvent *e)
{
    Q_UNUSED(e);
    Q_EMIT SIGNAL_visibilityChanged(false);
}

void LibreMinesPreferencesDialog::showEvent(QShowEvent *e)
{
    Q_UNUSED(e);
    Q_EMIT SIGNAL_visibilityChanged(true);
}

void LibreMinesPreferencesDialog::SLOT_updateCellLengthParameters()
{
    ui->sbMinimumCellLength->setMaximum(ui->sbMaximumCellLength->value());
    ui->sbMaximumCellLength->setMinimum(ui->sbMinimumCellLength->value());
}

