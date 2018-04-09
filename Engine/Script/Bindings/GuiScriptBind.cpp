// ------------------------------------ //
#include "GuiScriptBind.h"

#include "GUI/GuiManager.h"

#include "FileSystem.h"
#include "add_on/autowrapper/aswrappedcall.h"

#ifdef _WIN32
#include "WindowsInclude.h"
#endif // _WIN32

using namespace Leviathan;
// ------------------------------------ //

// Proxies etc.
// ------------------------------------ //

// ------------------------------------ //
// Start of the actual bind
namespace Leviathan {

} // namespace Leviathan

bool Leviathan::BindGUI(asIScriptEngine* engine)
{
    // GuiManager needed to use some functionality, registered so that it cannot be stored //
    if(engine->RegisterObjectType("GuiManager", 0, asOBJ_REF | asOBJ_NOHANDLE) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}
