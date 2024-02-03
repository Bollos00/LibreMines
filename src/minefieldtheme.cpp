#include "minefieldtheme.h"

#include <QDebug>
#include <QIcon>

#include "extrathemes.h"

MinefieldTheme::MinefieldTheme()
{

}

void MinefieldTheme::vSetMinefieldTheme(const QString& theme, const int cellLength)
{
    QString prefix;
    if(theme.compare("ClassicLight", Qt::CaseInsensitive) == 0)
    {
        prefix = ":/minefield_themes/classic_light/";
    }
    else if(theme.compare("ClassicDark", Qt::CaseInsensitive) == 0)
    {
        prefix = ":/minefield_themes/classic_dark/";
    }
    // If it is not one of the default themes, try to find it on the system.
    else
    {
        for(const QString& path :
            QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation))
        {
            QDir pathDir(path);

            // Continue if the directory does not exist
            if(!pathDir.exists() || !pathDir.cd("minefield_extra_themes") || !pathDir.cd(theme))
                continue;

            if(ExtraThemes::isValidTheme(pathDir.path(), ExtraThemes::Minefield))
            {
                prefix = pathDir.path() + '/';
                break;
            }
        }

        if(prefix.isNull())
        {
            qWarning() << "Minefield selected " << theme << " does not exist.\n"
                       << "Using the default Classic Dark instead";

            prefix = ":/minefield_themes/classic_dark/";
        }
    }

    pmMine.reset( new QPixmap(  QIcon(prefix + "mine.svg").pixmap(cellLength, cellLength)  ) );
    pmZero.reset( new QPixmap(  QIcon(prefix + "0.svg").pixmap(cellLength, cellLength)  ) );
    pmOne.reset( new QPixmap(  QIcon(prefix + "1.svg").pixmap(cellLength, cellLength)  ) );
    pmTwo.reset( new QPixmap(  QIcon(prefix + "2.svg").pixmap(cellLength, cellLength)  ) );
    pmThree.reset( new QPixmap(  QIcon(prefix + "3.svg").pixmap(cellLength, cellLength)  ) );
    pmFour.reset( new QPixmap(  QIcon(prefix + "4.svg").pixmap(cellLength, cellLength)  ) );
    pmFive.reset( new QPixmap(  QIcon(prefix + "5.svg").pixmap(cellLength, cellLength)  ) );
    pmSix.reset( new QPixmap(  QIcon(prefix + "6.svg").pixmap(cellLength, cellLength)  ) );
    pmSeven.reset( new QPixmap(  QIcon(prefix + "7.svg").pixmap(cellLength, cellLength)  ) );
    pmEight.reset( new QPixmap(  QIcon(prefix + "8.svg").pixmap(cellLength, cellLength)  ) );

    pmFlag.reset( new QPixmap(  QIcon(prefix + "flag.svg").pixmap(cellLength, cellLength)  ) );
    pmNoFlag.reset( new QPixmap(  QIcon(prefix + "no_flag.svg").pixmap(cellLength, cellLength)  ) );
	pmQuestionFlag.reset( new QPixmap(  QIcon(prefix + "question_flag.svg").pixmap(cellLength, cellLength)  ) );
    pmBoom.reset( new QPixmap(  QIcon(prefix + "boom.svg").pixmap(cellLength, cellLength)  ) );
    pmWrongFlag.reset( new QPixmap(  QIcon(prefix + "wrong_flag.svg").pixmap(cellLength, cellLength)  ) );
}

const QPixmap& MinefieldTheme::getPixmapFromCellValue(const CellValue value) const
{
    switch(value)
    {
        case CellValue::ZERO:
            return *pmZero.get();
        case CellValue::MINE:
            return *pmMine.get();
        case CellValue::ONE:
            return *pmOne.get();
        case CellValue::TWO:
            return *pmTwo.get();
        case CellValue::THREE:
            return *pmThree.get();
        case CellValue::FOUR:
            return *pmFour.get();
        case CellValue::FIVE:
            return *pmFive.get();
        case CellValue::SIX:
            return *pmSix.get();
        case CellValue::SEVEN:
            return *pmSeven.get();
        case CellValue::EIGHT:
            return *pmEight.get();
    }

    return *pmZero.get();
}

const QPixmap& MinefieldTheme::getPixmapButton(const FlagState state)
{
    switch (state) {
        case FlagState::NoFlag:
            return *pmNoFlag.get();
        case FlagState::HasFlag:
            return *pmFlag.get();
        case FlagState::Question:
            return *pmQuestionFlag.get();
    }
    return *pmNoFlag.get();
}

const QPixmap& MinefieldTheme::getPixmapQuestion()
{
	return *pmQuestionFlag.get();
}

const QPixmap& MinefieldTheme::getPixmapWrongFlag()
{
    return *pmWrongFlag.get();
}

const QPixmap& MinefieldTheme::getPixmapBoom()
{
    return *pmBoom.get();
}
