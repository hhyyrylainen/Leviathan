#ifndef LEVIATHAN_GUISCRIPTBIND
#define LEVIATHAN_GUISCRIPTBIND


#include "angelscript.h"
#include "GuiScriptInterface.h"
#include "BaseGuiObject.h"
#include "GuiManager.h"
#include "add_on/autowrapper/aswrappedcall.h"
#include "CEGUI/Window.h"


void CEGUIWindowSetTextProxy(CEGUI::Window* obj, const string &text){
	
	obj->setText(text);
}



bool BindGUIObjects(asIScriptEngine* engine){



	// bind GuiCollection action, this is released by gui object //
	if(engine->RegisterObjectType("GuiCollection", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectBehaviour("GuiCollection", asBEHAVE_ADDREF, "void f()", asMETHOD(Gui::GuiCollection, AddRefProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("GuiCollection", asBEHAVE_RELEASE, "void f()", asMETHOD(Gui::GuiCollection, ReleaseProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("GuiCollection", "string GetName()", asMETHOD(Gui::GuiCollection, GetNameProxy), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	// GuiManager needed to use some functionality, registered so that it cannot be stored //
	if(engine->RegisterObjectType("GuiManager", 0, asOBJ_REF | asOBJ_NOHANDLE) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("GuiManager", "bool SetCollectionState(string name, bool state = false)", asMETHOD(Gui::GuiManager, SetCollectionStateProxy), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}


	// bind GuiObject //
	if(engine->RegisterObjectType("GuiObject", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	// no factory function to prevent scripts from creating these functions //

	if(engine->RegisterObjectBehaviour("GuiObject", asBEHAVE_ADDREF, "void f()", asMETHOD(Gui::BaseGuiObject, AddRefProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("GuiObject", asBEHAVE_RELEASE, "void f()", asMETHOD(Gui::BaseGuiObject, ReleaseProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("GuiObject", "int GetID()", asMETHOD(Gui::BaseGuiObject, GetID), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("GuiObject", "ScriptSafeVariableBlock@ GetAndPopFirstUpdated()", asMETHOD(Gui::BaseGuiObject, GetAndPopFirstUpdated), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("GuiObject", "GuiManager& GetOwningManager()", asMETHOD(Gui::BaseGuiObject, GetOwningManager), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("GuiObject", "string GetName()", asMETHOD(Gui::BaseGuiObject, GetNameAsString), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	

	
	if(engine->SetDefaultNamespace("CEGUI") < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectType("Window", 0, asOBJ_REF | asOBJ_NOHANDLE) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("Window", "void SetText(const string &in newtext)", asFUNCTION(CEGUIWindowSetTextProxy), asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	// Restore the namespace //
	if(engine->SetDefaultNamespace("") < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("GuiManager", "CEGUI::Window& GetWindowByName(const string &in namepath)", asMETHOD(Gui::GuiManager, GetWindowByStringName), asCALL_THISCALL) < 0)
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
