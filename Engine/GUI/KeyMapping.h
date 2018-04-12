// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //

//! If set to 1 will print errors about unknown keys. Otherwise they are just ignored
//! Text input works well even with unknown keys
#define LEVIATHAN_PRINT_ERRORS_ABOUT_UNKOWN_KEYS 0

union SDL_Event;
struct SDL_Keysym;

enum KeyboardCode : int;

namespace Leviathan { namespace KeyMapping {


DLLEXPORT int GetCEFButtonFromSdlMouseButton(uint32_t whichbutton);

DLLEXPORT int32_t ConvertStringToKeyCode(const std::string& str);
DLLEXPORT std::string ConvertKeyCodeToString(const int32_t& code);


DLLEXPORT int SDLKeyToX11Key(const SDL_Keysym& key);


// CEF part

DLLEXPORT KeyboardCode GdkEventToWindowsKeyCode(int key);
DLLEXPORT KeyboardCode GetWindowsKeyCodeWithoutLocation(KeyboardCode key_code);
DLLEXPORT int GetControlCharacter(KeyboardCode windows_key_code, bool shift);




}} // namespace Leviathan::KeyMapping
