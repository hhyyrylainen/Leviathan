// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //

union SDL_Event;

namespace Leviathan { namespace KeyMapping {

DLLEXPORT int GetWindowsKeyCodeFromSDLEvent(const SDL_Event& event);

}} // namespace Leviathan::KeyMapping
