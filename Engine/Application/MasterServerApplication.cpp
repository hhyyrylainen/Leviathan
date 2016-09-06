// ------------------------------------ //
#include "MasterServerApplication.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::MasterServerApplication::MasterServerApplication(){

}

DLLEXPORT Leviathan::MasterServerApplication::~MasterServerApplication(){

}
// ------------------------------------ //
bool Leviathan::MasterServerApplication::PassCommandLine(int argcount, char* args[]){

    // Force nogui //
    _Engine->SetNoGUI();
    
    // Now pass it //
    return _Engine->PassCommandLine(argcount, args);
}




