// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
//! \file This file has forward declarations for all the individual bind functions
// ------------------------------------ //
#include "BindHelpers.h"

namespace Leviathan {

bool BindEntity(asIScriptEngine* engine);

bool BindTypes(asIScriptEngine* engine);

bool BindPhysics(asIScriptEngine* engine);

bool BindGUI(asIScriptEngine* engine);

bool BindEngineCommon(asIScriptEngine* engine);

bool BindRendering(asIScriptEngine* engine);

//! Binds standard and other utilities like: std::min, std::max etc
bool BindStandardFunctions(asIScriptEngine* engine);

} // namespace Leviathan
