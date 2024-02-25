#ifndef SOUNDEFFECTS_H
#define SOUNDEFFECTS_H

#include <QSoundEffect>
#include <QScopedPointer>

class SoundEffects : public QObject
{
    Q_OBJECT

public:
    SoundEffects(QObject* parent = nullptr);

    enum SoundType {
        GAME_BEGIN,
        GAME_WON,
        GAME_LOST,
        KEYBOARD_CONTROLLER_MOVE,
        RELEASE_CELL,
        FLAG_CELL
    };

public Q_SLOTS:
    void SLOT_playSound(const SoundType type);
    void SLOT_setVolume(const int vol);

private:
    QSoundEffect soundGameBegin;
    QSoundEffect soundGameWon;
    QSoundEffect soundGameLost;

    QSoundEffect soundKeyboardControllMove;
    QSoundEffect soundReleaseCell;
    QSoundEffect soundFlagCell;

    QList<QSoundEffect*> soundEffects;
};

Q_DECLARE_METATYPE(SoundEffects::SoundType)

#endif // SOUNDEFFECTS_H
