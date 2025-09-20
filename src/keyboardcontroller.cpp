#include "keyboardcontroller.h"

#include <QKeyEvent>

#include "libreminesgui.h"

KeyboardController::KeyboardController() :
    ctrlPressed(false),
    active(false),
    currentX(0),
    currentY(0),
    keysVaild(false)
{

}

bool KeyboardController::handleEvent(QEvent* e, LibreMinesGui* gui)
{
    // If the keyboard controller has invalid settings
    if(!isValid())
        return false;


    // Lock the cursor on the lower left of the screen while the controller is activated
    //    if(active)
    //    {
    //        qApp->overrideCursor()->setPos(90*qApp->primaryScreen()->geometry().width()/100,
    //                                       90*qApp->primaryScreen()->geometry().height()/100);
    //    }

    switch(e->type())
    {
        case QEvent::KeyPress:
        {
            Qt::Key key = (Qt::Key)((QKeyEvent*)e)->key();

            if(key == Qt::Key_Control)
            {
                ctrlPressed = true;
                return true;
            }
            if(isActive())
            {
                if(key == keyLeft ||
                    key == keyUp ||
                    key == keyDown ||
                    key == keyRight ||
                    key == keyCenterCell ||
                    key == keyFlagCell ||
                    key == keyReleaseCell)
                {
                    return true;
                }
            }

        }break;

        case QEvent::KeyRelease:
        {
            Qt::Key key = (Qt::Key)((QKeyEvent*)e)->key();

            if(key == Qt::Key_Control)
            {
                ctrlPressed = false;
                return true;
            }

            // Active the controller depending on the key
            // Additionally hide the cursor when the controller is activated
            if(!isActive())
            {
                if(key == keyLeft ||
                    key == keyUp ||
                    key == keyDown ||
                    key == keyRight)
                {
                    activate(gui);

                    return true;
                }
            }
            else
            {
                if(key == keyLeft ||
                    key == keyUp ||
                    key == keyDown ||
                    key == keyRight ||
                    key == keyCenterCell ||
                    key == keyFlagCell ||
                    key == keyReleaseCell)
                {
                    gui->setFocus();
                }


                if(key == keyLeft)
                {
                    moveLeft(gui);
                    return true;
                }
                if(key == keyUp)
                {
                    moveUp(gui);
                    return true;
                }
                if(key == keyDown)
                {
                    moveDown(gui);
                    return true;
                }
                if(key == keyRight)
                {
                    moveRight(gui);
                    return true;
                }
                if(key == keyReleaseCell)
                {
                    const LibreMinesGameEngine::CellGameEngine& cell =
                        gui->gameEngine->getPrincipalMatrix()[currentX][currentY];

                    if(cell.isHidden)
                    {
                        Q_EMIT gui->SIGNAL_cleanCell(currentX, currentY);
                        return true;
                    }
                    if(gui->preferences->optionCleanNeighborCellsWhenClickedOnShowedCell())
                    {
                        Q_EMIT gui->SIGNAL_cleanNeighborCells(currentX, currentY);
                        return true;
                    }

                }
                if(key == keyFlagCell)
                {
                    Q_EMIT gui->SIGNAL_addOrRemoveFlag(currentX, currentY);
                    return true;
                }
                if(key == Qt::Key_Space)
                {
                    centerCurrentCell(gui);
                    return true;
                }
                if(key == Qt::Key_Escape)
                {
                    deactivate(gui);
                    return true;
                }
            }

        }break;

        default:
            break;
    }

    return false;
}

bool KeyboardController::isValid() const
{
    return keysVaild;
}

int KeyboardController::getKeyLeft() const
{
    return keyLeft;
}

int KeyboardController::getKeyDown() const
{
    return keyDown;
}

int KeyboardController::getKeyRight() const
{
    return keyRight;
}

int KeyboardController::getKeyUp() const
{
    return keyUp;
}

int KeyboardController::getKeyReleaseCell() const
{
    return keyReleaseCell;
}

