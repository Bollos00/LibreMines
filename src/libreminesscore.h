#ifndef LIBREMINESSCORE_H
#define LIBREMINESSCORE_H

#include "common.h"

class LibreMinesScore
{
public:
    LibreMinesScore();

    operator QString() const;

    qint64 iTimeInNs;
    GAME_DIFFICULTY gameDifficulty;
    uchar width;
    uchar length;
    ushort mines;
    bool bGameCompleted;
    double dPercentageGameCompleted;
    QString username;

    static bool bFirstIsBetter(const LibreMinesScore& first, const LibreMinesScore& second);
};

QDataStream& operator<< (QDataStream& stream, const LibreMinesScore& score);
QDataStream& operator>> (QDataStream& stream, LibreMinesScore& score);
QDebug operator<<(QDebug debug, const LibreMinesScore& score);

#endif // LIBREMINESSCORE_H
