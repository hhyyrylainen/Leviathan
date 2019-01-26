// ------------------------------------ //
#include "GameConfiguration.h"

#include "FileSystem.h"

#include <boost/filesystem.hpp>
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::GameConfiguration::GameConfiguration(const std::string& configfile) :
    GameConfigFile(configfile)
{
    staticaccess = this;
}

DLLEXPORT GameConfiguration::GameConfiguration() : GameVars(new NamedVars()), InMemory(true)
{
    staticaccess = this;
}

DLLEXPORT Leviathan::GameConfiguration::~GameConfiguration()
{
    if(GameVars)
        Release();

    staticaccess = NULL;
}

DLLEXPORT GameConfiguration* Leviathan::GameConfiguration::Get()
{
    return staticaccess;
}

GameConfiguration* Leviathan::GameConfiguration::staticaccess = NULL;
// ------------------------------------ //
DLLEXPORT bool Leviathan::GameConfiguration::Init(
    std::function<void(Lock& guard, GameConfiguration* configobj)> sanitycheckcallback)
{
    GUARD_LOCK();

    if(!InMemory) {

        GameVars = new NamedVars();

        if(!GameVars->LoadVarsFromFile(GameConfigFile, Logger::Get())) {

            if(boost::filesystem::exists(GameConfigFile)) {
                // Unknown error //
                Logger::Get()->Error("GameConfiguration: Unknown error from LoadVarsFromFile");
                return false;
            }
            // Ignore missing file
        }
    }

    // First verify the global variables //
    VerifyGlobalVariables(guard);

    // Call the checking function //
    if(sanitycheckcallback)
        sanitycheckcallback(guard, this);
    return true;
}

DLLEXPORT void Leviathan::GameConfiguration::Release()
{
    SaveCheck();

    // We can now delete our variables //
    GUARD_LOCK();
    SAFE_DELETE(GameVars);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameConfiguration::SaveCheck()
{
    if(InMemory)
        return;

    std::string newfilecontents = "";
    // Writing to file doesn't need locking //
    {
        GUARD_LOCK();
        // If not modified we don't need to save anything //
        if(!Modified)
            return;

        // Write the variables to the file //
        auto vec = GameVars->GetVec();

        for(size_t i = 0; i < vec->size(); i++) {

            newfilecontents += vec->at(i)->ToText() + "\n";
        }

        // No longer needs to save modified values //
        Modified = false;
    }

    FileSystem::WriteToFile(newfilecontents, GameConfigFile);
}

DLLEXPORT void Leviathan::GameConfiguration::MarkModified(Lock& guard)
{
    Modified = true;
}
// ------------------------------------ //
DLLEXPORT NamedVars* Leviathan::GameConfiguration::AccessVariables(Lock& guard)
{
    VerifyLock(guard);

    return GameVars;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameConfiguration::VerifyGlobalVariables(Lock& guard)
{
    // Socket unbind control //
    if(GameVars->ShouldAddValueIfNotFoundOrWrongType<bool>("DisableSocketUnbind")) {
        // Add new //
        GameVars->AddVar("DisableSocketUnbind", new VariableBlock(false));
        MarkModified(guard);
    }
}
