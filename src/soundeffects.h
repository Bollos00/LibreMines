#ifndef SOUNDEFFECTS_H
#define SOUNDEFFECTS_H

#include <QSoundEffect>
#include <QScopedPointer>

class SoundEffects : public QObject
{
    Q_OBJECT

public:
    SoundEffects(QObject* parent = nullptr);

    void setVolume(const int vol);
    void setMuted(const bool mute);

Q_SIGNALS:

    void SIGNAL_gameBegin();
    void SIGNAL_gameWon();
    void SIGNAL_gameLost();
    void SIGNAL_clockTick();

    void SIGNAL_keyboardControllerMove();
    void SIGNAL_releaseCell();
    void SIGNAL_flagCell();

private:
    QSoundEffect soundGameBegin;
    QSoundEffect soundGameWon;
    QSoundEffect soundGameLost;
    QSoundEffect soundClockTick;

    QSoundEffect soundKeyboardControllMove;
    QSoundEffect soundReleaseCell;
    QSoundEffect soundFlagCell;

    QList<QSoundEffect*> soundEffects;
};

#endif // SOUNDEFFECTS_H
