#pragma once
// ------------------------------------ //
#include "Include.h"
#include "Threading/Monitor.h"

#include <memory>
#include <string>
#include <vector>

namespace Leviathan {

class ComplainOnce {
public:
    ComplainOnce() = delete;

    DLLEXPORT static bool PrintWarningOnce(
        const std::string& warning, const std::string& message);
    DLLEXPORT static bool PrintErrorOnce(const std::string& error, const std::string& message);

private:
    class ThreadUnsafeComplainOnce {
    public:
        bool PrintWarningOnce(const std::string& warning, const std::string& message);
        bool PrintErrorOnce(const std::string& error, const std::string& message);

    private:
        bool wasErrorLogged(const std::string& warning);

        // fired warnings/errors //
        std::vector<std::shared_ptr<std::string>> FiredErrors;
    };

    static Monitor<ThreadUnsafeComplainOnce> mon;
};

} // namespace Leviathan
