// ------------------------------------ //
#include "KeyMapping.h"

#include "Utility/Convert.h"

// Define required keymaps
#define XK_MISCELLANY
#define XK_LATIN1
#define XK_PUBLISHING
#define XK_XKB_KEYS
#define XK_3270
#define XK_KOREAN
#include "keysymdef.h"

#include "include/cef_keyboard_handler.h"

#include <SDL.h>

using namespace Leviathan;
// ------------------------------------ //

DLLEXPORT int KeyMapping::GetCEFButtonFromSdlMouseButton(uint32_t whichbutton)
{
    if(whichbutton == SDL_BUTTON_LEFT) {
        return MBT_LEFT;

    } else if(whichbutton == SDL_BUTTON_RIGHT) {
        return MBT_RIGHT;

    } else if(whichbutton == SDL_BUTTON_MIDDLE) {
        return MBT_MIDDLE;

    } else {
        return -1;
    }
}

DLLEXPORT int32_t KeyMapping::ConvertStringToKeyCode(const std::string& str)
{

    auto key = SDL_GetKeyFromName(str.c_str());

    if(key == SDLK_UNKNOWN) {

        LOG_ERROR("Invalid SDL key name('" + str + "'): " + std::string(SDL_GetError()));
        return key;
    }

    return key;
}

DLLEXPORT std::string KeyMapping::ConvertKeyCodeToString(const int32_t& code)
{

    return std::string(SDL_GetKeyName(code));
}



