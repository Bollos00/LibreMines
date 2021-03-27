/*****************************************************************************
 * LibreMines                                                                *
 * Copyright (C) 2020-2021  Bruno Bollos Correa                              *
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


#include "qkeyinput.h"

#include <QKeyEvent>
#include <QDebug>

QKeyInput::QKeyInput(QWidget *parent):
    QLineEdit(parent),
    key(-1)
{
    setReadOnly(true);
}

int QKeyInput::currentKey() const
{
    return key;
}

bool QKeyInput::setKey(const int k)
{
    const QString text = QKeyInput::getTextFromKey(k);

    if(text.isEmpty())
    {
        setText("");
        key = -1;
        return false;
    }

    setText(text);
    key = k;
    return true;

}

bool QKeyInput::isValid() const
{
    return key != -1 && !text().isEmpty();
}

QString QKeyInput::getTextFromKey(const int k)
{
    switch (k)
    {
        case Qt::Key_A: return "A";
        case Qt::Key_B: return "B";
        case Qt::Key_C: return "C";
        case Qt::Key_D: return "D";
        case Qt::Key_E: return "E";
        case Qt::Key_F: return "F";
        case Qt::Key_G: return "G";
        case Qt::Key_H: return "H";
        case Qt::Key_I: return "I";
        case Qt::Key_J: return "J";
        case Qt::Key_K: return "K";
        case Qt::Key_L: return "L";
        case Qt::Key_M: return "M";
        case Qt::Key_N: return "N";
        case Qt::Key_O: return "O";
        case Qt::Key_P: return "P";
        case Qt::Key_Q: return "Q";
        case Qt::Key_R: return "R";
        case Qt::Key_S: return "S";
        case Qt::Key_T: return "T";
        case Qt::Key_U: return "U";
        case Qt::Key_V: return "V";
        case Qt::Key_W: return "W";
        case Qt::Key_X: return "X";
        case Qt::Key_Y: return "Y";
        case Qt::Key_Z: return "Z";
        case Qt::Key_0: return "0";
        case Qt::Key_1: return "1";
        case Qt::Key_2: return "2";
        case Qt::Key_3: return "3";
        case Qt::Key_4: return "4";
        case Qt::Key_5: return "5";
        case Qt::Key_6: return "6";
        case Qt::Key_7: return "7";
        case Qt::Key_8: return "8";
        case Qt::Key_9: return "9";
        case Qt::Key_F1:  return "F1";
        case Qt::Key_F2:  return "F2";
        case Qt::Key_F3:  return "F3";
        case Qt::Key_F4:  return "F4";
        case Qt::Key_F5:  return "F5";
        case Qt::Key_F6:  return "F6";
        case Qt::Key_F7:  return "F7";
        case Qt::Key_F8:  return "F8";
        case Qt::Key_F9:  return "F9";
        case Qt::Key_F10: return "F10";
        case Qt::Key_F11: return "F11";
        case Qt::Key_F12: return "F12";
        case Qt::Key_F13: return "F13";
        case Qt::Key_F14: return "F14";
        case Qt::Key_F15: return "F15";
        case Qt::Key_F16: return "F16";
        case Qt::Key_F17: return "F17";
        case Qt::Key_F18: return "F18";
        case Qt::Key_F19: return "F19";
        case Qt::Key_F20: return "F20";
        case Qt::Key_Down:          return "Down";
        case Qt::Key_Up:            return "Up";
        case Qt::Key_Left:          return "Left";
        case Qt::Key_Right:         return "Right";
//        case Qt::Key_Enter:         return "Enter";
        case Qt::Key_Space:         return "Space";
        case Qt::Key_Backspace:     return "Backspace";
        case Qt::Key_Delete:        return "Delete";
        case Qt::Key_PageUp:        return "PageUp";
        case Qt::Key_PageDown:      return "PageDown";
        case Qt::Key_End:           return "End";
        case Qt::Key_Home:          return "Home";
        case Qt::Key_AsciiTilde:    return "~";
        case Qt::Key_QuoteLeft:     return "`";
        case Qt::Key_Exclam:        return "!";
        case Qt::Key_At:            return "@";
        case Qt::Key_NumberSign:    return "#";
        case Qt::Key_Dollar:        return "$";
        case Qt::Key_Percent:       return "%";
        case Qt::Key_AsciiCircum:   return "^";
        case Qt::Key_Ampersand:     return "&";
        case Qt::Key_Asterisk:      return "*";
        case Qt::Key_ParenLeft:     return "(";
        case Qt::Key_ParenRight:    return ")";
        case Qt::Key_Minus:         return "-";
        case Qt::Key_Underscore:    return "_";
        case Qt::Key_BraceLeft:     return "{";
        case Qt::Key_BraceRight:    return "}";
        case Qt::Key_BracketLeft:   return "[";
        case Qt::Key_BracketRight:  return "]";
        case Qt::Key_Equal:         return "=";
        case Qt::Key_Plus:          return "+";
        case Qt::Key_Semicolon:     return ";";
        case Qt::Key_Colon:         return ":";
        case Qt::Key_QuoteDbl:      return "\"";
        case Qt::Key_Apostrophe:    return "\'";
        case Qt::Key_Comma:         return ",";
        case Qt::Key_Less:          return "<";
        case Qt::Key_Greater:       return ">";
        case Qt::Key_Period:        return ".";
        case Qt::Key_Slash:         return "/";
        case Qt::Key_Backslash:     return "\\";
        case Qt::Key_Question:      return "?";
        case Qt::Key_Bar:           return "|";
    }

    return QString();
}

void QKeyInput::keyPressEvent(QKeyEvent *e)
{
    // clear (reset) the line when the escape key is pressed
    if(e->key() == Qt::Key_Escape)
    {
        setText("");
        key = -1;
        return;
    }

    const QString currentKeyText = QKeyInput::getTextFromKey(e->key());

    if(!currentKeyText.isEmpty())
    {
        setText(currentKeyText);
        key = e->key();
    }
}
