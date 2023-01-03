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


#include <QDataStream>
#include <QDebug>
#include "libreminesscore.h"

LibreMinesScore::LibreMinesScore():
    iTimeInNs(0),
    gameDifficulty(GameDifficulty::NONE),
    width(0),
    heigth(0),
    mines(0),
    bGameCompleted(false),
    dPercentageGameCompleted(0)
{

}

LibreMinesScore::operator QString() const
{
    QString strGameDiffuclty;
    if(this->gameDifficulty == GameDifficulty::EASY)
        strGameDiffuclty = "Easy";
    else if(this->gameDifficulty == GameDifficulty::MEDIUM)
        strGameDiffuclty = "Medium";
    else if(this->gameDifficulty == GameDifficulty::HARD)
        strGameDiffuclty = "Hard";
    else if(this->gameDifficulty == GameDifficulty::CUSTOMIZED)
    {
        strGameDiffuclty = "Customized " + QString::number(this->width) +
                           " x " + QString::number(this->heigth) + " : " +
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
       first.heigth != second.heigth)
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

void LibreMinesScore::sort(QList<LibreMinesScore>& l)
{
    // todo: Use iterators instead of indexes
    for(int i=0; i<l.size()-1; i++)
    {
        for(int j=i+1; j<l.size(); j++)
        {
            if(LibreMinesScore::bFirstIsBetter(l[j], l[i]))
            {
                l.move(j, i);
            }
        }
    }

}


QDataStream& operator<< (QDataStream& stream, const LibreMinesScore& score)
{
    stream.setVersion(QDataStream::Qt_5_12);
    //write
    stream << score.iTimeInNs;
    stream << (uchar)score.gameDifficulty;
    stream << score.width;
    stream << score.heigth;
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
    score.gameDifficulty = (GameDifficulty)charGameDifficulty;
    stream >> score.width;
    stream >> score.heigth;
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