// This conversion approach is from:
// https://github.com/inniyah/gtk3_sdl2_test/blob/master/MainGtk3App.cpp
// It has been adapted to work with X11 keysyms which have the same values as GDK keys
DLLEXPORT int KeyMapping::SDLKeyToX11Key(const SDL_Keysym& key)
{
    switch(key.sym) {
        // case SDLK_a: return XK_A;
        // case SDLK_b: return XK_B;
        // case SDLK_c: return XK_C;
        // case SDLK_d: return XK_D;
        // case SDLK_e: return XK_E;
        // case SDLK_f: return XK_F;
        // case SDLK_g: return XK_G;
        // case SDLK_h: return XK_H;
        // case SDLK_i: return XK_I;
        // case SDLK_j: return XK_J;
        // case SDLK_k: return XK_K;
        // case SDLK_l: return XK_L;
        // case SDLK_m: return XK_M;
        // case SDLK_n: return XK_N;
        // case SDLK_o: return XK_O;
        // case SDLK_p: return XK_P;
        // case SDLK_q: return XK_Q;
        // case SDLK_r: return XK_R;
        // case SDLK_s: return XK_S;
        // case SDLK_t: return XK_T;
        // case SDLK_u: return XK_U;
        // case SDLK_v: return XK_V;
        // case SDLK_w: return XK_W;
        // case SDLK_x: return XK_X;
        // case SDLK_y: return XK_Y;
        // case SDLK_z: return XK_Z;

    case SDLK_a: return XK_a;
    case SDLK_b: return XK_b;
    case SDLK_c: return XK_c;
    case SDLK_d: return XK_d;
    case SDLK_e: return XK_e;
    case SDLK_f: return XK_f;
    case SDLK_g: return XK_g;
    case SDLK_h: return XK_h;
    case SDLK_i: return XK_i;
    case SDLK_j: return XK_j;
    case SDLK_k: return XK_k;
    case SDLK_l: return XK_l;
    case SDLK_m: return XK_m;
    case SDLK_n: return XK_n;
    case SDLK_o: return XK_o;
    case SDLK_p: return XK_p;
    case SDLK_q: return XK_q;
    case SDLK_r: return XK_r;
    case SDLK_s: return XK_s;
    case SDLK_t: return XK_t;
    case SDLK_u: return XK_u;
    case SDLK_v: return XK_v;
    case SDLK_w: return XK_w;
    case SDLK_x: return XK_x;
    case SDLK_y: return XK_y;
    case SDLK_z: return XK_z;

    case SDLK_1: return XK_1;
    case SDLK_2: return XK_2;
    case SDLK_3: return XK_3;
    case SDLK_4: return XK_4;
    case SDLK_5: return XK_5;
    case SDLK_6: return XK_6;
    case SDLK_7: return XK_7;
    case SDLK_8: return XK_8;
    case SDLK_9: return XK_9;
    case SDLK_0: return XK_0;

    case SDLK_RETURN: return XK_Return;
    case SDLK_ESCAPE: return XK_Escape;
    case SDLK_BACKSPACE: return XK_BackSpace;
    case SDLK_TAB: return XK_Tab;
    case SDLK_SPACE: return XK_space;

    case SDLK_PLUS: return XK_plus;
    case SDLK_MINUS: return XK_minus;
    case SDLK_EQUALS: return XK_equal;
    case SDLK_LEFTBRACKET: return XK_bracketleft;
    case SDLK_RIGHTBRACKET: return XK_bracketright;
    case SDLK_BACKSLASH: return XK_backslash;
    case SDLK_COLON: return XK_colon;
    case SDLK_SEMICOLON: return XK_semicolon;
    case SDLK_QUOTE: return XK_apostrophe;
    case SDLK_BACKQUOTE: return XK_grave;
    case SDLK_COMMA: return XK_comma;
    case SDLK_PERIOD: return XK_period;
    case SDLK_SLASH: return XK_slash;
    case SDLK_CAPSLOCK: return XK_Caps_Lock;

    case SDLK_F1: return XK_F1;
    case SDLK_F2: return XK_F2;
    case SDLK_F3: return XK_F3;
    case SDLK_F4: return XK_F4;
    case SDLK_F5: return XK_F5;
    case SDLK_F6: return XK_F6;
    case SDLK_F7: return XK_F7;
    case SDLK_F8: return XK_F8;
    case SDLK_F9: return XK_F9;
    case SDLK_F10: return XK_F10;
    case SDLK_F11: return XK_F11;
    case SDLK_F12: return XK_F12;

    case SDLK_PRINTSCREEN: return XK_Print;
    case SDLK_SCROLLLOCK: return XK_Scroll_Lock;
    case SDLK_PAUSE: return XK_Pause;
    case SDLK_INSERT: return XK_Insert;
    case SDLK_HOME: return XK_Home;
    case SDLK_PAGEUP: return XK_Page_Up;
    case SDLK_DELETE: return XK_Delete;
    case SDLK_END: return XK_End;
    case SDLK_PAGEDOWN: return XK_Page_Down;
    case SDLK_RIGHT: return XK_Right;
    case SDLK_LEFT: return XK_Left;
    case SDLK_DOWN: return XK_Down;
    case SDLK_UP: return XK_Up;

    case SDLK_NUMLOCKCLEAR: return XK_Num_Lock;
    case SDLK_CLEAR: return XK_Clear;

    case SDLK_KP_DIVIDE: return XK_KP_Divide;
    case SDLK_KP_MULTIPLY: return XK_KP_Multiply;
    case SDLK_KP_MINUS: return XK_KP_Subtract;
    case SDLK_KP_PLUS: return XK_KP_Add;
    case SDLK_KP_ENTER: return XK_KP_Enter;
    case SDLK_KP_PERIOD: return XK_KP_Decimal;

    case SDLK_KP_1: return XK_KP_1;
    case SDLK_KP_2: return XK_KP_2;
    case SDLK_KP_3: return XK_KP_3;
    case SDLK_KP_4: return XK_KP_4;
    case SDLK_KP_5: return XK_KP_5;
    case SDLK_KP_6: return XK_KP_6;
    case SDLK_KP_7: return XK_KP_7;
    case SDLK_KP_8: return XK_KP_8;
    case SDLK_KP_9: return XK_KP_9;
    case SDLK_KP_0: return XK_KP_0;

    case SDLK_LCTRL: return XK_Control_L;
    case SDLK_LSHIFT: return XK_Shift_L;
    case SDLK_LALT: return XK_Alt_L;
    case SDLK_RCTRL: return XK_Control_R;
    case SDLK_RSHIFT: return XK_Shift_R;
    case SDLK_RALT:
        return XK_Alt_R;

        // case SDLK_LGUI: return XK_Meta_L;
    case SDLK_LGUI:
        return XK_Super_L;
        // case SDLK_RGUI: return XK_Meta_R;
    case SDLK_RGUI: return XK_Super_R;
    case SDLK_MENU: return XK_Menu;

    case SDLK_LEFTPAREN: return XK_parenleft;
    case SDLK_RIGHTPAREN: return XK_parenright;
    case SDLK_KP_LEFTBRACE: return XK_braceleft;
    case SDLK_KP_RIGHTBRACE: return XK_braceright;

    case SDLK_LESS: return XK_less;
    case SDLK_GREATER: return XK_greater;
    case SDLK_KP_VERTICALBAR: return XK_bar;
    case SDLK_EXCLAIM: return XK_exclam;
    case SDLK_QUOTEDBL: return XK_quotedbl;
    case SDLK_HASH: return XK_numbersign;
    case SDLK_PERCENT: return XK_percent;
    case SDLK_DOLLAR: return XK_dollar;
    case SDLK_AMPERSAND: return XK_ampersand;
    case SDLK_ASTERISK: return XK_asterisk;
    case SDLK_QUESTION: return XK_question;
    case SDLK_AT: return XK_at;
    case SDLK_UNKNOWN: return XK_periodcentered;
    case SDLK_CARET: return XK_caret;
    case SDLK_UNDERSCORE: return XK_underscore;

    default:
#if LEVIATHAN_PRINT_ERRORS_ABOUT_UNKOWN_KEYS == 1
        LOG_ERROR("Unkown SDL key: " + ConvertKeyCodeToString(key.sym) + +"(" +
                  std::to_string(key.sym) + ")");
#endif
        return XK_VoidSymbol;
    }
}


