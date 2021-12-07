#include "minefieldtheme.h"

#include <QDebug>
#include <QIcon>

#include "minefieldextratheme.h"

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
    else if(theme.compare("TwEmoji", Qt::CaseInsensitive) == 0)
    {
        prefix = ":/minefield_themes/twemoji/";
    }
    // If it is not one of the defaults themes, try to find it on the system.
    else
    {
        for(const QString& path :
            QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation))
        {
            QDir pathDir(path);

            // Continue if the directory does not exist
            if(!pathDir.exists() || !pathDir.cd("minefield_extra_themes") || !pathDir.cd(theme))
                continue;

            if(MinefieldExtraTheme::isValidTheme(pathDir.path()))
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
    pmBoom.reset( new QPixmap(  QIcon(prefix + "boom.svg").pixmap(cellLength, cellLength)  ) );
    pmWrongFlag.reset( new QPixmap(  QIcon(prefix + "wrong_flag.svg").pixmap(cellLength, cellLength)  ) );
}

const QPixmap& MinefieldTheme::getPixmapFromCellState(const CELL_STATE state) const
{
    switch(state)
    {
        case ZERO:
            return *pmZero.get();
        case MINE:
            return *pmMine.get();
        case ONE:
            return *pmOne.get();
        case TWO:
            return *pmTwo.get();
        case THREE:
            return *pmThree.get();
        case FOUR:
            return *pmFour.get();
        case FIVE:
            return *pmFive.get();
        case SIX:
            return *pmSix.get();
        case SEVEN:
            return *pmSeven.get();
        case EIGHT:
            return *pmEight.get();
    }

    return *pmZero.get();
}

const QPixmap& MinefieldTheme::getPixmapButton(const bool flag)
{
    return flag ? *pmFlag.get() : *pmNoFlag.get();
}

const QPixmap& MinefieldTheme::getPixmapWrongFlag()
{
    return *pmWrongFlag.get();
}

const QPixmap& MinefieldTheme::getPixmapBoom()
{
    return *pmBoom.get();
}
