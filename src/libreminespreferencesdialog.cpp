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

#include <QStyleFactory>
#include <QMessageBox>

LibreMinesPreferencesDialog::LibreMinesPreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LibreMinesPreferencesDialog),
    updateLanguageDialog(true)
{
    ui->setupUi(this);

    // Dark and light fusion
    ui->comboBoxApplicationStyle->addItems({"Fusion Dark", "Fusion Light"});


    // QSS
    ui->comboBoxApplicationStyle->addItems({"ConsoleStyle", "NeonButtons"});

    // QDarkStyle
    ui->comboBoxApplicationStyle->addItems({"QDarkStyle", "QDarkStyle Light"});

    // Breeze
    ui->comboBoxApplicationStyle->addItems({"Breeze Dark", "Breeze Light"});

    // Styles from system
    ui->comboBoxApplicationStyle->addItems(QStyleFactory::keys());

    ui->comboBoxMinefieldTheme->addItems({"Classic Dark", "Classic Light", "TwEmoji"});
    ui->comboBoxFacesReaction->addItems({"Open Emoji Colored", "Open Emoji Black", "Open Emoji White",
                                         "TwEmoji Colored", "Disable"});
    ui->comboBoxWhenCtrlIsPressed->addItems({tr("Go to the Edge"), tr("Jump 3 Cells"), tr("Jump 5 Cells"), tr("Jump 10 Cells")});

    ui->comboBoxLanguage->addItems({"English", "Português do Brasil"});

    ui->comboBoxMinefieldGenerationAnimation->addItems({tr("On"), tr("Limited"), tr("Off")});


    // Space character is not allowed. This will remove every space character of the line edit
//    ui->lineEditUsername->setValidator(new QRegExpValidator(QRegExp("^(?i)[a-z][a-z0-9]*$")));
    connect(ui->lineEditUsername, &QLineEdit::textChanged,
            [this](QString text)
    { ui->lineEditUsername->setText(text.remove(" ", Qt::CaseInsensitive)); });

    connect(ui->comboBoxApplicationStyle, &QComboBox::currentTextChanged,
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

    connect(ui->comboBoxLanguage, &QComboBox::currentTextChanged,
            this, &LibreMinesPreferencesDialog::SLOT_updateLanguage);

    ui->keyInputMoveLeft->setKey(Qt::Key_A);
    ui->keyInputMoveUp->setKey(Qt::Key_W);
    ui->keyInputMoveRight->setKey(Qt::Key_D);
    ui->keyInputMoveDown->setKey(Qt::Key_S);
    ui->keyInputReleaseCell->setKey(Qt::Key_O);
    ui->keyInputFlagCell->setKey(Qt::Key_P);
    ui->keyInputCenterCell->setKey(Qt::Key_Space);
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

QString LibreMinesPreferencesDialog::optionApplicationStyle() const
{
    QString s = ui->comboBoxApplicationStyle->currentText();
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

QString LibreMinesPreferencesDialog::optionsLanguage() const
{
    if(ui->comboBoxLanguage->currentText().compare("English", Qt::CaseInsensitive) == 0)
        return "en";
    if(ui->comboBoxLanguage->currentText().compare("Português do Brasil", Qt::CaseInsensitive) == 0)
        return "pt_BR";
    return "";
}

uchar LibreMinesPreferencesDialog::optionMinefieldGenerationAnimation() const
{
    return ui->comboBoxMinefieldGenerationAnimation->currentIndex();
}

QString LibreMinesPreferencesDialog::optionMinefieldGenerationAnimationString() const
{
    return ui->comboBoxMinefieldGenerationAnimation->currentText();
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

void LibreMinesPreferencesDialog::setOptionApplicationStyle(const QString &option)
{
    QString s = option;
    if(option.compare("FusionDark", Qt::CaseInsensitive) == 0){ s = "Fusion Dark"; }
    else if(option.compare("FusionLight", Qt::CaseInsensitive) == 0){ s = "Fusion Light"; }
    else if(option.compare("QDarkStyleLight", Qt::CaseInsensitive) == 0){ s = "QDarkStyle Light"; }
    else if(option.compare("BreezeDark", Qt::CaseInsensitive) == 0){ s = "Breeze Dark"; }
    else if(option.compare("BreezeLight", Qt::CaseInsensitive) == 0){ s = "Breeze Light"; }

    ui->comboBoxApplicationStyle->setCurrentText(s);
}

void LibreMinesPreferencesDialog::setOptionMinefieldTheme(const QString &option)
{
    QString s = option;
    if(option.compare("ClassicLight", Qt::CaseInsensitive) == 0){ s = "Classic Light"; }
    else if(option.compare("ClassicDark", Qt::CaseInsensitive) == 0){ s = "Classic Dark"; }
    else if(option.compare("TwEmoji", Qt::CaseInsensitive) == 0){ s = "TwEmoji"; }

    ui->comboBoxMinefieldTheme->setCurrentText(s);
}

void LibreMinesPreferencesDialog::setOptionFacesReaction(const QString &option)
{
    QString s = "Disable";
    if(option.compare("OpenEmojiColored", Qt::CaseInsensitive) == 0){ s = "Open Emoji Colored"; }
    else if(option.compare("OpenEmojiBlack", Qt::CaseInsensitive) == 0){ s = "Open Emoji Black"; }
    else if(option.compare("OpenEmojiWhite", Qt::CaseInsensitive) == 0){ s = "Open Emoji White"; }
    else if(option.compare("TwEmojiColored", Qt::CaseInsensitive) == 0){ s = "TwEmoji Colored"; }

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

void LibreMinesPreferencesDialog::setOptionLanguage(const QString &option)
{
    QString s = "English";
    if(option.compare("pt_BR", Qt::CaseInsensitive) == 0)
        s = "Português do Brasil";

    // Ugly way to avoid the creation of the dialog when this function is called
    updateLanguageDialog = false;
    ui->comboBoxLanguage->setCurrentText(s);
    QTimer::singleShot(10, [this](){ updateLanguageDialog = true; });
}

void LibreMinesPreferencesDialog::setOptionMinefieldGenerationAnimation(const uchar option)
{
    if(ui->comboBoxMinefieldGenerationAnimation->count() > option)
        ui->comboBoxMinefieldGenerationAnimation->setCurrentIndex(option);
}

void LibreMinesPreferencesDialog::setOptionMinefieldGenerationAnimation(const QString &option)
{
    ui->comboBoxMinefieldGenerationAnimation->setCurrentText(option);
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
        ui->keyInputFlagCell->currentKey(),
        ui->keyInputCenterCell->currentKey()
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
    s += ' ';
    s += QString::number(ui->keyInputCenterCell->currentKey(), 16);

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
    if(keys.size() == 7)
    {
        ui->keyInputCenterCell->setKey(keys.at(6));
    }
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

void LibreMinesPreferencesDialog::SLOT_updateLanguage()
{
    if(updateLanguageDialog)
    {
        QMessageBox::information(this, tr("Restart LibreMines to apply this preference!"),
                                 tr("Please restart the application to redefine your language"));
    }
}

