#ifndef FACESREACTION_H
#define FACESREACTION_H

#include <QScopedPointer>
#include <QPixmap>

class FacesReaction
{
public:

    enum GameEvent
    {
        GAME_LOST,
        UNHIDDEN_CELL_PRESSED,
        GAME_WON,
        HIDDEN_CELL_PRESSED,
        GAME_BEING_GENERATED,
        DEFAULT
    };

    FacesReaction();

    void vSetFacesReactionTheme(const QString &which, const int length);
    
    const QPixmap& getPixmapFromGameEvent(const GameEvent ev);

private:
    QScopedPointer<QPixmap> pmDizzyFace; /**< On game lost. */
    QScopedPointer<QPixmap> pmGrimacingFace; /**< On unhidden cell is pressed. */
    QScopedPointer<QPixmap> pmGrinningFace; /**< On game won. */
    QScopedPointer<QPixmap> pmOpenMouthFace; /**< On hidden cell is pressed. */
    QScopedPointer<QPixmap> pmSleepingFace; /**< On game being generated. */
    QScopedPointer<QPixmap> pmSmillingFace; /**< On default state. */

signals:
};

#endif // FACESREACTION_H