// The following key translations are from CEF
// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
// Copyright (c) 2008-2014 Marshall A. Greenblatt. Portions Copyright (c)
// 2006-2009 Google Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//    * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//    * Neither the name of Google Inc. nor the name Chromium Embedded
// Framework nor the names of its contributors may be used to endorse
// or promote products derived from this software without specific prior
// written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// From ui/events/keycodes/keyboard_codes_posix.h.
enum KeyboardCode : int {
    VKEY_BACK = 0x08,
    VKEY_TAB = 0x09,
    VKEY_BACKTAB = 0x0A,
    VKEY_CLEAR = 0x0C,
    VKEY_RETURN = 0x0D,
    VKEY_SHIFT = 0x10,
    VKEY_CONTROL = 0x11,
    VKEY_MENU = 0x12,
    VKEY_PAUSE = 0x13,
    VKEY_CAPITAL = 0x14,
    VKEY_KANA = 0x15,
    VKEY_HANGUL = 0x15,
    VKEY_JUNJA = 0x17,
    VKEY_FINAL = 0x18,
    VKEY_HANJA = 0x19,
    VKEY_KANJI = 0x19,
    VKEY_ESCAPE = 0x1B,
    VKEY_CONVERT = 0x1C,
    VKEY_NONCONVERT = 0x1D,
    VKEY_ACCEPT = 0x1E,
    VKEY_MODECHANGE = 0x1F,
    VKEY_SPACE = 0x20,
    VKEY_PRIOR = 0x21,
    VKEY_NEXT = 0x22,
    VKEY_END = 0x23,
    VKEY_HOME = 0x24,
    VKEY_LEFT = 0x25,
    VKEY_UP = 0x26,
    VKEY_RIGHT = 0x27,
    VKEY_DOWN = 0x28,
    VKEY_SELECT = 0x29,
    VKEY_PRINT = 0x2A,
    VKEY_EXECUTE = 0x2B,
    VKEY_SNAPSHOT = 0x2C,
    VKEY_INSERT = 0x2D,
    VKEY_DELETE = 0x2E,
    VKEY_HELP = 0x2F,
    VKEY_0 = 0x30,
    VKEY_1 = 0x31,
    VKEY_2 = 0x32,
    VKEY_3 = 0x33,
    VKEY_4 = 0x34,
    VKEY_5 = 0x35,
    VKEY_6 = 0x36,
    VKEY_7 = 0x37,
    VKEY_8 = 0x38,
    VKEY_9 = 0x39,
    VKEY_A = 0x41,
    VKEY_B = 0x42,
    VKEY_C = 0x43,
    VKEY_D = 0x44,
    VKEY_E = 0x45,
    VKEY_F = 0x46,
    VKEY_G = 0x47,
    VKEY_H = 0x48,
    VKEY_I = 0x49,
    VKEY_J = 0x4A,
    VKEY_K = 0x4B,
    VKEY_L = 0x4C,
    VKEY_M = 0x4D,
    VKEY_N = 0x4E,
    VKEY_O = 0x4F,
    VKEY_P = 0x50,
    VKEY_Q = 0x51,
    VKEY_R = 0x52,
    VKEY_S = 0x53,
    VKEY_T = 0x54,
    VKEY_U = 0x55,
    VKEY_V = 0x56,
    VKEY_W = 0x57,
    VKEY_X = 0x58,
    VKEY_Y = 0x59,
    VKEY_Z = 0x5A,
    VKEY_LWIN = 0x5B,
    VKEY_COMMAND = VKEY_LWIN, // Provide the Mac name for convenience.
    VKEY_RWIN = 0x5C,
    VKEY_APPS = 0x5D,
    VKEY_SLEEP = 0x5F,
    VKEY_NUMPAD0 = 0x60,
    VKEY_NUMPAD1 = 0x61,
    VKEY_NUMPAD2 = 0x62,
    VKEY_NUMPAD3 = 0x63,
    VKEY_NUMPAD4 = 0x64,
    VKEY_NUMPAD5 = 0x65,
    VKEY_NUMPAD6 = 0x66,
    VKEY_NUMPAD7 = 0x67,
    VKEY_NUMPAD8 = 0x68,
    VKEY_NUMPAD9 = 0x69,
    VKEY_MULTIPLY = 0x6A,
    VKEY_ADD = 0x6B,
    VKEY_SEPARATOR = 0x6C,
    VKEY_SUBTRACT = 0x6D,
    VKEY_DECIMAL = 0x6E,
    VKEY_DIVIDE = 0x6F,
    VKEY_F1 = 0x70,
    VKEY_F2 = 0x71,
    VKEY_F3 = 0x72,
    VKEY_F4 = 0x73,
    VKEY_F5 = 0x74,
    VKEY_F6 = 0x75,
    VKEY_F7 = 0x76,
    VKEY_F8 = 0x77,
    VKEY_F9 = 0x78,
    VKEY_F10 = 0x79,
    VKEY_F11 = 0x7A,
    VKEY_F12 = 0x7B,
    VKEY_F13 = 0x7C,
    VKEY_F14 = 0x7D,
    VKEY_F15 = 0x7E,
    VKEY_F16 = 0x7F,
    VKEY_F17 = 0x80,
    VKEY_F18 = 0x81,
    VKEY_F19 = 0x82,
    VKEY_F20 = 0x83,
    VKEY_F21 = 0x84,
    VKEY_F22 = 0x85,
    VKEY_F23 = 0x86,
    VKEY_F24 = 0x87,
    VKEY_NUMLOCK = 0x90,
    VKEY_SCROLL = 0x91,
    VKEY_LSHIFT = 0xA0,
    VKEY_RSHIFT = 0xA1,
    VKEY_LCONTROL = 0xA2,
    VKEY_RCONTROL = 0xA3,
    VKEY_LMENU = 0xA4,
    VKEY_RMENU = 0xA5,
    VKEY_BROWSER_BACK = 0xA6,
    VKEY_BROWSER_FORWARD = 0xA7,
    VKEY_BROWSER_REFRESH = 0xA8,
    VKEY_BROWSER_STOP = 0xA9,
    VKEY_BROWSER_SEARCH = 0xAA,
    VKEY_BROWSER_FAVORITES = 0xAB,
    VKEY_BROWSER_HOME = 0xAC,
    VKEY_VOLUME_MUTE = 0xAD,
    VKEY_VOLUME_DOWN = 0xAE,
    VKEY_VOLUME_UP = 0xAF,
    VKEY_MEDIA_NEXT_TRACK = 0xB0,
    VKEY_MEDIA_PREV_TRACK = 0xB1,
    VKEY_MEDIA_STOP = 0xB2,
    VKEY_MEDIA_PLAY_PAUSE = 0xB3,
    VKEY_MEDIA_LAUNCH_MAIL = 0xB4,
    VKEY_MEDIA_LAUNCH_MEDIA_SELECT = 0xB5,
    VKEY_MEDIA_LAUNCH_APP1 = 0xB6,
    VKEY_MEDIA_LAUNCH_APP2 = 0xB7,
    VKEY_OEM_1 = 0xBA,
    VKEY_OEM_PLUS = 0xBB,
    VKEY_OEM_COMMA = 0xBC,
    VKEY_OEM_MINUS = 0xBD,
    VKEY_OEM_PERIOD = 0xBE,
    VKEY_OEM_2 = 0xBF,
    VKEY_OEM_3 = 0xC0,
    VKEY_OEM_4 = 0xDB,
    VKEY_OEM_5 = 0xDC,
    VKEY_OEM_6 = 0xDD,
    VKEY_OEM_7 = 0xDE,
    VKEY_OEM_8 = 0xDF,
    VKEY_OEM_102 = 0xE2,
    VKEY_OEM_103 = 0xE3, // GTV KEYCODE_MEDIA_REWIND
    VKEY_OEM_104 = 0xE4, // GTV KEYCODE_MEDIA_FAST_FORWARD
    VKEY_PROCESSKEY = 0xE5,
    VKEY_PACKET = 0xE7,
    VKEY_DBE_SBCSCHAR = 0xF3,
    VKEY_DBE_DBCSCHAR = 0xF4,
    VKEY_ATTN = 0xF6,
    VKEY_CRSEL = 0xF7,
    VKEY_EXSEL = 0xF8,
    VKEY_EREOF = 0xF9,
    VKEY_PLAY = 0xFA,
    VKEY_ZOOM = 0xFB,
    VKEY_NONAME = 0xFC,
    VKEY_PA1 = 0xFD,
    VKEY_OEM_CLEAR = 0xFE,
    VKEY_UNKNOWN = 0,

