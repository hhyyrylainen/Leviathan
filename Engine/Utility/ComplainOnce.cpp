// ------------------------------------ //
#include "ComplainOnce.h"

#include "../Logger.h"
using namespace Leviathan;
// ------------------------------------ //


bool ComplainOnce::ThreadUnsafeComplainOnce::PrintWarningOnce(
    const std::string& warning, const std::string& message)
{
    // Print only once to log //
    if(wasErrorLogged(warning)) return false;

    // Value wasn't there, print //
    Logger::Get()->Warning(message);

    FiredErrors.push_back(std::make_shared<std::string>(warning));
    return true;
}

bool ComplainOnce::ThreadUnsafeComplainOnce::PrintErrorOnce(
    const std::string& error, const std::string& message)
{
    // Print only once to log //
    if(wasErrorLogged(error)) return false;

    // Value wasn't there, print //
    Logger::Get()->Error(message);

    FiredErrors.push_back(std::make_shared<std::string>(error));
    return true;
}

bool ComplainOnce::ThreadUnsafeComplainOnce::wasErrorLogged(const std::string& warning)
{
    for(auto iter = FiredErrors.begin(); iter != FiredErrors.end(); ++iter)
        if(**iter == warning) return true;

	return false;
}

DLLEXPORT bool Leviathan::ComplainOnce::PrintWarningOnce(const std::string& warning,
    const std::string& message)
{
    return mon->PrintWarningOnce(warning, message);
}

DLLEXPORT  bool Leviathan::ComplainOnce::PrintErrorOnce(const std::string& error,
    const std::string& message)
{
    return mon->PrintErrorOnce(error, message);
}

Monitor<ComplainOnce::ThreadUnsafeComplainOnce> ComplainOnce::mon;
