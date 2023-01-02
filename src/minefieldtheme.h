#ifndef MINEFIELDTHEME_H
#define MINEFIELDTHEME_H

#include <QScopedPointer>
#include <QPixmap>

#include "common.h"

class MinefieldTheme
{
public:
    MinefieldTheme();

    void vSetMinefieldTheme(const QString &theme, const int cellLength);

    const QPixmap& getPixmapFromCellState(const CELL_STATE state)const;
    const QPixmap& getPixmapButton(const bool flag);
    const QPixmap& getPixmapQuestion();
    const QPixmap& getPixmapWrongFlag();
    const QPixmap& getPixmapBoom();
private:
    QScopedPointer<QPixmap> pmMine;
    QScopedPointer<QPixmap> pmZero;
    QScopedPointer<QPixmap> pmOne;
    QScopedPointer<QPixmap> pmTwo;
    QScopedPointer<QPixmap> pmThree;
    QScopedPointer<QPixmap> pmFour;
    QScopedPointer<QPixmap> pmFive;
    QScopedPointer<QPixmap> pmSix;
    QScopedPointer<QPixmap> pmSeven;
    QScopedPointer<QPixmap> pmEight;

    QScopedPointer<QPixmap> pmFlag;
    QScopedPointer<QPixmap> pmQuestionFlag;
    QScopedPointer<QPixmap> pmNoFlag;
    QScopedPointer<QPixmap> pmBoom;
    QScopedPointer<QPixmap> pmWrongFlag;

};

#endif // MINEFIELDTHEME_H