    // POSIX specific VKEYs. Note that as of Windows SDK 7.1, 0x97-9F, 0xD8-DA,
    // and 0xE8 are unassigned.
    VKEY_WLAN = 0x97,
    VKEY_POWER = 0x98,
    VKEY_BRIGHTNESS_DOWN = 0xD8,
    VKEY_BRIGHTNESS_UP = 0xD9,
    VKEY_KBD_BRIGHTNESS_DOWN = 0xDA,
    VKEY_KBD_BRIGHTNESS_UP = 0xE8,

    // Windows does not have a specific key code for AltGr. We use the unused 0xE1
    // (VK_OEM_AX) code to represent AltGr, matching the behaviour of Firefox on
    // Linux.
    VKEY_ALTGR = 0xE1,
    // Windows does not have a specific key code for Compose. We use the unused
    // 0xE6 (VK_ICO_CLEAR) code to represent Compose.
    VKEY_COMPOSE = 0xE6,
};

// From ui/events/keycodes/keyboard_code_conversion_x.cc.
// Gdk key codes (e.g. GDK_BackSpace) and X keysyms (e.g. XK_BackSpace) share
// the same values.
KeyboardCode KeyboardCodeFromXKeysym(unsigned int keysym)
{
    switch(keysym) {
    case XK_BackSpace: return VKEY_BACK;
    case XK_Delete:
    case XK_KP_Delete: return VKEY_DELETE;
    case XK_Tab:
    case XK_KP_Tab:
    case XK_ISO_Left_Tab:
    case XK_3270_BackTab: return VKEY_TAB;
    case XK_Linefeed:
    case XK_Return:
    case XK_KP_Enter:
    case XK_ISO_Enter: return VKEY_RETURN;
    case XK_Clear:
    case XK_KP_Begin: // NumPad 5 without Num Lock, for crosbug.com/29169.
        return VKEY_CLEAR;
    case XK_KP_Space:
    case XK_space: return VKEY_SPACE;
    case XK_Home:
    case XK_KP_Home: return VKEY_HOME;
    case XK_End:
    case XK_KP_End: return VKEY_END;
    case XK_Page_Up:
    case XK_KP_Page_Up: // aka XK_KP_Prior
        return VKEY_PRIOR;
    case XK_Page_Down:
    case XK_KP_Page_Down: // aka XK_KP_Next
        return VKEY_NEXT;
    case XK_Left:
    case XK_KP_Left: return VKEY_LEFT;
    case XK_Right:
    case XK_KP_Right: return VKEY_RIGHT;
    case XK_Down:
    case XK_KP_Down: return VKEY_DOWN;
    case XK_Up:
    case XK_KP_Up: return VKEY_UP;
    case XK_Escape: return VKEY_ESCAPE;
    case XK_Kana_Lock:
    case XK_Kana_Shift: return VKEY_KANA;
    case XK_Hangul: return VKEY_HANGUL;
    case XK_Hangul_Hanja: return VKEY_HANJA;
    case XK_Kanji: return VKEY_KANJI;
    case XK_Henkan: return VKEY_CONVERT;
    case XK_Muhenkan: return VKEY_NONCONVERT;
    case XK_Zenkaku_Hankaku: return VKEY_DBE_DBCSCHAR;
    case XK_A:
    case XK_a: return VKEY_A;
    case XK_B:
    case XK_b: return VKEY_B;
    case XK_C:
    case XK_c: return VKEY_C;
    case XK_D:
    case XK_d: return VKEY_D;
    case XK_E:
    case XK_e: return VKEY_E;
    case XK_F:
    case XK_f: return VKEY_F;
    case XK_G:
    case XK_g: return VKEY_G;
    case XK_H:
    case XK_h: return VKEY_H;
    case XK_I:
    case XK_i: return VKEY_I;
    case XK_J:
    case XK_j: return VKEY_J;
    case XK_K:
    case XK_k: return VKEY_K;
    case XK_L:
    case XK_l: return VKEY_L;
    case XK_M:
    case XK_m: return VKEY_M;
    case XK_N:
    case XK_n: return VKEY_N;
    case XK_O:
    case XK_o: return VKEY_O;
    case XK_P:
    case XK_p: return VKEY_P;
    case XK_Q:
    case XK_q: return VKEY_Q;
    case XK_R:
    case XK_r: return VKEY_R;
    case XK_S:
    case XK_s: return VKEY_S;
    case XK_T:
    case XK_t: return VKEY_T;
    case XK_U:
    case XK_u: return VKEY_U;
    case XK_V:
    case XK_v: return VKEY_V;
    case XK_W:
    case XK_w: return VKEY_W;
    case XK_X:
    case XK_x: return VKEY_X;
    case XK_Y:
    case XK_y: return VKEY_Y;
    case XK_Z:
    case XK_z: return VKEY_Z;

    case XK_0:
    case XK_1:
    case XK_2:
    case XK_3:
    case XK_4:
    case XK_5:
    case XK_6:
    case XK_7:
    case XK_8:
    case XK_9: return static_cast<KeyboardCode>(VKEY_0 + (keysym - XK_0));

    case XK_parenright: return VKEY_0;
    case XK_exclam: return VKEY_1;
    case XK_at: return VKEY_2;
    case XK_numbersign: return VKEY_3;
    case XK_dollar: return VKEY_4;
    case XK_percent: return VKEY_5;
    case XK_asciicircum: return VKEY_6;
    case XK_ampersand: return VKEY_7;
    case XK_asterisk: return VKEY_8;
    case XK_parenleft: return VKEY_9;

    case XK_KP_0:
    case XK_KP_1:
    case XK_KP_2:
    case XK_KP_3:
    case XK_KP_4:
    case XK_KP_5:
    case XK_KP_6:
    case XK_KP_7:
    case XK_KP_8:
    case XK_KP_9: return static_cast<KeyboardCode>(VKEY_NUMPAD0 + (keysym - XK_KP_0));

    case XK_multiply:
    case XK_KP_Multiply: return VKEY_MULTIPLY;
    case XK_KP_Add: return VKEY_ADD;
    case XK_KP_Separator: return VKEY_SEPARATOR;
    case XK_KP_Subtract: return VKEY_SUBTRACT;
    case XK_KP_Decimal: return VKEY_DECIMAL;
    case XK_KP_Divide: return VKEY_DIVIDE;
    case XK_KP_Equal:
    case XK_equal:
    case XK_plus: return VKEY_OEM_PLUS;
    case XK_comma:
    case XK_less: return VKEY_OEM_COMMA;
    case XK_minus:
    case XK_underscore: return VKEY_OEM_MINUS;
    case XK_greater:
    case XK_period: return VKEY_OEM_PERIOD;
    case XK_colon:
    case XK_semicolon: return VKEY_OEM_1;
    case XK_question:
    case XK_slash: return VKEY_OEM_2;
    case XK_asciitilde:
    case XK_quoteleft: return VKEY_OEM_3;
    case XK_bracketleft:
    case XK_braceleft: return VKEY_OEM_4;
    case XK_backslash:
    case XK_bar: return VKEY_OEM_5;
    case XK_bracketright:
    case XK_braceright: return VKEY_OEM_6;
    case XK_quoteright:
    case XK_quotedbl:
        return VKEY_OEM_7;
        // case XK_ISO_Level5_Shift: return VKEY_OEM_8;
    case XK_Shift_L:
    case XK_Shift_R: return VKEY_SHIFT;
    case XK_Control_L:
    case XK_Control_R: return VKEY_CONTROL;
    case XK_Meta_L:
    case XK_Meta_R:
    case XK_Alt_L:
    case XK_Alt_R: return VKEY_MENU;
    case XK_ISO_Level3_Shift: return VKEY_ALTGR;
    case XK_Multi_key: return VKEY_COMPOSE;
    case XK_Pause: return VKEY_PAUSE;
    case XK_Caps_Lock: return VKEY_CAPITAL;
    case XK_Num_Lock: return VKEY_NUMLOCK;
    case XK_Scroll_Lock: return VKEY_SCROLL;
    case XK_Select: return VKEY_SELECT;
    case XK_Print: return VKEY_PRINT;
    case XK_Execute: return VKEY_EXECUTE;
    case XK_Insert:
    case XK_KP_Insert: return VKEY_INSERT;
    case XK_Help: return VKEY_HELP;
    case XK_Super_L: return VKEY_LWIN;
    case XK_Super_R: return VKEY_RWIN;
    case XK_Menu: return VKEY_APPS;
    case XK_F1:
    case XK_F2:
    case XK_F3:
    case XK_F4:
    case XK_F5:
    case XK_F6:
    case XK_F7:
    case XK_F8:
    case XK_F9:
    case XK_F10:
    case XK_F11:
    case XK_F12:
    case XK_F13:
    case XK_F14:
    case XK_F15:
    case XK_F16:
    case XK_F17:
    case XK_F18:
    case XK_F19:
    case XK_F20:
    case XK_F21:
    case XK_F22:
    case XK_F23:
    case XK_F24: return static_cast<KeyboardCode>(VKEY_F1 + (keysym - XK_F1));
    case XK_KP_F1:
    case XK_KP_F2:
    case XK_KP_F3:
    case XK_KP_F4: return static_cast<KeyboardCode>(VKEY_F1 + (keysym - XK_KP_F1));

    case XK_guillemotleft:
    case XK_guillemotright:
    case XK_degree:
        // In the case of canadian multilingual keyboard layout, VKEY_OEM_102 is
        // assigned to ugrave key.
    case XK_ugrave:
    case XK_Ugrave:
    case XK_brokenbar:
        return VKEY_OEM_102; // international backslash key in 102 keyboard.

        //     // When evdev is in use, /usr/share/X11/xkb/symbols/inet maps F13-18 keys
        //     // to the special XF86XK symbols to support Microsoft Ergonomic keyboards:
        //     // https://bugs.freedesktop.org/show_bug.cgi?id=5783
        //     // In Chrome, we map these X key symbols back to F13-18 since we don't have
        //     // VKEYs for these XF86XK symbols.
        // case XF86XK_Tools: return VKEY_F13;
        // case XF86XK_Launch5: return VKEY_F14;
        // case XF86XK_Launch6: return VKEY_F15;
        // case XF86XK_Launch7: return VKEY_F16;
        // case XF86XK_Launch8: return VKEY_F17;
        // case XF86XK_Launch9: return VKEY_F18;
        // case XF86XK_Refresh:
        // case XF86XK_History:
        // case XF86XK_OpenURL:
        // case XF86XK_AddFavorite:
        // case XF86XK_Go:
        // case XF86XK_ZoomIn:
        // case XF86XK_ZoomOut:
        //     // ui::AcceleratorGtk tries to convert the XF86XK_ keysyms on Chrome
        //     // startup. It's safe to return VKEY_UNKNOWN here since ui::AcceleratorGtk
        //     // also checks a Gdk keysym. http://crbug.com/109843
        //     return VKEY_UNKNOWN;
        //     // For supporting multimedia buttons on a USB keyboard.
        // case XF86XK_Back: return VKEY_BROWSER_BACK;
        // case XF86XK_Forward: return VKEY_BROWSER_FORWARD;
        // case XF86XK_Reload: return VKEY_BROWSER_REFRESH;
        // case XF86XK_Stop: return VKEY_BROWSER_STOP;
        // case XF86XK_Search: return VKEY_BROWSER_SEARCH;
        // case XF86XK_Favorites: return VKEY_BROWSER_FAVORITES;
        // case XF86XK_HomePage: return VKEY_BROWSER_HOME;
        // case XF86XK_AudioMute: return VKEY_VOLUME_MUTE;
        // case XF86XK_AudioLowerVolume: return VKEY_VOLUME_DOWN;
        // case XF86XK_AudioRaiseVolume: return VKEY_VOLUME_UP;
        // case XF86XK_AudioNext: return VKEY_MEDIA_NEXT_TRACK;
        // case XF86XK_AudioPrev: return VKEY_MEDIA_PREV_TRACK;
        // case XF86XK_AudioStop: return VKEY_MEDIA_STOP;
        // case XF86XK_AudioPlay: return VKEY_MEDIA_PLAY_PAUSE;
        // case XF86XK_Mail: return VKEY_MEDIA_LAUNCH_MAIL;
        // case XF86XK_LaunchA: // F3 on an Apple keyboard.
        //     return VKEY_MEDIA_LAUNCH_APP1;
        // case XF86XK_LaunchB: // F4 on an Apple keyboard.
        // case XF86XK_Calculator: return VKEY_MEDIA_LAUNCH_APP2;
        // case XF86XK_WLAN: return VKEY_WLAN;
        // case XF86XK_PowerOff: return VKEY_POWER;
        // case XF86XK_MonBrightnessDown: return VKEY_BRIGHTNESS_DOWN;
        // case XF86XK_MonBrightnessUp: return VKEY_BRIGHTNESS_UP;
        // case XF86XK_KbdBrightnessDown: return VKEY_KBD_BRIGHTNESS_DOWN;
        // case XF86XK_KbdBrightnessUp:
        //     return VKEY_KBD_BRIGHTNESS_UP;

        // TODO(sad): some keycodes are still missing.
        // Unknown translated to unknown
    case XK_VoidSymbol: return VKEY_UNKNOWN;
    }

#if LEVIATHAN_PRINT_ERRORS_ABOUT_UNKOWN_KEYS == 1
    LOG_ERROR("Unknown X11 key to VKEY: " + Convert::ToHexadecimalString(keysym));
#endif
    return VKEY_UNKNOWN;
}

