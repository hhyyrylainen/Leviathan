#ifndef LEVIATHAN_GUISCRIPTBIND
#define LEVIATHAN_GUISCRIPTBIND


#include "angelscript.h"
#include "GuiScriptInterface.h"
#include "BaseGuiObject.h"
#include "GuiManager.h"
#include "add_on/autowrapper/aswrappedcall.h"
#include "CEGUI/Window.h"
#include "CEGUI/widgets/TabControl.h"

void CEGUIWindowSetTextProxy(CEGUI::Window* obj, const string &text){
	
	obj->setText(text);
}

string CEGUIWindowGetTextProxy(CEGUI::Window* obj, const string &text){

	return string(obj->getText().c_str());
}

CEGUI::Window* CEGUIWindowGetChildWindowProxy(CEGUI::Window* obj, const string &text){

	return obj->getChild(text);
}

void CEGUIWindowSetSizeProxy(CEGUI::Window* obj, float width, float widthpixels, float height, float heightpixels){

	obj->setSize(CEGUI::USize(CEGUI::UDim(width, widthpixels), CEGUI::UDim(height, heightpixels)));
}

void CEGUIWindowInvalidateProxy(CEGUI::Window* obj, bool recursive){

	obj->invalidate(recursive);
}


void CEGUITabControlSetActiveTabIndex(CEGUI::Window* obj, int index){

	CEGUI::TabControl* convtabs = dynamic_cast<CEGUI::TabControl*>(obj);
	if(convtabs != NULL){
		
		convtabs->setSelectedTabAtIndex(index);
	}
}


template<class From, class To>
To* DoReferenceCastDynamicNoRef(From* ptr){
	// If already invalid just return it //
	if(!ptr)
		return NULL;

	To* newptr = dynamic_cast<To*>(ptr);

	// Return the ptr (which might be invalid) //
	return newptr;
}

template<class From, class To>
To* DoReferenceCastStaticNoRef(From* ptr){
	// If already invalid just return it //
	if(!ptr)
		return NULL;

	To* newptr = static_cast<To*>(ptr);

	// Return the ptr (which might be invalid) //
	return newptr;
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
	
	if(engine->RegisterObjectMethod("GuiObject", "void PrintWindowsRecursive()", asMETHOD(Gui::BaseGuiObject, PrintWindowsRecursiveProxy), asCALL_THISCALL) < 0)
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

	if(engine->RegisterObjectMethod("Window", "string GetText()", asFUNCTION(CEGUIWindowGetTextProxy), asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("Window", "Window& GetChildWindow(const string &in namepath)", asFUNCTION(CEGUIWindowGetChildWindowProxy), asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("Window", "void SetSize(float width, float widthpixels, float height, float heightpixels)", 
		asFUNCTION(CEGUIWindowSetSizeProxy), asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("Window", "void Invalidate(bool recursive = false)", asFUNCTION(CEGUIWindowInvalidateProxy), asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("Window", "void SetSelectedTabIndex(int index)", asFUNCTION(CEGUITabControlSetActiveTabIndex), asCALL_CDECL_OBJFIRST) < 0)
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
	if(engine->RegisterObjectMethod("GuiObject", "CEGUI::Window& GetTargetElement()", asMETHOD(Gui::BaseGuiObject, GetTargetWindow), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("GuiObject", "CEGUI::Window& GetTargetWindow()", asMETHOD(Gui::BaseGuiObject, GetTargetWindow), asCALL_THISCALL) < 0)
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
