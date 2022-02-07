/*****************************************************************************
 * LibreMines                                                                *
 * Copyright (C) 2020-2022  Bruno Bollos Correa                              *
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


#ifndef LIBREMINESPREFERENCESDIALOG_H
#define LIBREMINESPREFERENCESDIALOG_H

#include <QDialog>
#include <QtCore>

namespace Ui {
class LibreMinesPreferencesDialog;
}

namespace LibreMines
{
enum WhenCtrlIsPressedOptions : uchar
{
    GoToTheEdge = 0,
    Jump3Cells = 1,
    Jump5Cells = 2,
    Jump10Cells = 3
};

enum MinefieldGenerationAnimation : uchar
{
    AnimationOn = 0,
    AnimationLimited = 1,
    AnimationOff = 2
};

enum AskToSaveMatchScore : uchar
{
    SaveNever              = 0x00,
    SaveWhenNewHighScore   = 0x01,
    SaveWhenGameCompleted  = 0x02,
    SaveAlways             = 0x04
};

}
Q_DECLARE_FLAGS(AskToSaveMatchScore, LibreMines::AskToSaveMatchScore)
Q_DECLARE_OPERATORS_FOR_FLAGS(AskToSaveMatchScore)

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

Q_SIGNALS:
    void SIGNAL_optionChanged(const QString& name, const QString& value);
    void SIGNAL_visibilityChanged(const bool visible);
};

#endif // LIBREMINESPREFERENCESDIALOG_H