int KeyboardController::getKeyFlagCell() const
{
    return keyFlagCell;
}

int KeyboardController::getKeyCenterCell() const
{
    return keyCenterCell;
}

void KeyboardController::setKeys(QList<int> keys)
{
    keyLeft = keys[0];
    keyUp = keys[1];
    keyRight = keys[2];
    keyDown = keys[3];
    keyReleaseCell = keys[4];
    keyFlagCell = keys[5];
    keyCenterCell = keys[6];

    keysVaild = true;
    for (int i=0; i<keys.size()-1; ++i)
    {
        for(int j=i+1; j<keys.size(); ++j)
        {
            keysVaild = keysVaild && (keys[i] != keys[j]);
        }
    }
    keysVaild = keysVaild && (!keys.contains(-1));

}


void KeyboardController::setCurrentCell(const uchar x, const uchar y, LibreMinesGui* gui)
{
    currentX = x;
    currentY = y;

    const LibreMinesGameEngine::CellGameEngine& cellGE = gui->gameEngine->getPrincipalMatrix()[currentX][currentY];
    LibreMinesGui::CellGui& cellGui = gui->principalMatrix[currentX][currentY];

    if(cellGE.isHidden)
    {
        cellGui.button->setPixmapInverted();
    }
    else
    {
        cellGui.label->setPixmapInverted();
    }

    gui->scrollAreaBoard->ensureVisible(
        x*gui->cellLength + gui->cellLength/2, y*gui->cellLength + gui->cellLength/2,
        gui->cellLength/2 + 1, gui->cellLength/2 + 1
    );
}

void KeyboardController::unsetCurrentCell(LibreMinesGui* gui)
{
    const LibreMinesGameEngine::CellGameEngine& cellGE = gui->gameEngine->getPrincipalMatrix()[currentX][currentY];
    LibreMinesGui::CellGui& cellGui = gui->principalMatrix[currentX][currentY];

    if(cellGE.isHidden)
    {
        cellGui.button->setPixmapNormal();
    }
    else
    {
        cellGui.label->setPixmapNormal();
    }
}

void KeyboardController::moveLeft(LibreMinesGui* gui)
{
    unsetCurrentCell(gui);

    uchar destX = 0;

    if(ctrlPressed)
    {
        if(gui->preferences->optionWhenCtrlIsPressed() == LibreMines::Jump3Cells &&
            currentX >= 3)
        {
            destX = currentX - 3;
        }
        else if(gui->preferences->optionWhenCtrlIsPressed() == LibreMines::Jump5Cells &&
                 currentX >= 5)
        {
            destX = currentX - 5;
        }
        else if(gui->preferences->optionWhenCtrlIsPressed() == LibreMines::Jump10Cells &&
                 currentX >= 10)
        {
            destX = currentX - 10;
        }
//        else
//        {
//            destX = 0;
//        }
    }
    else
    {
        destX = (currentX == 0) ? gui->gameEngine->rows() - 1 : currentX - 1;
    }

    setCurrentCell(destX, currentY, gui);

    Q_EMIT gui->SIGNAL_playSoundEffect(SoundEffects::KEYBOARD_CONTROLLER_MOVE);
}

void KeyboardController::moveRight(LibreMinesGui* gui)
{
    unsetCurrentCell(gui);

    uchar destX = gui->gameEngine->rows() - 1;

    if(ctrlPressed)
    {
        if(gui->preferences->optionWhenCtrlIsPressed() == LibreMines::Jump3Cells &&
            currentX < gui->gameEngine->rows() - 3)
        {
            destX = currentX + 3;
        }
        else if(gui->preferences->optionWhenCtrlIsPressed() == LibreMines::Jump5Cells &&
                 currentX < gui->gameEngine->rows() - 5)
        {
            destX = currentX + 5;
        }
        else if(gui->preferences->optionWhenCtrlIsPressed() == LibreMines::Jump10Cells &&
                 currentX < gui->gameEngine->rows() - 10)
        {
            destX = currentX + 10;
        }
    }
    else
    {
        destX = (currentX == gui->gameEngine->rows() - 1) ? 0 : (currentX + 1);
    }

    setCurrentCell(destX, currentY, gui);

    Q_EMIT gui->SIGNAL_playSoundEffect(SoundEffects::KEYBOARD_CONTROLLER_MOVE);
}

