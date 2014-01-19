#ifndef LEVIATHAN_GUISCRIPTBIND
#define LEVIATHAN_GUISCRIPTBIND


#include "angelscript.h"
#include "GuiAnimation.h"
#include "GuiScriptInterface.h"
#include "BaseGuiObject.h"
#include "GuiManager.h"
#include "add_on\autowrapper\aswrappedcall.h"

string RocketProxyEventGetValue(Rocket::Core::Event* evt, string valuename){
	// Get the parameter from the event //
	auto strtype = evt->GetParameter(valuename.c_str(), Rocket::Core::String(""));
	// Convert and return //
	return string(strtype.CString());
}

void RocketElementProxySetProperty(asIScriptGeneric* gen){
	// Get arguments //
	Rocket::Core::Element* element = reinterpret_cast<Rocket::Core::Element*>(gen->GetObject());
	string* propertyname = reinterpret_cast<string*>(gen->GetArgObject(0));
	string* valuetoset = reinterpret_cast<string*>(gen->GetArgObject(1));

	element->SetProperty(propertyname->c_str(), valuetoset->c_str());
}

void RocketElementProxySetInternalRML(asIScriptGeneric* gen){
	// Get arguments //
	Rocket::Core::Element* element = reinterpret_cast<Rocket::Core::Element*>(gen->GetObject());
	string* rml = reinterpret_cast<string*>(gen->GetArgObject(0));

	element->SetInnerRML(rml->c_str());
}


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
	if(engine->RegisterObjectMethod("GuiManager", "void GUIObjectsCheckRocketLinkage()", WRAP_MFN(Gui::GuiManager, GUIObjectsCheckRocketLinkage), asCALL_GENERIC) < 0)
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
	if(engine->RegisterObjectMethod("BaseGuiObject", "int SetInternalElementRML(string rmlcode)", WRAP_MFN(Gui::BaseGuiObject, SetInternalRMLWrapper), asCALL_GENERIC) < 0)
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


	
	// rocket objects //
	if(engine->RegisterObjectType("RocketEvent", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("RocketEvent", asBEHAVE_ADDREF, "void f()", WRAP_MFN(Rocket::Core::Event, AddReference), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("RocketEvent", asBEHAVE_RELEASE, "void f()", WRAP_MFN(Rocket::Core::Event, RemoveReference), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("RocketEvent", "string GetValue(string name)", WRAP_FN(RocketProxyEventGetValue), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	

	// rocket element //
	if(engine->RegisterObjectType("RocketElement", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("RocketElement", asBEHAVE_ADDREF, "void f()", WRAP_MFN(Rocket::Core::Element, AddReference), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("RocketElement", asBEHAVE_RELEASE, "void f()", WRAP_MFN(Rocket::Core::Element, RemoveReference), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("RocketElement", "void SetProperty(string &in property, string &in value)", asFUNCTION(RocketElementProxySetProperty), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("RocketElement", "void SetInternalRML(string &in rml)", asFUNCTION(RocketElementProxySetInternalRML), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	
	
	// bind sheets //
	if(engine->RegisterObjectType("GuiLoadedSheet", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectBehaviour("GuiLoadedSheet", asBEHAVE_ADDREF, "void f()", WRAP_MFN(Gui::GuiLoadedSheet, AddRefProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("GuiLoadedSheet", asBEHAVE_RELEASE, "void f()", WRAP_MFN(Gui::GuiLoadedSheet, ReleaseProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("GuiLoadedSheet", "void PullSheetToFront()", WRAP_MFN(Gui::GuiLoadedSheet, PullSheetToFront), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("GuiLoadedSheet", "void PushSheetToBack()", WRAP_MFN(Gui::GuiLoadedSheet, PushSheetToBack), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	
	if(engine->RegisterObjectMethod("GuiLoadedSheet", "RocketElement@ GetElementByID(string id)", WRAP_MFN(Gui::GuiLoadedSheet, GetElementByIDProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("GuiCollection", "GuiLoadedSheet@ GetOwningSheet()", WRAP_MFN(Gui::GuiCollection, GetOwningSheetProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}


	
		


	return true;
}


void RegisterGUIScriptTypeNames(asIScriptEngine* engine, std::map<int, wstring> &typeids){

	typeids.insert(make_pair(engine->GetTypeIdByDecl("GuiCollection"), L"GuiCollection"));
	typeids.insert(make_pair(engine->GetTypeIdByDecl("BaseGuiObject"), L"BaseGuiObject"));
	typeids.insert(make_pair(engine->GetTypeIdByDecl("RocketEvent"), L"RocketEvent"));
}

#endif