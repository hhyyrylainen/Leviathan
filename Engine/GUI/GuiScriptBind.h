#ifndef LEVIATHAN_GUISCRIPTBIND
#define LEVIATHAN_GUISCRIPTBIND


#include "angelscript.h"
#include "GuiAnimation.h"
#include "GuiScriptInterface.h"
#include "BaseGuiObject.h"
#include "GuiManager.h"


void RocketProxyAddEventReference(Rocket::Core::Event* evt){

	evt->AddReference();
}
void RocketProxyReleaseEventReference(Rocket::Core::Event* evt){

	evt->RemoveReference();
}

string RocketProxyEventGetValue(Rocket::Core::Event* evt, string valuename){
	// Get the parameter from the event //
	auto strtype = evt->GetParameter(valuename.c_str(), Rocket::Core::String(""));
	// Convert and return //
	return string(strtype.CString());
}

void RocketProxyAddElementReference(Rocket::Core::Element* element){

	element->AddReference();
}
void RocketProxyReleaseElementReference(Rocket::Core::Element* element){

	element->RemoveReference();
}
void RocketElementProxySetProperty(Rocket::Core::Element* element, string propertyname, string valuetoset){
	element->SetProperty(propertyname.c_str(), valuetoset.c_str());
}
void RocketElementProxySetInternalRML(Rocket::Core::Element* element, string rml){

	element->SetInnerRML(rml.c_str());
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
	if(engine->RegisterObjectMethod("GuiManager", "void GUIObjectsCheckRocketLinkage()", asMETHOD(Gui::GuiManager, GUIObjectsCheckRocketLinkage), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}





	// bind BaseGuiObject //
	if(engine->RegisterObjectType("BaseGuiObject", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	// no factory function to prevent scripts from creating these functions //

	if(engine->RegisterObjectBehaviour("BaseGuiObject", asBEHAVE_ADDREF, "void f()", asMETHOD(Gui::BaseGuiObject, AddRefProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("BaseGuiObject", asBEHAVE_RELEASE, "void f()", asMETHOD(Gui::BaseGuiObject, ReleaseProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("BaseGuiObject", "int GetID()", asMETHOD(Gui::BaseGuiObject, GetID), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("BaseGuiObject", "int SetInternalElementRML(string rmlcode)", asMETHOD(Gui::BaseGuiObject, SetInternalRMLWrapper), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("BaseGuiObject", "ScriptSafeVariableBlock@ GetAndPopFirstUpdated()", asMETHOD(Gui::BaseGuiObject, GetAndPopFirstUpdatedProxy), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("BaseGuiObject", "GuiManager& GetOwningManager()", asMETHOD(Gui::BaseGuiObject, GetOwningManager), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}


	
	// rocket objects //
	if(engine->RegisterObjectType("RocketEvent", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("RocketEvent", asBEHAVE_ADDREF, "void f()", asFUNCTION(RocketProxyAddEventReference), asCALL_CDECL_OBJFIRST) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("RocketEvent", asBEHAVE_RELEASE, "void f()", asFUNCTION(RocketProxyReleaseEventReference), asCALL_CDECL_OBJFIRST) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("RocketEvent", "string GetValue(string name)", asFUNCTION(RocketProxyEventGetValue), asCALL_CDECL_OBJFIRST) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	

	// rocket element //
	if(engine->RegisterObjectType("RocketElement", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("RocketElement", asBEHAVE_ADDREF, "void f()", asFUNCTION(RocketProxyAddElementReference), asCALL_CDECL_OBJFIRST) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("RocketElement", asBEHAVE_RELEASE, "void f()", asFUNCTION(RocketProxyReleaseElementReference), asCALL_CDECL_OBJFIRST) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("RocketElement", "void SetProperty(string property, string value)", asFUNCTION(RocketElementProxySetProperty), asCALL_CDECL_OBJFIRST) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("RocketElement", "void SetInternalRML(string rml)", asFUNCTION(RocketElementProxySetInternalRML), asCALL_CDECL_OBJFIRST) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	
	
	// bind sheets //
	if(engine->RegisterObjectType("GuiLoadedSheet", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectBehaviour("GuiLoadedSheet", asBEHAVE_ADDREF, "void f()", asMETHOD(Gui::GuiLoadedSheet, AddRefProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("GuiLoadedSheet", asBEHAVE_RELEASE, "void f()", asMETHOD(Gui::GuiLoadedSheet, ReleaseProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("GuiLoadedSheet", "void PullSheetToFront()", asMETHOD(Gui::GuiLoadedSheet, PullSheetToFront), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("GuiLoadedSheet", "void PushSheetToBack()", asMETHOD(Gui::GuiLoadedSheet, PushSheetToBack), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	
	if(engine->RegisterObjectMethod("GuiLoadedSheet", "RocketElement@ GetElementByID(string id)", asMETHOD(Gui::GuiLoadedSheet, GetElementByIDProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("GuiCollection", "GuiLoadedSheet@ GetOwningSheet()", asMETHOD(Gui::GuiCollection, GetOwningSheetProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("BaseGuiObject", "GuiLoadedSheet@ GetOwningSheet()", asMETHOD(Gui::BaseGuiObject, GetOwningSheetProxy), asCALL_THISCALL) < 0){
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