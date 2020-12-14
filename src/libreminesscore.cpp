#include <QDataStream>
#include <QDebug>
#include "libreminesscore.h"

LibreMinesScore::LibreMinesScore():
    iTimeInNs(0),
    gameDifficulty(NONE),
    width(0),
    length(0),
    mines(0),
    bGameCompleted(false),
    dPercentageGameCompleted(0)
{

}

LibreMinesScore::operator QString() const
{
    QString strGameDiffuclty;
    if(this->gameDifficulty == EASY)
        strGameDiffuclty = "Easy";
    else if(this->gameDifficulty == MEDIUM)
        strGameDiffuclty = "Medium";
    else if(this->gameDifficulty == HARD)
        strGameDiffuclty = "Hard";
    else if(this->gameDifficulty == CUSTOMIZED)
    {
        strGameDiffuclty = "Customized " + QString::number(this->width) +
                           " x " + QString::number(this->length) + " : " +
                           QString::number(this->mines);
    }


    return  QString(this->username + '\n'
                    + "Time: " + QString::number(this->iTimeInNs*1e-9) + " secs\n"
                    + "Difficulty: " + strGameDiffuclty + '\n'
                    + "Game Completed: " + QString::number(this->dPercentageGameCompleted)) + '%';
}

bool LibreMinesScore::bFirstIsBetter(const LibreMinesScore &first, const LibreMinesScore &second)
{
    if(first.gameDifficulty != second.gameDifficulty ||
       first.width != second.width ||
       first.length != second.length)
    {
        qFatal("The two Scores must have the same game diffuculty");
    }

    if(first.bGameCompleted && second.bGameCompleted)
    {
        return first.iTimeInNs < second.iTimeInNs;
    }
    else if(first.bGameCompleted && !second.bGameCompleted)
    {
        return true;
    }
    else if(!first.bGameCompleted && second.bGameCompleted)
    {
        return false;
    }
    else
    {
        return first.dPercentageGameCompleted > second.dPercentageGameCompleted;
    }
}


QDataStream& operator<< (QDataStream& stream, const LibreMinesScore& score)
{
    stream.setVersion(QDataStream::Qt_5_12);
    //write
    stream << score.iTimeInNs;
    stream << (uchar)score.gameDifficulty;
    stream << score.width;
    stream << score.length;
    stream << score.mines;
    stream << score.bGameCompleted;
    stream << score.dPercentageGameCompleted;
    stream << score.username;

    return stream;
};

QDataStream& operator>> (QDataStream& stream, LibreMinesScore& score)
{
    stream.setVersion(QDataStream::Qt_5_12);
    //read
    stream >> score.iTimeInNs;
    uchar charGameDifficulty;
    stream >> charGameDifficulty;
    score.gameDifficulty = (GAME_DIFFICULTY)charGameDifficulty;
    stream >> score.width;
    stream >> score.length;
    stream >> score.mines;
    stream >> score.bGameCompleted;
    stream >> score.dPercentageGameCompleted;
    stream >> score.username;

    return stream;
};

QDebug operator<<(QDebug debug, const LibreMinesScore& score)
{
    QDebugStateSaver saver(debug);

    debug.nospace() << qPrintable((QString)score);
    return debug;
}
