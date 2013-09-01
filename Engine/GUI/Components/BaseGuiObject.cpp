#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUI_BASEOBJECT
#include "BaseGuiObject.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::Gui::BaseGuiObject::BaseGuiObject(GuiManager* owner, const int &flags, const int &id, const int &objecttype, 
	shared_ptr<ScriptScript> script /*= NULL*/) : OwningInstance(owner), ObjectFlags(flags), ID(id), ObjectType(objecttype), Scripting(script)
{

}

DLLEXPORT Leviathan::Gui::BaseGuiObject::~BaseGuiObject(){
	// script has smart pointer //

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::BaseGuiObject::LoadFromFileStructure(GuiManager* owner, vector<BaseGuiObject*> &tempobjects, vector<Int2> &idmappairs, 
	ObjectFileObject& dataforthis)
{
	return false;
}
// ------------------------------------ //




