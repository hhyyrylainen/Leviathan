// ------------------------------------ //
#include "ComplainOnce.h"

#include "Logger.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT bool Leviathan::ComplainOnce::PrintWarningOnce(
    const std::string& warning, const std::string& message)
{
    Lock lock(ErrorsMutex);

    // Print only once to log //
    if(FiredErrors.find(warning) != FiredErrors.end())
        return false;

    // Value wasn't there, print //
    Logger::Get()->Warning(message);

    FiredErrors.insert(warning);
    return true;
}

DLLEXPORT bool Leviathan::ComplainOnce::PrintErrorOnce(
    const std::string& error, const std::string& message)
{
    Lock lock(ErrorsMutex);

    // Print only once to log //
    if(FiredErrors.find(error) != FiredErrors.end())
        return false;

    // Value wasn't there, print //
    Logger::Get()->Error(message);

    FiredErrors.insert(error);
    return true;
}

std::unordered_set<std::string> ComplainOnce::FiredErrors;
Mutex ComplainOnce::ErrorsMutex;
// ------------------------------------ //
