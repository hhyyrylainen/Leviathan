// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "Common/ThreadSafe.h"

#include <unordered_set>

namespace Leviathan {

//! \brief Prints a warning or an error once
class ComplainOnce {
public:
    ComplainOnce() = delete;
    DLLEXPORT static bool PrintWarningOnce(
        const std::string& warning, const std::string& message);
    DLLEXPORT static bool PrintErrorOnce(const std::string& error, const std::string& message);

private:
    //! fired warnings/errors
    static std::unordered_set<std::string> FiredErrors;

    static Mutex ErrorsMutex;
};

} // namespace Leviathan
