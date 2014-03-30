#ifndef LEVIATHAN_GUISCRIPTBIND
#define LEVIATHAN_GUISCRIPTBIND


#include "angelscript.h"
#include "GuiView.h"
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


	// bind BaseGuiObject //
	if(engine->RegisterObjectType("BaseGuiObject", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	// no factory function to prevent scripts from creating these functions //

	if(engine->RegisterObjectBehaviour("BaseGuiObject", asBEHAVE_ADDREF, "void f()", WRAP_MFN(Gui::BaseGuiObject, AddRefProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("BaseGuiObject", asBEHAVE_RELEASE, "void f()", WRAP_MFN(Gui::BaseGuiObject, ReleaseProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("BaseGuiObject", "int GetID()", WRAP_MFN(Gui::BaseGuiObject, GetID), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("BaseGuiObject", "ScriptSafeVariableBlock@ GetAndPopFirstUpdated()", WRAP_MFN(Gui::BaseGuiObject, GetAndPopFirstUpdated), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("BaseGuiObject", "GuiManager& GetOwningManager()", WRAP_MFN(Gui::BaseGuiObject, GetOwningManager), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}


	// Bind Gui::View //
	if(engine->RegisterObjectType("GuiView", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectBehaviour("GuiView", asBEHAVE_ADDREF, "void f()", WRAP_MFN(Gui::View, AddRefProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("GuiView", asBEHAVE_RELEASE, "void f()", WRAP_MFN(Gui::View, ReleaseProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	// Modifying elements //
	if(engine->RegisterObjectMethod("GuiView", "void ToggleElement(string &in elementid)", WRAP_MFN(Gui::View, ToggleElement), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}


	// Getting view from collection //
	if(engine->RegisterObjectMethod("GuiCollection", "GuiView@ GetContainingView()", WRAP_MFN(Gui::GuiCollection, GetContainingViewProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	



	return true;
}


void RegisterGUIScriptTypeNames(asIScriptEngine* engine, std::map<int, wstring> &typeids){

	typeids.insert(make_pair(engine->GetTypeIdByDecl("GuiCollection"), L"GuiCollection"));
	typeids.insert(make_pair(engine->GetTypeIdByDecl("BaseGuiObject"), L"BaseGuiObject"));
	typeids.insert(make_pair(engine->GetTypeIdByDecl("GuiView"), L"GuiView"));
}

#endif
