// ------------------------------------ //
#include "ComplainOnce.h"

#include "../Logger.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT bool Leviathan::ComplainOnce::PrintWarningOnce(const std::string& warning,
    const std::string& message)
{
	// Print only once to log //
    auto end = FiredErrors.end();
    for(auto iter = FiredErrors.begin(); iter != end; ++iter){

        if(**iter == warning)
            return false;
    }
    
    // Value wasn't there, print //
    Logger::Get()->Warning(message);
    
    FiredErrors.push_back(std::make_shared<std::string>(warning));
    return true;
}

DLLEXPORT  bool Leviathan::ComplainOnce::PrintErrorOnce(const std::string& error,
    const std::string& message)
{
	// Print only once to log //
    auto end = FiredErrors.end();
    for(auto iter = FiredErrors.begin(); iter != end; ++iter){

        if(**iter == error)
            return false;
    }
    
    // Value wasn't there, print //
    Logger::Get()->Error(message);
    
    FiredErrors.push_back(std::make_shared<std::string>(error));
    return true;
}

std::vector<std::shared_ptr<std::string>> ComplainOnce::FiredErrors;
// ------------------------------------ //


