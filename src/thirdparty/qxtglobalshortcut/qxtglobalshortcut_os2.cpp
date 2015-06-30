/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtGui module of the Qxt library.
 **
 ** This library is free software; you can redistribute it and/or modify it
 ** under the terms of the Common Public License, version 1.0, as published
 ** by IBM, and/or under the terms of the GNU Lesser General Public License,
 ** version 2.1, as published by the Free Software Foundation.
 **
 ** This file is provided "AS IS", without WARRANTIES OR CONDITIONS OF ANY
 ** KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 ** WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR
 ** FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** You should have received a copy of the CPL and the LGPL along with this
 ** file. See the LICENSE file and the cpl1.0.txt/lgpl-2.1.txt files
 ** included with the source distribution for more information.
 ** If you did not receive a copy of the licenses, contact the Qxt Foundation.
 **
 ** <http://libqxt.org>  <foundation@libqxt.org>
 **
 ****************************************************************************/
#include "qxtglobalshortcut_p.h"
#include <pmhotkey.h>
#include <scancode.h>

bool QxtGlobalShortcutPrivate::eventFilter(void* message)
{
    PQMSG pqmsg = static_cast<PQMSG>(message);
    if (pqmsg->msg == WM_HOTKEY)
    {
        const quint32 modifiers = SHORT1FROMMP(pqmsg->mp2);
        const quint32 keycode = SHORT2FROMMP(pqmsg->mp2);
        activateShortcut(keycode, modifiers);
    }
    return false;
}

quint32 QxtGlobalShortcutPrivate::nativeModifiers(Qt::KeyboardModifiers modifiers)
{
    // MOD_ALT, MOD_CONTROL, (MOD_KEYUP), MOD_SHIFT, MOD_WIN
    quint32 native = 0;
    if (modifiers & Qt::ShiftModifier)
        native |= KC_SHIFT;
    if (modifiers & Qt::ControlModifier)
        native |= KC_CTRL;
    if (modifiers & Qt::AltModifier)
        native |= KC_ALT;
//    if (modifiers & Qt::MetaModifier)
//        native |= MOD_WIN;
    // TODO: resolve these?
    //if (modifiers & Qt::KeypadModifier)
    //if (modifiers & Qt::GroupSwitchModifier)
    return native;
}

quint32 QxtGlobalShortcutPrivate::nativeKeycode(Qt::Key key)
{
    switch (key)
    {
    case Qt::Key_Escape:
        return SC_ESC;
    case Qt::Key_Tab:
        return SC_TAB;
    case Qt::Key_Backtab:
        return SC_TAB;
    case Qt::Key_Backspace:
        return SC_BACKSPACE;
    case Qt::Key_Return:
        return SC_RETURN;
    case Qt::Key_Enter:
        return SC_ENTER;
    case Qt::Key_Insert:
        return SC_INSERT;
    case Qt::Key_Delete:
        return SC_DELETE;
    case Qt::Key_Print:
        return SC_PRINT;
    case Qt::Key_ScrollLock:
        return SC_SCROLL;
    case Qt::Key_Pause:
        return SC_PAUSE;
    case Qt::Key_Home:
        return SC_HOME;
    case Qt::Key_End:
        return SC_END;
    case Qt::Key_Left:
        return SC_LEFT;
    case Qt::Key_Up:
        return SC_UP;
    case Qt::Key_Right:
        return SC_RIGHT;
    case Qt::Key_Down:
        return SC_DOWN;
    case Qt::Key_PageUp:
        return SC_PGUP;
    case Qt::Key_PageDown:
        return SC_PGDOWN;
    case Qt::Key_F1:
        return SC_F1;
    case Qt::Key_F2:
        return SC_F2;
    case Qt::Key_F3:
        return SC_F3;
    case Qt::Key_F4:
        return SC_F4;
    case Qt::Key_F5:
        return SC_F5;
    case Qt::Key_F6:
        return SC_F6;
    case Qt::Key_F7:
        return SC_F7;
    case Qt::Key_F8:
        return SC_F8;
    case Qt::Key_F9:
        return SC_F9;
    case Qt::Key_F10:
        return SC_F10;
    case Qt::Key_F11:
        return SC_F11;
    case Qt::Key_F12:
        return SC_F12;
    case Qt::Key_Space:
        return SC_SPACE;

        // Symbols
    case Qt::Key_Asterisk:
        return SC_MULTIPLY;
    case Qt::Key_Plus:
        return SC_PLUS;
    case Qt::Key_Comma:
        return SC_COMMA;
    case Qt::Key_Minus:
        return SC_MINUS;
    case Qt::Key_Slash:
        return SC_RSLASH;

        // numbers
    case Qt::Key_0:
        return SC_0;
    case Qt::Key_1:
	return SC_1;
    case Qt::Key_2:
	return SC_2;
    case Qt::Key_3:
	return SC_3;
    case Qt::Key_4:
	return SC_4;
    case Qt::Key_5:
	return SC_5;
    case Qt::Key_6:
	return SC_6;
    case Qt::Key_7:
	return SC_7;
    case Qt::Key_8:
	return SC_8;
    case Qt::Key_9:
	return SC_9;

        // letters
    case Qt::Key_A:
	return SC_A;
    case Qt::Key_B:
	return SC_B;
    case Qt::Key_C:
	return SC_C;
    case Qt::Key_D:
	return SC_D;
    case Qt::Key_E:
	return SC_E;
    case Qt::Key_F:
	return SC_F;
    case Qt::Key_G:
	return SC_G;
    case Qt::Key_H:
	return SC_H;
    case Qt::Key_I:
	return SC_I;
    case Qt::Key_J:
	return SC_J;
    case Qt::Key_K:
	return SC_K;
    case Qt::Key_L:
	return SC_L;
    case Qt::Key_M:
	return SC_M;
    case Qt::Key_N:
	return SC_N;
    case Qt::Key_O:
	return SC_O;
    case Qt::Key_P:
	return SC_P;
    case Qt::Key_Q:
	return SC_Q;
    case Qt::Key_R:
	return SC_R;
    case Qt::Key_S:
	return SC_S;
    case Qt::Key_T:
	return SC_T;
    case Qt::Key_U:
	return SC_U;
    case Qt::Key_V:
	return SC_V;
    case Qt::Key_W:
	return SC_W;
    case Qt::Key_X:
	return SC_X;
    case Qt::Key_Y:
	return SC_Y;
    case Qt::Key_Z:
	return SC_Z;

    default:
        return 0;
    }
}

bool QxtGlobalShortcutPrivate::registerShortcut(quint32 nativeKey, quint32 nativeMods)
{
    return HKRegisterHotKey(0, nativeMods ^ nativeKey, nativeMods, nativeKey);
}

bool QxtGlobalShortcutPrivate::unregisterShortcut(quint32 nativeKey, quint32 nativeMods)
{
   return HKUnregisterHotKey(0, nativeMods ^ nativeKey);
}
