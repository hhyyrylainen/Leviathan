// ------------------------------------ //
#include "CrashHandler.h"

#include "bsfCore/BsCorePrerequisites.h"
#include "bsfUtility/Error/BsCrashHandler.h"

using namespace Leviathan;
// ------------------------------------ //
std::function<void()> CrashHandler::GenericCallback;
std::function<void(void* data)> CrashHandler::SEHCallback;
bool CrashHandler::BSFTriggered = false;
// ------------------------------------ //
void CrashHandler::DoBreakpadCrashDumpIfRegistered()
{
    BSFTriggered = true;
    if(GenericCallback)
        GenericCallback();
}

void CrashHandler::DoBreakpadSEHCrashDumpIfRegistered(void* data)
{
    BSFTriggered = true;
    if(SEHCallback)
        SEHCallback(data);
}
// ------------------------------------ //
DLLEXPORT void CrashHandler::RegisterBreakpadGenericHandler(std::function<void()> callback)
{
    GenericCallback = callback;
}

DLLEXPORT void CrashHandler::RegisterBreakpadWindowsSEHHandler(
    std::function<void(void* data)> callback)
{
    SEHCallback = callback;
}
// ------------------------------------ //
DLLEXPORT void CrashHandler::DoBSFCallStackAfterBreakpad()
{
    if(!BSFTriggered) {
        BSFTriggered = true;

        auto log = Logger::Get();

        if(log) {

            log->Write("Attempting to dump stack trace with BSF:");
            log->Write(bs::CrashHandler::getStackTrace().c_str());
            log->Save();
        }
    }
}
// ------------------------------------ //
DLLEXPORT bool CrashHandler::IsBreakpadRegistered()
{
    return GenericCallback || SEHCallback;
}
