// ------------------------------------ //
#include "KeyMapping.h"

#include <SDL.h>

using namespace Leviathan;
// ------------------------------------ //

// This enum is from CEF
// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
// From ui/events/keycodes/keyboard_codes_posix.h.
enum KeyboardCode {
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

    // This is an addition not in CEF
    VKEY_CANCEL = 0x03,
};

DLLEXPORT int KeyMapping::GetWindowsKeyCodeFromSDLEvent(const SDL_Event& event)
{
    switch(event.key.keysym.sym) {
    case SDLK_RETURN: return VKEY_RETURN;
    case SDLK_ESCAPE: return VKEY_ESCAPE;
    case SDLK_BACKSPACE: return VKEY_BACK;
    case SDLK_TAB: return VKEY_TAB;
    case SDLK_SPACE:
        return VKEY_SPACE;
    // case SDLK_EXCLAIM: return VKEY_EXCLAIM;
    // case SDLK_QUOTEDBL: return VKEY_QUOTEDBL;
    // case SDLK_HASH: return VKEY_HASH;
    // case SDLK_PERCENT: return VKEY_PERCENT;
    // case SDLK_DOLLAR: return VKEY_DOLLAR;
    // case SDLK_AMPERSAND: return VKEY_AMPERSAND;
    // case SDLK_QUOTE: return VKEY_QUOTE;
    // case SDLK_LEFTPAREN: return VKEY_LEFTPAREN;
    // case SDLK_RIGHTPAREN: return VKEY_RIGHTPAREN;
    // case SDLK_ASTERISK: return VKEY_ASTERISK;
    // case SDLK_PLUS: return VKEY_PLUS;
    // case SDLK_COMMA: return VKEY_COMMA;
    // case SDLK_MINUS: return VKEY_MINUS;
    // case SDLK_PERIOD: return VKEY_PERIOD;
    // case SDLK_SLASH: return VKEY_SLASH;
    case SDLK_0: return VKEY_0;
    case SDLK_1: return VKEY_1;
    case SDLK_2: return VKEY_2;
    case SDLK_3: return VKEY_3;
    case SDLK_4: return VKEY_4;
    case SDLK_5: return VKEY_5;
    case SDLK_6: return VKEY_6;
    case SDLK_7: return VKEY_7;
    case SDLK_8: return VKEY_8;
    case SDLK_9:
        return VKEY_9;
    // case SDLK_COLON: return VKEY_COLON;
    // case SDLK_SEMICOLON: return VKEY_SEMICOLON;
    // case SDLK_LESS: return VKEY_LESS;
    // case SDLK_EQUALS: return VKEY_EQUALS;
    // case SDLK_GREATER: return VKEY_GREATER;
    // case SDLK_QUESTION: return VKEY_QUESTION;
    // case SDLK_AT:
    //     return VKEY_AT;
        /*
        Skip uppercase letters
        */
    case SDLK_LEFTBRACKET: return VKEY_OEM_4;
    case SDLK_BACKSLASH: return VKEY_OEM_5;
    case SDLK_RIGHTBRACKET:
        return VKEY_OEM_6;
    // case SDLK_CARET: return VKEY_CARET;
    // case SDLK_UNDERSCORE: return VKEY_UNDERSCORE;
    // case SDLK_BACKQUOTE: return VKEY_BACKQUOTE;
    case SDLK_a: return VKEY_A;
    case SDLK_b: return VKEY_B;
    case SDLK_c: return VKEY_C;
    case SDLK_d: return VKEY_D;
    case SDLK_e: return VKEY_E;
    case SDLK_f: return VKEY_F;
    case SDLK_g: return VKEY_G;
    case SDLK_h: return VKEY_H;
    case SDLK_i: return VKEY_I;
    case SDLK_j: return VKEY_J;
    case SDLK_k: return VKEY_K;
    case SDLK_l: return VKEY_L;
    case SDLK_m: return VKEY_M;
    case SDLK_n: return VKEY_N;
    case SDLK_o: return VKEY_O;
    case SDLK_p: return VKEY_P;
    case SDLK_q: return VKEY_Q;
    case SDLK_r: return VKEY_R;
    case SDLK_s: return VKEY_S;
    case SDLK_t: return VKEY_T;
    case SDLK_u: return VKEY_U;
    case SDLK_v: return VKEY_V;
    case SDLK_w: return VKEY_W;
    case SDLK_x: return VKEY_X;
    case SDLK_y: return VKEY_Y;
    case SDLK_z: return VKEY_Z;

    case SDLK_CAPSLOCK: return VKEY_CAPITAL;

    case SDLK_F1: return VKEY_F1;
    case SDLK_F2: return VKEY_F2;
    case SDLK_F3: return VKEY_F3;
    case SDLK_F4: return VKEY_F4;
    case SDLK_F5: return VKEY_F5;
    case SDLK_F6: return VKEY_F6;
    case SDLK_F7: return VKEY_F7;
    case SDLK_F8: return VKEY_F8;
    case SDLK_F9: return VKEY_F9;
    case SDLK_F10: return VKEY_F10;
    case SDLK_F11: return VKEY_F11;
    case SDLK_F12: return VKEY_F12;

    case SDLK_PRINTSCREEN: return VKEY_SNAPSHOT;
    case SDLK_SCROLLLOCK: return VKEY_SCROLL;
    case SDLK_PAUSE: return VKEY_PAUSE;
    case SDLK_INSERT: return VKEY_INSERT;
    case SDLK_HOME: return VKEY_HOME;
    case SDLK_PAGEUP: return VKEY_PRIOR;
    case SDLK_DELETE: return VKEY_DELETE;
    case SDLK_END: return VKEY_END;
    case SDLK_PAGEDOWN: return VKEY_NEXT;
    case SDLK_RIGHT: return VKEY_RIGHT;
    case SDLK_LEFT: return VKEY_LEFT;
    case SDLK_DOWN: return VKEY_DOWN;
    case SDLK_UP:
        return VKEY_UP;

        // case SDLK_NUMLOCKCLEAR: return VKEY_NUMLOCKCLEAR;
    case SDLK_KP_DIVIDE: return VKEY_DIVIDE;
    case SDLK_KP_MULTIPLY: return VKEY_MULTIPLY;
    case SDLK_KP_MINUS: return VKEY_OEM_MINUS;
    case SDLK_KP_PLUS: return VKEY_OEM_PLUS;
    case SDLK_KP_ENTER: return VKEY_RETURN;
    case SDLK_KP_1: return VKEY_1;
    case SDLK_KP_2: return VKEY_2;
    case SDLK_KP_3: return VKEY_3;
    case SDLK_KP_4: return VKEY_4;
    case SDLK_KP_5: return VKEY_5;
    case SDLK_KP_6: return VKEY_6;
    case SDLK_KP_7: return VKEY_7;
    case SDLK_KP_8: return VKEY_8;
    case SDLK_KP_9: return VKEY_9;
    case SDLK_KP_0: return VKEY_0;
    case SDLK_KP_PERIOD: return VKEY_OEM_PERIOD;

    case SDLK_APPLICATION: return VKEY_APPS;
    case SDLK_POWER:
        return VKEY_POWER;
        // case SDLK_KP_EQUALS: return VKEY_KP_EQUALS;
    case SDLK_F13: return VKEY_F13;
    case SDLK_F14: return VKEY_F14;
    case SDLK_F15: return VKEY_F15;
    case SDLK_F16: return VKEY_F16;
    case SDLK_F17: return VKEY_F17;
    case SDLK_F18: return VKEY_F18;
    case SDLK_F19: return VKEY_F19;
    case SDLK_F20: return VKEY_F20;
    case SDLK_F21: return VKEY_F21;
    case SDLK_F22: return VKEY_F22;
    case SDLK_F23: return VKEY_F23;
    case SDLK_F24: return VKEY_F24;
    case SDLK_EXECUTE: return VKEY_EXECUTE;
    case SDLK_HELP: return VKEY_HELP;
    case SDLK_MENU: return VKEY_MENU;
    case SDLK_SELECT: return VKEY_SELECT;
    case SDLK_STOP:
        return VKEY_MEDIA_STOP;
        // case SDLK_AGAIN: return VKEY_AGAIN;
        // case SDLK_UNDO: return VKEY_UNDO;
    // case SDLK_CUT: return VKEY_CUT;
    // case SDLK_COPY: return VKEY_COPY;
    // case SDLK_PASTE: return VKEY_PASTE;
    // case SDLK_FIND: return VKEY_FIND;
    case SDLK_MUTE: return VKEY_VOLUME_MUTE;
    case SDLK_VOLUMEUP: return VKEY_VOLUME_UP;
    case SDLK_VOLUMEDOWN: return VKEY_VOLUME_DOWN;
    case SDLK_KP_COMMA:
        return VKEY_OEM_COMMA;
        // case SDLK_KP_EQUALSAS400: return VKEY_KP_EQUALSAS400;


        // case SDLK_ALTERASE: return VKEY_ALTERASE;
        // case SDLK_SYSREQ: return VKEY_SYSREQ;
    case SDLK_CANCEL: return VKEY_CANCEL;
    case SDLK_CLEAR: return VKEY_CLEAR;
    case SDLK_PRIOR:
        return VKEY_PRIOR;
        // case SDLK_RETURN2: return VKEY_RETURN2;
    case SDLK_SEPARATOR:
        return VKEY_SEPARATOR;
        // case SDLK_OUT: return VKEY_OUT;
        // case SDLK_OPER: return VKEY_OPER;
        // case SDLK_CLEARAGAIN: return VKEY_CLEARAGAIN;
    case SDLK_CRSEL: return VKEY_CRSEL;
    case SDLK_EXSEL:
        return VKEY_EXSEL;

        // case SDLK_KP_00: return VKEY_KP_00;
        // case SDLK_KP_000: return VKEY_KP_000;
    case SDLK_THOUSANDSSEPARATOR: return VKEY_SEPARATOR;

    case SDLK_DECIMALSEPARATOR:
        return VKEY_SEPARATOR;

        // case SDLK_CURRENCYUNIT: return VKEY_CURRENCYUNIT;
        // case SDLK_CURRENCYSUBUNIT: return VKEY_CURRENCYSUBUNIT;

    case SDLK_KP_LEFTPAREN: return VKEY_9;
    case SDLK_KP_RIGHTPAREN: return VKEY_0;
    case SDLK_KP_LEFTBRACE: return VKEY_4;
    case SDLK_KP_RIGHTBRACE: return VKEY_KP_RIGHTBRACE;
    case SDLK_KP_TAB: return VKEY_KP_TAB;
    case SDLK_KP_BACKSPACE: return VKEY_KP_BACKSPACE;
    case SDLK_KP_A: return VKEY_KP_A;
    case SDLK_KP_B: return VKEY_KP_B;
    case SDLK_KP_C: return VKEY_KP_C;
    case SDLK_KP_D: return VKEY_KP_D;
    case SDLK_KP_E: return VKEY_KP_E;
    case SDLK_KP_F: return VKEY_KP_F;
    case SDLK_KP_XOR: return VKEY_KP_XOR;
    case SDLK_KP_POWER: return VKEY_KP_POWER;
    case SDLK_KP_PERCENT: return VKEY_KP_PERCENT;
    case SDLK_KP_LESS: return VKEY_KP_LESS;
    case SDLK_KP_GREATER: return VKEY_KP_GREATER;
    case SDLK_KP_AMPERSAND: return VKEY_KP_AMPERSAND;
    case SDLK_KP_DBLAMPERSAND: return VKEY_KP_DBLAMPERSAND;

    case SDLK_KP_VERTICALBAR: return VKEY_KP_VERTICALBAR;

    case SDLK_KP_DBLVERTICALBAR: return VKEY_KP_DBLVERTICALBAR;

    case SDLK_KP_COLON: return VKEY_KP_COLON;
    case SDLK_KP_HASH: return VKEY_KP_HASH;
    case SDLK_KP_SPACE: return VKEY_KP_SPACE;
    case SDLK_KP_AT: return VKEY_KP_AT;
    case SDLK_KP_EXCLAM: return VKEY_KP_EXCLAM;
    case SDLK_KP_MEMSTORE: return VKEY_KP_MEMSTORE;
    case SDLK_KP_MEMRECALL: return VKEY_KP_MEMRECALL;
    case SDLK_KP_MEMCLEAR: return VKEY_KP_MEMCLEAR;
    case SDLK_KP_MEMADD: return VKEY_KP_MEMADD;
    case SDLK_KP_MEMSUBTRACT: return VKEY_KP_MEMSUBTRACT;

    case SDLK_KP_MEMMULTIPLY: return VKEY_KP_MEMMULTIPLY;

    case SDLK_KP_MEMDIVIDE: return VKEY_KP_MEMDIVIDE;
    case SDLK_KP_PLUSMINUS: return VKEY_KP_PLUSMINUS;
    case SDLK_KP_CLEAR: return VKEY_KP_CLEAR;
    case SDLK_KP_CLEARENTRY: return VKEY_KP_CLEARENTRY;
    case SDLK_KP_BINARY: return VKEY_KP_BINARY;
    case SDLK_KP_OCTAL: return VKEY_KP_OCTAL;
    case SDLK_KP_DECIMAL: return VKEY_KP_DECIMAL;
    case SDLK_KP_HEXADECIMAL: return VKEY_KP_HEXADECIMAL;


    case SDLK_LCTRL: return VKEY_LCTRL;
    case SDLK_LSHIFT: return VKEY_LSHIFT;
    case SDLK_LALT: return VKEY_LALT;
    case SDLK_LGUI: return VKEY_LGUI;
    case SDLK_RCTRL: return VKEY_RCTRL;
    case SDLK_RSHIFT: return VKEY_RSHIFT;
    case SDLK_RALT: return VKEY_RALT;
    case SDLK_RGUI: return VKEY_RGUI;

    case SDLK_MODE: return VKEY_MODE;

    case SDLK_AUDIONEXT: return VKEY_AUDIONEXT;
    case SDLK_AUDIOPREV: return VKEY_AUDIOPREV;
    case SDLK_AUDIOSTOP: return VKEY_AUDIOSTOP;
    case SDLK_AUDIOPLAY: return VKEY_AUDIOPLAY;
    case SDLK_AUDIOMUTE: return VKEY_AUDIOMUTE;
    case SDLK_MEDIASELECT: return VKEY_MEDIASELECT;
    case SDLK_WWW: return VKEY_WWW;
    case SDLK_MAIL: return VKEY_MAIL;
    case SDLK_CALCULATOR: return VKEY_CALCULATOR;
    case SDLK_COMPUTER: return VKEY_COMPUTER;
    case SDLK_AC_SEARCH: return VKEY_AC_SEARCH;
    case SDLK_AC_HOME: return VKEY_AC_HOME;
    case SDLK_AC_BACK: return VKEY_AC_BACK;
    case SDLK_AC_FORWARD: return VKEY_AC_FORWARD;
    case SDLK_AC_STOP: return VKEY_AC_STOP;
    case SDLK_AC_REFRESH: return VKEY_AC_REFRESH;
    case SDLK_AC_BOOKMARKS: return VKEY_AC_BOOKMARKS;

    case SDLK_BRIGHTNESSDOWN: return VKEY_BRIGHTNESSDOWN;

    case SDLK_BRIGHTNESSUP: return VKEY_BRIGHTNESSUP;
    case SDLK_DISPLAYSWITCH: return VKEY_DISPLAYSWITCH;
    case SDLK_KBDILLUMTOGGLE: return VKEY_KBDILLUMTOGGLE;

    case SDLK_KBDILLUMDOWN: return VKEY_KBDILLUMDOWN;
    case SDLK_KBDILLUMUP: return VKEY_KBDILLUMUP;
    case SDLK_EJECT: return VKEY_EJECT;
    case SDLK_SLEEP: return VKEY_SLEEP;
    case SDLK_APP1: return VKEY_APP1;
    case SDLK_APP2: return VKEY_APP2;

    case SDLK_AUDIOREWIND: return VKEY_AUDIOREWIND;
    case SDLK_AUDIOFASTFORWARD: return VKEY_AUDIOFASTFORWARD;

    default:
        LOG_ERROR("Unknown SDL_Keycode: " + std::to_string(event.key.keysym.sym));
        return 0;
    }
}
