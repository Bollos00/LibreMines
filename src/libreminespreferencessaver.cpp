#include "libreminespreferencessaver.h"

#include <QSpinBox>
#include <QCheckBox>

void LibreMinesPreferencesSaver::vLastSessionLoadConfigurationFile(
    LibreMinesPreferencesDialog& preferences, const QDir& dirAppData,
    QSpinBox& sbCustomizedPercentageMines, QSpinBox& sbCustomizedX, QSpinBox& sbCustomizedY,
    QCheckBox& cbCustomizedMinesInPercentage
)
{
    QScopedPointer<QFile> filePreferences
        ( new QFile(dirAppData.absoluteFilePath("libreminesLastSession.txt")) );

    if(filePreferences->open(QIODevice::ReadOnly))
    {
        QTextStream stream(filePreferences.get());

        while(!stream.atEnd())
        {
            QString s = stream.readLine();

            if(s.at(0) == '#')
                continue;

            QStringList terms = s.split(" ");

            if(terms.size() < 2)
                continue;

            if(terms.at(0).compare("FirstCellClean", Qt::CaseInsensitive) == 0)
            {
                if(terms.size() != 2)
                    continue;

                preferences.setOptionFirstCellClean(terms.at(1));
            }
            else if(terms.at(0).compare("ApplicationStyle", Qt::CaseInsensitive) == 0 ||
                    terms.at(0).compare("ApplicationTheme", Qt::CaseInsensitive) == 0)
            {
                if(terms.size() != 2)
                    continue;

                preferences.setOptionApplicationStyle(terms.at(1));
            }
            else if(terms.at(0).compare("ClearNeighborCellsWhenClickedOnShowedCell",
                                        Qt::CaseInsensitive) == 0)
            {
                if(terms.size() != 2)
                    continue;

                preferences.setOptionCleanNeighborCellsWhenClickedOnShowedCell(terms.at(1));
            }
            else if(terms.at(0).compare("Username", Qt::CaseInsensitive) == 0)
            {
                if(terms.size() != 2)
                    continue;

                preferences.setOptionUsername(terms.at(1));
            }
            else if(terms.at(0).compare("CustomizedPercentageOfMines", Qt::CaseInsensitive) == 0)
            {
                if(terms.size() != 2)
                    continue;

                sbCustomizedPercentageMines.setValue(terms.at(1).toInt());
            }
            else if(terms.at(0).compare("CustomizedX", Qt::CaseInsensitive) == 0)
            {
                if(terms.size() != 2)
                    continue;

                sbCustomizedX.setValue(terms.at(1).toInt());
            }
            else if(terms.at(0).compare("CustomizedY", Qt::CaseInsensitive) == 0)
            {
                if(terms.size() != 2)
                    continue;

                sbCustomizedY.setValue(terms.at(1).toInt());
            }
            else if(terms.at(0).compare("KeyboardControllerKeys", Qt::CaseInsensitive) == 0)
            {
                if(terms.size() == 7)
                {
                    preferences.setOptionKeyboardControllerKeys(
                                {
                                    terms.at(1).toInt(nullptr, 16),
                                    terms.at(2).toInt(nullptr, 16),
                                    terms.at(3).toInt(nullptr, 16),
                                    terms.at(4).toInt(nullptr, 16),
                                    terms.at(5).toInt(nullptr, 16),
                                    terms.at(6).toInt(nullptr, 16)
                                });
                }
                else if(terms.size() == 8)
                {
                    preferences.setOptionKeyboardControllerKeys(
                                {
                                    terms.at(1).toInt(nullptr, 16),
                                    terms.at(2).toInt(nullptr, 16),
                                    terms.at(3).toInt(nullptr, 16),
                                    terms.at(4).toInt(nullptr, 16),
                                    terms.at(5).toInt(nullptr, 16),
                                    terms.at(6).toInt(nullptr, 16),
                                    terms.at(7).toInt(nullptr, 16)
                                });
                }
            }
            else if(terms.at(0).compare("MinefieldTheme", Qt::CaseInsensitive) == 0)
            {
                if(terms.size() != 2)
                    continue;

                preferences.setOptionMinefieldTheme(terms.at(1));
            }
            else if(terms.at(0).compare("CustomizedMinesInPercentage", Qt::CaseInsensitive) == 0)
            {
                if(terms.size() != 2)
                    continue;

                cbCustomizedMinesInPercentage.setChecked(
                    terms.at(1).compare("On", Qt::CaseInsensitive) == 0);
            }
            else if(terms.at(0).compare("WhenCtrlIsPressed", Qt::CaseInsensitive) == 0)
            {
                if(terms.size() != 2)
                    continue;

                preferences.setOptionWhenCtrlIsPressed(terms.at(1).toInt());
            }
            else if(terms.at(0).compare("MinimumCellLength", Qt::CaseInsensitive) == 0)
            {
                if(terms.size() != 2)
                    continue;

                preferences.setOptionMinimumCellLength(terms.at(1).toInt());
            }
            else if(terms.at(0).compare("MaximumCellLength", Qt::CaseInsensitive) == 0)
            {
                if(terms.size() != 2)
                    continue;

                preferences.setOptionMaximumCellLength(terms.at(1).toInt());
            }
            else if(terms.at(0).compare("FacesReaction", Qt::CaseInsensitive) == 0)
            {
                if(terms.size() != 2)
                    continue;

                preferences.setOptionFacesReaction(terms.at(1));
            }
            else if(terms.at(0).compare("ProgressBar", Qt::CaseInsensitive) == 0)
            {
                if(terms.size() != 2)
                    continue;

                preferences.setOptionProgressBar(terms.at(1));
            }
            else if(terms.at(0).compare("MinefieldGenerationAnimation", Qt::CaseInsensitive) == 0)
            {
                if(terms.size() != 2)
                    continue;

                preferences.setOptionMinefieldGenerationAnimation(terms.at(1));
            }
            else if(terms.at(0).compare("AskToSaveMatchScoreBehaviour", Qt::CaseInsensitive) == 0)
            {
                if(terms.size() != 2)
                    continue;

                preferences.setOptionAskToSaveMatchScoreBehaviour(terms.at(1).toUInt());
            }
            else if(terms.at(0).compare("SoundVolume", Qt::CaseInsensitive) == 0)
            {
                if(terms.size() != 2)
                    continue;

                preferences.setOptionSoundVolume(terms.at(1).toInt());
            }
        }
    }


    QScopedPointer<QFile> fileLanguage
        ( new QFile(dirAppData.absoluteFilePath("libreminesDefaultLanguage.txt")) );

    if(fileLanguage->open(QIODevice::ReadOnly))
    {
        QTextStream stream(fileLanguage.get());
        QString language;
        stream >> language;

        preferences.setOptionLanguage(language);
    }

}

