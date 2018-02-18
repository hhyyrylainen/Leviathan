// ------------------------------------ //
#include "ScriptRunningSetup.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT ScriptRunningSetup::ScriptRunningSetup() {}

DLLEXPORT ScriptRunningSetup::ScriptRunningSetup(const std::string& entrypoint) :
    Entryfunction(entrypoint)
{
}

DLLEXPORT ScriptRunningSetup::~ScriptRunningSetup() {}
