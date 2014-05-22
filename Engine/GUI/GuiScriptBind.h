#ifndef LEVIATHAN_GUISCRIPTBIND
#define LEVIATHAN_GUISCRIPTBIND


#include "angelscript.h"
#include "GuiScriptInterface.h"
#include "BaseGuiObject.h"
#include "GuiManager.h"
#include "add_on/autowrapper/aswrappedcall.h"


bool BindGUIObjects(asIScriptEngine* engine){



	// bind GuiCollection action, this is released by gui object //
	if(engine->RegisterObjectType("GuiCollection", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectBehaviour("GuiCollection", asBEHAVE_ADDREF, "void f()", WRAP_MFN(Gui::GuiCollection, AddRefProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("GuiCollection", asBEHAVE_RELEASE, "void f()", WRAP_MFN(Gui::GuiCollection, ReleaseProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("GuiCollection", "string GetName()", WRAP_MFN(Gui::GuiCollection, GetNameProxy), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	// GuiManager needed to use some functionality, registered so that it cannot be stored //
	if(engine->RegisterObjectType("GuiManager", 0, asOBJ_REF | asOBJ_NOHANDLE) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("GuiManager", "bool SetCollectionState(string name, bool state = false)", WRAP_MFN(Gui::GuiManager, SetCollectionStateProxy), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}


	// bind GuiObject //
	if(engine->RegisterObjectType("GuiObject", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	// no factory function to prevent scripts from creating these functions //

	if(engine->RegisterObjectBehaviour("GuiObject", asBEHAVE_ADDREF, "void f()", WRAP_MFN(Gui::BaseGuiObject, AddRefProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("GuiObject", asBEHAVE_RELEASE, "void f()", WRAP_MFN(Gui::BaseGuiObject, ReleaseProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("GuiObject", "int GetID()", WRAP_MFN(Gui::BaseGuiObject, GetID), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("GuiObject", "ScriptSafeVariableBlock@ GetAndPopFirstUpdated()", WRAP_MFN(Gui::BaseGuiObject, GetAndPopFirstUpdated), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("GuiObject", "GuiManager& GetOwningManager()", WRAP_MFN(Gui::BaseGuiObject, GetOwningManager), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	
	if(engine->SetDefaultNamespace("CEGUI") < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	//if(engine->RegisterObjectType("Event", 0, asOBJ_REF) < 0){
	//	ANGELSCRIPT_REGISTERFAIL;
	//}




	// Restore the namespace //
	if(engine->SetDefaultNamespace("") < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	return true;
}


void RegisterGUIScriptTypeNames(asIScriptEngine* engine, std::map<int, wstring> &typeids){

	typeids.insert(make_pair(engine->GetTypeIdByDecl("GuiCollection"), L"GuiCollection"));
	typeids.insert(make_pair(engine->GetTypeIdByDecl("GuiObject"), L"GuiObject"));
}

#endif
