// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#include "Define.h"
// ------------------------------------ //

namespace Leviathan {

//! \brief This returns a mime type based on the file extension
//!
//! This could be extended in the future to actually read the file header
DLLEXPORT std::string_view GetMimeTypeFromPath(const std::string_view& path);
} // namespace Leviathan
