#include "facesreaction.h"

#include "extrathemes.h"

#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QIcon>
#include <QCoreApplication>

FacesReaction::FacesReaction()
{

}

void FacesReaction::vSetFacesReactionTheme(const QString& which, const int length)
{
    if(which.compare("Disable", Qt::CaseInsensitive) == 0)
    {
        pmDizzyFace.reset( new QPixmap(length, length) );
        pmGrimacingFace.reset( new QPixmap(length, length) );
        pmGrinningFace.reset( new QPixmap(length, length) );
        pmOpenMouthFace.reset( new QPixmap(length, length) );
        pmSleepingFace.reset( new QPixmap(length, length) );
        pmSmillingFace.reset( new QPixmap(length, length) );
    }
    else
    {
        QString prefix = ":/facesreaction/faces_reaction/open-emoji-color/";
        if(which.compare("OpenEmojiColored", Qt::CaseInsensitive) == 0)
        {
            prefix = ":/facesreaction/faces_reaction/open-emoji-color/";
        }
        else
        {
            // Build list of paths to search
            QStringList searchPaths = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
            
#ifdef Q_OS_LINUX
            // Add AppImage paths if we're running as AppImage
            QString appImagePath = qgetenv("APPIMAGE");
            if(!appImagePath.isEmpty())
            {
                QString executableDir = QCoreApplication::applicationDirPath();
                searchPaths << executableDir + "/../share/libremines";
            }
#endif

            // Search all paths for the theme
            for(const QString& path : searchPaths)
            {
                QDir pathDir(path);

                // Continue if the directory does not exist
                if(!pathDir.exists() || !pathDir.cd("facesreaction_extra_themes") || !pathDir.cd(which))
                    continue;

                if(ExtraThemes::isValidTheme(pathDir.path(), ExtraThemes::FacesReaction))
                {
                    prefix = pathDir.path() + '/';
                    break;
                }
            }

            if(prefix.isNull())
            {
                qWarning() << "Faces reaction option: \"" << qPrintable(which)
                           << "\" will not be handled\n"
                           << "Using the default OpenEmojiColored instead";
                prefix = ":/facesreaction/faces_reaction/open-emoji-color/";
            }

        }

        pmDizzyFace.reset( new QPixmap( QIcon(prefix + "dizzy_face.svg").pixmap(length, length) ));
        pmGrimacingFace.reset( new QPixmap( QIcon(prefix + "grimacing_face.svg").pixmap(length, length) ));
        pmGrinningFace.reset( new QPixmap( QIcon(prefix + "grinning_face.svg").pixmap(length, length) ));
        pmOpenMouthFace.reset( new QPixmap( QIcon(prefix + "open_mouth_face.svg").pixmap(length, length) ));
        pmSleepingFace.reset( new QPixmap( QIcon(prefix + "sleeping_face.svg").pixmap(length, length) ));
        pmSmillingFace.reset( new QPixmap( QIcon(prefix + "smilling_face.svg").pixmap(length, length) ));
    }

}

const QPixmap& FacesReaction::getPixmapFromGameEvent(const GameEvent ev)
{
    switch(ev)
    {
        case FacesReaction::GAME_LOST:
            return *pmDizzyFace.get();
        case FacesReaction::UNHIDDEN_CELL_PRESSED:
            return *pmGrimacingFace.get();
        case FacesReaction::GAME_WON:
            return *pmGrinningFace.get();
        case FacesReaction::HIDDEN_CELL_PRESSED:
            return *pmOpenMouthFace.get();
        case FacesReaction::GAME_BEING_GENERATED:
            return *pmSleepingFace.get();
        case FacesReaction::DEFAULT:
            return *pmSmillingFace.get();
    }

    return *pmSleepingFace.get();
}