// From content/browser/renderer_host/input/web_input_event_util_posix.cc.
DLLEXPORT KeyboardCode KeyMapping::GdkEventToWindowsKeyCode(int key)
{
    static const unsigned int kHardwareCodeToXKKeyval[] = {
        0, // 0x00:
        0, // 0x01:
        0, // 0x02:
        0, // 0x03:
        0, // 0x04:
        0, // 0x05:
        0, // 0x06:
        0, // 0x07:
        0, // 0x08:
        0, // 0x09: GDK_Escape
        XK_1, // 0x0A: XK_1
        XK_2, // 0x0B: XK_2
        XK_3, // 0x0C: XK_3
        XK_4, // 0x0D: XK_4
        XK_5, // 0x0E: XK_5
        XK_6, // 0x0F: XK_6
        XK_7, // 0x10: XK_7
        XK_8, // 0x11: XK_8
        XK_9, // 0x12: XK_9
        XK_0, // 0x13: XK_0
        XK_minus, // 0x14: XK_minus
        XK_equal, // 0x15: XK_equal
        0, // 0x16: XK_BackSpace
        0, // 0x17: XK_Tab
        XK_q, // 0x18: XK_q
        XK_w, // 0x19: XK_w
        XK_e, // 0x1A: XK_e
        XK_r, // 0x1B: XK_r
        XK_t, // 0x1C: XK_t
        XK_y, // 0x1D: XK_y
        XK_u, // 0x1E: XK_u
        XK_i, // 0x1F: XK_i
        XK_o, // 0x20: XK_o
        XK_p, // 0x21: XK_p
        XK_bracketleft, // 0x22: XK_bracketleft
        XK_bracketright, // 0x23: XK_bracketright
        0, // 0x24: XK_Return
        0, // 0x25: XK_Control_L
        XK_a, // 0x26: XK_a
        XK_s, // 0x27: XK_s
        XK_d, // 0x28: XK_d
        XK_f, // 0x29: XK_f
        XK_g, // 0x2A: XK_g
        XK_h, // 0x2B: XK_h
        XK_j, // 0x2C: XK_j
        XK_k, // 0x2D: XK_k
        XK_l, // 0x2E: XK_l
        XK_semicolon, // 0x2F: XK_semicolon
        XK_apostrophe, // 0x30: XK_apostrophe
        XK_grave, // 0x31: XK_grave
        0, // 0x32: XK_Shift_L
        XK_backslash, // 0x33: XK_backslash
        XK_z, // 0x34: XK_z
        XK_x, // 0x35: XK_x
        XK_c, // 0x36: XK_c
        XK_v, // 0x37: XK_v
        XK_b, // 0x38: XK_b
        XK_n, // 0x39: XK_n
        XK_m, // 0x3A: XK_m
        XK_comma, // 0x3B: XK_comma
        XK_period, // 0x3C: XK_period
        XK_slash, // 0x3D: XK_slash
        0, // 0x3E: XK_Shift_R
        0, // 0x3F:
        0, // 0x40:
        0, // 0x41:
        0, // 0x42:
        0, // 0x43:
        0, // 0x44:
        0, // 0x45:
        0, // 0x46:
        0, // 0x47:
        0, // 0x48:
        0, // 0x49:
        0, // 0x4A:
        0, // 0x4B:
        0, // 0x4C:
        0, // 0x4D:
        0, // 0x4E:
        0, // 0x4F:
        0, // 0x50:
        0, // 0x51:
        0, // 0x52:
        0, // 0x53:
        0, // 0x54:
        0, // 0x55:
        0, // 0x56:
        0, // 0x57:
        0, // 0x58:
        0, // 0x59:
        0, // 0x5A:
        0, // 0x5B:
        0, // 0x5C:
        0, // 0x5D:
        0, // 0x5E:
        0, // 0x5F:
        0, // 0x60:
        0, // 0x61:
        0, // 0x62:
        0, // 0x63:
        0, // 0x64:
        0, // 0x65:
        0, // 0x66:
        0, // 0x67:
        0, // 0x68:
        0, // 0x69:
        0, // 0x6A:
        0, // 0x6B:
        0, // 0x6C:
        0, // 0x6D:
        0, // 0x6E:
        0, // 0x6F:
        0, // 0x70:
        0, // 0x71:
        0, // 0x72:
        XK_Super_L, // 0x73: XK_Super_L
        XK_Super_R, // 0x74: XK_Super_R
    };

    // |windows_key_code| has to include a valid virtual-key code even when we
    // use non-US layouts, e.g. even when we type an 'A' key of a US keyboard
    // on the Hebrew layout, |windows_key_code| should be VK_A.
    // On the other hand, |event->keyval| value depends on the current
    // GdkKeymap object, i.e. when we type an 'A' key of a US keyboard on
    // the Hebrew layout, |event->keyval| becomes XK_hebrew_shin and this
    // KeyboardCodeFromXKeysym() call returns 0.
    // To improve compatibilty with Windows, we use |event->hardware_keycode|
    // for retrieving its Windows key-code for the keys when the
    // WebCore::windows_key_codeForEvent() call returns 0.
    // We shouldn't use |event->hardware_keycode| for keys that GdkKeymap
    // objects cannot change because |event->hardware_keycode| doesn't change
    // even when we change the layout options, e.g. when we swap a control
    // key and a caps-lock key, GTK doesn't swap their
    // |event->hardware_keycode| values but swap their |event->keyval| values.
    KeyboardCode windows_key_code = KeyboardCodeFromXKeysym(key);
    if(windows_key_code)
        return windows_key_code;

    if(static_cast<size_t>(key) < std::size(kHardwareCodeToXKKeyval)) {
        int keyval = kHardwareCodeToXKKeyval[key];
        if(keyval)
            return KeyboardCodeFromXKeysym(keyval);
    }

    // This key is one that keyboard-layout drivers cannot change.
    // Use |event->keyval| to retrieve its |windows_key_code| value.
    return KeyboardCodeFromXKeysym(key);
}

