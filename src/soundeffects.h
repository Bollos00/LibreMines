#ifndef SOUNDEFFECTS_H
#define SOUNDEFFECTS_H

#include <QMediaPlayer>
#include <QScopedPointer>

class SoundEffects
{
public:
    SoundEffects();

    void setVolume(const int vol);

    void gameBegin();
    void gameWon();
    void gameLost();
    void clockTick();

    void keyboardControllerMove();
    void releaseCell();
    void flagCell();

private:
    void play(const QString& file_path);

    QScopedPointer<QMediaPlayer> player;
};

#endif // SOUNDEFFECTS_H
