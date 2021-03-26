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

    ui->comboBoxApplicationTheme->addItems({"Dark", "Light"});
    ui->comboBoxMinefieldTheme->addItems({"Classic Dark", "Classic Light"});
    ui->comboBoxWhenCtrlIsPressed->addItems({"Go to the Edge", "Jump 3 Cells", "Jump 5 Cells", "Jump 10 Cells"});


    // Space character is not allowed. This will remove every space character of the line edit
//    ui->lineEditUsername->setValidator(new QRegExpValidator(QRegExp("^(?i)[a-z][a-z0-9]*$")));
    connect(ui->lineEditUsername, &QLineEdit::textChanged,
            [this](QString text)
    { ui->lineEditUsername->setText(text.remove(" ", Qt::CaseInsensitive)); });

    connect(ui->comboBoxApplicationTheme, &QComboBox::currentTextChanged,
            [this](const QString& text)
    { Q_EMIT SIGNAL_optionChanged("ApplicationTheme", text); });

    connect(ui->comboBoxMinefieldTheme, &QComboBox::currentTextChanged,
            [this](QString text)
    {
        text.remove(" ", Qt::CaseInsensitive);
        Q_EMIT SIGNAL_optionChanged("MinefieldTheme", text);
    });

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

QString LibreMinesPreferencesDialog::optionApplicationTheme() const
{
    return ui->comboBoxApplicationTheme->currentText();
}

QString LibreMinesPreferencesDialog::optionMinefieldTheme() const
{
    QString s = ui->comboBoxMinefieldTheme->currentText();
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

void LibreMinesPreferencesDialog::setOptionFirstCellClean(const QString &option)
{
    ui->cbFirstCellClean->setChecked(option.compare("On", Qt::CaseInsensitive) == 0);
}

void LibreMinesPreferencesDialog::setOptionCleanNeighborCellsWhenClickedOnShowedCell(const QString &option)
{
    ui->cbCleanNeighborCellsWhenClickedOnShowedCell->setChecked(option.compare("On", Qt::CaseInsensitive) == 0);
}

void LibreMinesPreferencesDialog::setOptionApplicationTheme(const QString &theme)
{
    ui->comboBoxApplicationTheme->setCurrentText(theme);
}

void LibreMinesPreferencesDialog::setOptionMinefieldTheme(const QString &theme)
{
    QString s;
    if(theme == "ClassicLight"){ s = "Classic Light"; }
    else if(theme == "ClassicDark"){ s = "Classic Dark"; }

    ui->comboBoxMinefieldTheme->setCurrentText(s);
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

