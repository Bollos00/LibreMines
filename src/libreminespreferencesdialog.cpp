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


#include "libreminespreferencesdialog.h"
#include "ui_libreminespreferencesdialog.h"

#include <QStyleFactory>
#include <QMessageBox>
#include <QSoundEffect>

#include "extrathemes.h"

LibreMinesPreferencesDialog::LibreMinesPreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LibreMinesPreferencesDialog),
    updateLanguageDialog(true)
{
    ui->setupUi(this);

    // Default system theme
    ui->comboBoxApplicationStyle->addItem("default");

    // Dark and light fusion
    ui->comboBoxApplicationStyle->addItems({"Fusion Dark", "Fusion Light"});

    // Styles from system
    ui->comboBoxApplicationStyle->addItems(QStyleFactory::keys());

    // Default Minefield themes
    ui->comboBoxMinefieldTheme->addItems({"Classic Dark", "Classic Light"});
    ui->comboBoxMinefieldTheme->addItems(ExtraThemes::getExtraThemes(ExtraThemes::Minefield));

    ui->comboBoxFacesReaction->addItems({"Open Emoji Colored", "Disable"});
    ui->comboBoxFacesReaction->addItems(ExtraThemes::getExtraThemes(ExtraThemes::FacesReaction));


    ui->comboBoxWhenCtrlIsPressed->addItems({tr("Go to the Edge"), tr("Jump 3 Cells"),
                                             tr("Jump 5 Cells"), tr("Jump 10 Cells")});

    ui->comboBoxLanguage->addItems({"English", "Português do Brasil"});

    ui->comboBoxMinefieldGenerationAnimation->addItems({tr("On"), tr("Limited"), tr("Off")});


    // Space character is not allowed. This will remove every space character of the line edit
//    ui->lineEditUsername->setValidator(new QRegExpValidator(QRegExp("^(?i)[a-z][a-z0-9]*$")));
    connect(ui->lineEditUsername, &QLineEdit::textChanged,
            this, [this](QString text)
    { ui->lineEditUsername->setText(text.remove(" ", Qt::CaseInsensitive)); });

    connect(ui->comboBoxApplicationStyle, &QComboBox::currentTextChanged,
            this, [this](QString text)
    {
        text.remove(" ", Qt::CaseInsensitive);
        Q_EMIT SIGNAL_optionChanged("ApplicationTheme", text);
    });

    connect(ui->comboBoxMinefieldTheme, &QComboBox::currentTextChanged,
            this, [this](QString text)
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

AskToSaveMatchScore LibreMinesPreferencesDialog::optionAskToSaveMatchScoreBehaviour() const
{
    if(ui->rbSaveScoreNever->isChecked())
        return LibreMines::SaveNever;
    if(ui->rbSaveScoreAlways->isChecked())
        return LibreMines::SaveAlways;
    if(ui->rbSaveScoreWhenHighScore->isChecked())
        return LibreMines::SaveWhenNewHighScore;
    if(ui->rbSaveScoreWhenGameCompleted->isChecked())
        return LibreMines::SaveWhenGameCompleted;
    if(ui->rbSaveScoreWhenHighScoreAndGameCompleted->isChecked())
        return LibreMines::SaveWhenGameCompleted | LibreMines::SaveWhenNewHighScore;
    return LibreMines::SaveNever;
}

int LibreMinesPreferencesDialog::optionSoundVolume() const
{
    return ui->sliderSoundVolume->value();
}

bool LibreMinesPreferencesDialog::optionUseQuestionMark() const
{
    return ui->cbUseQuestionMark->isChecked();
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
    
    ui->comboBoxApplicationStyle->setCurrentText(s);
}

void LibreMinesPreferencesDialog::setOptionMinefieldTheme(const QString &option)
{
    QString s = option;
    if(option.compare("ClassicLight", Qt::CaseInsensitive) == 0){ s = "Classic Light"; }
    else if(option.compare("ClassicDark", Qt::CaseInsensitive) == 0){ s = "Classic Dark"; }

    ui->comboBoxMinefieldTheme->setCurrentText(s);
}

void LibreMinesPreferencesDialog::setOptionFacesReaction(const QString &option)
{
    QString s = option;
    if(option.compare("OpenEmojiColored", Qt::CaseInsensitive) == 0){ s = "Open Emoji Colored"; }

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
    QTimer::singleShot(10, this, [this](){ updateLanguageDialog = true; });
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

void LibreMinesPreferencesDialog::setOptionAskToSaveMatchScoreBehaviour(const uchar option)
{
    switch(option)
    {
        case LibreMines::SaveNever:
            return ui->rbSaveScoreNever->setChecked(true);
        case LibreMines::SaveWhenNewHighScore:
            return ui->rbSaveScoreWhenHighScore->setChecked(true);
        case LibreMines::SaveWhenGameCompleted:
            return ui->rbSaveScoreWhenGameCompleted->setChecked(true);
        case LibreMines::SaveWhenGameCompleted | LibreMines::SaveWhenNewHighScore:
            return ui->rbSaveScoreWhenHighScoreAndGameCompleted->setChecked(true);
        case LibreMines::SaveAlways:
            return ui->rbSaveScoreAlways->setChecked(true);
    }

    return ui->rbSaveScoreNever->setChecked(true);
}

void LibreMinesPreferencesDialog::setOptionSoundVolume(const int option)
{
    ui->sliderSoundVolume->setValue(option);
    ui->labelSoundVolume->setText(QString::number(option));

    ui->cbSoundMute->setChecked(option == 0);
}

void LibreMinesPreferencesDialog::setOptionUseQuestionMark(const QString& option)
{
    ui->cbUseQuestionMark->setChecked(option.compare("On", Qt::CaseInsensitive) == 0);
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


void LibreMinesPreferencesDialog::on_cbSoundMute_stateChanged(int arg1)
{
    // If muted, the volume goes to zero and become unchaged until it is unmuted again
    if(arg1 == Qt::Checked)
    {
        ui->sliderSoundVolume->setValue(0);
        ui->labelSoundVolume->setText(QString::number(0));
        ui->sliderSoundVolume->setEnabled(false);
    }
    else
    {
        ui->sliderSoundVolume->setEnabled(true);
    }
}


void LibreMinesPreferencesDialog::on_sliderSoundVolume_valueChanged(int value)
{
    static bool firstTime = true;

    ui->labelSoundVolume->setText(QString::number(value));

    if(value != 0 && !firstTime)
    {
        static QSoundEffect sound;
        sound.setSource(QUrl("qrc:/sound_effects/move.wav"));
        sound.setVolume(value/100.f);
        sound.play();
    }

    firstTime = false;
}