// From content/browser/renderer_host/input/web_input_event_util_posix.cc.
DLLEXPORT KeyboardCode KeyMapping::GetWindowsKeyCodeWithoutLocation(KeyboardCode key_code)
{
    switch(key_code) {
    case VKEY_LCONTROL:
    case VKEY_RCONTROL: return VKEY_CONTROL;
    case VKEY_LSHIFT:
    case VKEY_RSHIFT: return VKEY_SHIFT;
    case VKEY_LMENU:
    case VKEY_RMENU: return VKEY_MENU;
    default: return key_code;
    }
}

// From content/browser/renderer_host/input/web_input_event_builders_gtk.cc.
// Gets the corresponding control character of a specified key code. See:
// http://en.wikipedia.org/wiki/Control_characters
// We emulate Windows behavior here.
DLLEXPORT int KeyMapping::GetControlCharacter(KeyboardCode windows_key_code, bool shift)
{
    if(windows_key_code >= VKEY_A && windows_key_code <= VKEY_Z) {
        // ctrl-A ~ ctrl-Z map to \x01 ~ \x1A
        return windows_key_code - VKEY_A + 1;
    }
    if(shift) {
        // following graphics chars require shift key to input.
        switch(windows_key_code) {
            // ctrl-@ maps to \x00 (Null byte)
        case VKEY_2:
            return 0;
            // ctrl-^ maps to \x1E (Record separator, Information separator two)
        case VKEY_6:
            return 0x1E;
            // ctrl-_ maps to \x1F (Unit separator, Information separator one)
        case VKEY_OEM_MINUS:
            return 0x1F;
            // Returns 0 for all other keys to avoid inputting unexpected chars.
        default: return 0;
        }
    } else {
        switch(windows_key_code) {
            // ctrl-[ maps to \x1B (Escape)
        case VKEY_OEM_4:
            return 0x1B;
            // ctrl-\ maps to \x1C (File separator, Information separator four)
        case VKEY_OEM_5:
            return 0x1C;
            // ctrl-] maps to \x1D (Group separator, Information separator three)
        case VKEY_OEM_6:
            return 0x1D;
            // ctrl-Enter maps to \x0A (Line feed)
        case VKEY_RETURN:
            return 0x0A;
            // Returns 0 for all other keys to avoid inputting unexpected chars.
        default: return 0;
        }
    }
}
