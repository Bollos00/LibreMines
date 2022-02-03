#ifndef SOUNDEFFECTS_H
#define SOUNDEFFECTS_H

#include <QMediaPlayer>
#include <QScopedPointer>

class SoundEffects : public QObject
{
    Q_OBJECT

public:
    SoundEffects(QObject* parent = nullptr);


    void setVolume(const int vol);
    void setMuted(const bool mute);

public Q_SLOTS:

    void SLOT_gameBegin();
    void SLOT_gameWon();
    void SLOT_gameLost();
    void SLOT_clockTick();

    void SLOT_keyboardControllerMove();
    void SLOT_releaseCell();
    void SLOT_flagCell();

Q_SIGNALS:

    void SIGNAL_gameBegin();
    void SIGNAL_gameWon();
    void SIGNAL_gameLost();
    void SIGNAL_clockTick();

    void SIGNAL_keyboardControllerMove();
    void SIGNAL_releaseCell();
    void SIGNAL_flagCell();

private:
    void play(const QString& file_path);

    QScopedPointer<QMediaPlayer> player;
};

#endif // SOUNDEFFECTS_H
