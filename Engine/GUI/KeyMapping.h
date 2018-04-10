// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //

union SDL_Event;
struct SDL_Keysym;

enum KeyboardCode : int;

namespace Leviathan { namespace KeyMapping {

DLLEXPORT int SDLKeyToX11Key(const SDL_Keysym& key);


// CEF part

DLLEXPORT KeyboardCode GdkEventToWindowsKeyCode(int key);
DLLEXPORT KeyboardCode GetWindowsKeyCodeWithoutLocation(KeyboardCode key_code);
DLLEXPORT int GetControlCharacter(KeyboardCode windows_key_code, bool shift);




}} // namespace Leviathan::KeyMapping