void LibreMinesPreferencesSaver::vLastSessionSaveConfigurationFile(
    const LibreMinesPreferencesDialog& preferences, const QDir& dirAppData,
    const QSpinBox& sbCustomizedPercentageMines, const QSpinBox& sbCustomizedX,
    const QSpinBox& sbCustomizedY,
    const QCheckBox& cbCustomizedMinesInPercentage
)
{
    QScopedPointer<QFile> fileLastSession
        ( new QFile(dirAppData.absoluteFilePath("libreminesLastSession.txt")) );

    fileLastSession->open(QIODevice::WriteOnly);
    {
        QTextStream stream(fileLastSession.get());

        stream << "FirstCellClean" << ' '
               << (preferences.optionFirstCellClean() ? "On" : "Off") << '\n'
               << "ApplicationStyle" << ' ' << preferences.optionApplicationStyle() << '\n'
               << "ClearNeighborCellsWhenClickedOnShowedCell" << ' '
               << (preferences.optionCleanNeighborCellsWhenClickedOnShowedCell() ? "On" : "Off") << '\n'
               << "Username" << ' ' << preferences.optionUsername() << '\n'
               << "CustomizedPercentageOfMines" << ' '
               << sbCustomizedPercentageMines.value() << '\n'
               << "CustomizedX" << ' ' << sbCustomizedX.value() << '\n'
               << "CustomizedY" << ' ' << sbCustomizedY.value() << '\n'
               << "KeyboardControllerKeys" << ' '
               << preferences.optionKeyboardControllerKeysString() << '\n'
               << "MinefieldTheme" << ' ' << preferences.optionMinefieldTheme() << '\n'
               << "CustomizedMinesInPercentage" << ' '
               << (cbCustomizedMinesInPercentage.isChecked() ? "On" : "Off") << '\n'
               << "WhenCtrlIsPressed" << ' ' << preferences.optionWhenCtrlIsPressed() << '\n'
               << "MinimumCellLength" << ' ' << preferences.optionMinimumCellLength() << '\n'
               << "MaximumCellLength" << ' ' << preferences.optionMaximumCellLength() << '\n'
               << "FacesReaction" << ' ' << preferences.optionFacesReaction() << '\n'
               << "ProgressBar" << ' ' << (preferences.optionProgressBar() ? "On" : "Off") << '\n'
               << "MinefieldGenerationAnimation" << ' '
               << preferences.optionMinefieldGenerationAnimationString() << '\n'
               << "AskToSaveMatchScoreBehaviour" << ' '
               << (uchar)preferences.optionAskToSaveMatchScoreBehaviour() << '\n'
               << "SoundVolume" << ' ' << preferences.optionSoundVolume() << '\n';
    }

    QScopedPointer<QFile> fileLanguage
        ( new QFile(dirAppData.absoluteFilePath("libreminesDefaultLanguage.txt")) );

    if(fileLanguage->open(QIODevice::WriteOnly))
    {
        QTextStream stream(fileLanguage.get());
        stream << preferences.optionsLanguage();
    }
}
