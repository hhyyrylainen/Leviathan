// ------------------------------------ //
#include "ServerApplication.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::ServerApplication::ServerApplication(){

}

DLLEXPORT Leviathan::ServerApplication::~ServerApplication(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ServerApplication::PassCommandLine(int argcount, char* args[]){

    // Force nogui //
    _Engine->SetNoGUI();
    
    // Now pass it //
    return _Engine->PassCommandLine(argcount, args);
}