void KeyboardController::moveDown(LibreMinesGui* gui)
{
    unsetCurrentCell(gui);

    uchar destY = gui->gameEngine->lines() - 1;

    if(ctrlPressed)
    {
        if(gui->preferences->optionWhenCtrlIsPressed() == LibreMines::Jump3Cells &&
            currentY < gui->gameEngine->lines() - 3)
        {
            destY = currentY + 3;
        }
        else if(gui->preferences->optionWhenCtrlIsPressed() == LibreMines::Jump5Cells &&
                 currentY < gui->gameEngine->lines() - 5)
        {
            destY = currentY + 5;
        }
        else if(gui->preferences->optionWhenCtrlIsPressed() == LibreMines::Jump10Cells &&
                 currentY < gui->gameEngine->lines() - 10)
        {
            destY = currentY + 10;
        }
    }
    else
    {
        destY = (currentY == gui->gameEngine->lines() - 1) ? 0 : (currentY + 1);
    }

    setCurrentCell(currentX, destY, gui);

    Q_EMIT gui->SIGNAL_playSoundEffect(SoundEffects::KEYBOARD_CONTROLLER_MOVE);
}

void KeyboardController::moveUp(LibreMinesGui* gui)
{
    unsetCurrentCell(gui);

    uchar destY = 0;

    if(ctrlPressed)
    {
        if(gui->preferences->optionWhenCtrlIsPressed() == LibreMines::Jump3Cells &&
            currentY >= 3)
        {
            destY = currentY - 3;
        }
        else if(gui->preferences->optionWhenCtrlIsPressed() == LibreMines::Jump5Cells &&
                 currentY >= 5)
        {
            destY = currentY - 5;
        }
        else if(gui->preferences->optionWhenCtrlIsPressed() == LibreMines::Jump10Cells &&
                 currentY >= 10)
        {
            destY = currentY - 10;
        }
    }
    else
    {
        destY = (currentY == 0) ? gui->gameEngine->lines() - 1 : currentY - 1;
    }

    setCurrentCell(currentX, destY, gui);

    Q_EMIT gui->SIGNAL_playSoundEffect(SoundEffects::KEYBOARD_CONTROLLER_MOVE);
}

void KeyboardController::centerCurrentCell(LibreMinesGui* gui)
{
    const uchar x = currentX;
    const uchar y = currentY;
    gui->scrollAreaBoard->ensureVisible(
        x*gui->cellLength + gui->cellLength/2,
        y*gui->cellLength + gui->cellLength/2,
        gui->cellLength/2 + gui->scrollAreaBoard->width()/2,
        gui->cellLength/2 + gui->scrollAreaBoard->height()/2
    );

}

bool KeyboardController::isActive() const
{
    return active;
}

// This functions is needed when an unhidden cell is pressed,
//  so it will invert the color of the label
void KeyboardController::refresh(uchar x, uchar y, LibreMinesGui* gui)
{
    if(isActive() && currentX == x && currentY == y)
    {
        unsetCurrentCell(gui);
        setCurrentCell(currentX, currentY, gui);
    }

}

void KeyboardController::deactivate(LibreMinesGui* gui)
{
    if(isActive())
    {
        active = false;
        unsetCurrentCell(gui);
        qApp->restoreOverrideCursor();
    }
}

void KeyboardController::activate(LibreMinesGui* gui)
{
    if(!isActive())
    {
        active = true;
        setCurrentCell(0, 0, gui);
        qApp->setOverrideCursor(QCursor(Qt::BlankCursor));
        // qApp->overrideCursor()->setPos(90*qApp->primaryScreen()->geometry().width()/100,
        //                                90*qApp->primaryScreen()->geometry().height()/100);

        gui->setFocus();
    }
}
