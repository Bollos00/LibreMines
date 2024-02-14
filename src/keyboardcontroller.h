#ifndef KEYBOARDCONTROLLER_H
#define KEYBOARDCONTROLLER_H

#include <QEvent>

#include <libreminesgameengine.h>
class LibreMinesGui;

class KeyboardController
{
public:
    KeyboardController();

    bool handleEvent(QEvent* e, LibreMinesGui* gui);

    bool isValid() const;

    int getKeyLeft() const;
    int getKeyDown() const;
    int getKeyRight() const;
    int getKeyUp() const;
    int getKeyReleaseCell() const;
    int getKeyFlagCell() const;
    int getKeyCenterCell() const;

    void setKeys(QList<int> keys);

    bool isActive() const;

    void refresh(uchar x, uchar y, LibreMinesGui* gui);
    void deactivate(LibreMinesGui* gui);

private:
    void activate(LibreMinesGui* gui);

    void setCurrentCell(const uchar x, const uchar y, LibreMinesGui* gui);
    void unsetCurrentCell(LibreMinesGui* gui);
    void moveLeft(LibreMinesGui* gui);
    void moveRight(LibreMinesGui* gui);
    void moveDown(LibreMinesGui* gui);
    void moveUp(LibreMinesGui* gui);
    void centerCurrentCell(LibreMinesGui* gui);

    bool ctrlPressed;
    bool active;
    uchar currentX;
    uchar currentY;

    int keyLeft;
    int keyDown;
    int keyRight;
    int keyUp;
    int keyReleaseCell;
    int keyFlagCell;
    int keyCenterCell;

    bool keysVaild;
};

#endif // KEYBOARDCONTROLLER_H
