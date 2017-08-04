#pragma once

#include "angelscript.h"
#include "GuiScriptInterface.h"
#include "BaseGuiObject.h"
#include "GuiManager.h"
#include "add_on/autowrapper/aswrappedcall.h"
#include "../CEGUIInclude.h"
#include "FileSystem.h"

using namespace std;

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

void CEGUIWindowSetDisabledState(CEGUI::Window* obj, bool disabled){

    obj->setEnabled(!disabled);
}


bool CEGUITabControlSetActiveTabIndex(CEGUI::Window* obj, int index){

	CEGUI::TabControl* convtabs = dynamic_cast<CEGUI::TabControl*>(obj);
	if(convtabs != NULL){
		
		convtabs->setSelectedTabAtIndex(index);
		return true;
	}

	return false;
}

bool CEGUIComboboxSetSelectedItem(CEGUI::Window* obj, const string &text){

    CEGUI::Combobox* convbox = dynamic_cast<CEGUI::Combobox*>(obj);

    if(!convbox)
        return false;

    CEGUI::StandardItem* wanted = convbox->findItemWithText(CEGUI::String(text), NULL);

    if(!wanted)
        return false;

    // Skip setting it as selected if it already is //
    if(wanted == convbox->getSelectedItem())
        return true;

    convbox->clearAllSelections();
    convbox->setItemSelectState(wanted, true);

    return true;    
}

bool CEGUIComboboxAddItem(CEGUI::Window* obj, const string &text){

    CEGUI::Combobox* convbox = dynamic_cast<CEGUI::Combobox*>(obj);
    
    if(!convbox)
        return false;
    
    convbox->addItem(new CEGUI::StandardItem(CEGUI::String(text)));

    return true;
}

bool CEGUIComboboxClearItems(CEGUI::Window* obj){

    CEGUI::Combobox* convbox = dynamic_cast<CEGUI::Combobox*>(obj);

    if(!convbox)
        return false;

    convbox->resetList();

    return true;
}

bool CEGUIAdvancedCreateTabFromFile(CEGUI::Window* obj, const string &filename, const string &tabname,
    const string &lookfor, const string &replacer)
{

	// Find the file //
	const string onlyname = StringOperations::RemoveExtensionString(filename, true);

	const string &targetfile = FileSystem::Get()->SearchForFile(FILEGROUP_SCRIPT, onlyname,
        StringOperations::GetExtensionString(filename), true);

	if(targetfile.empty()){

		// The file wasn't found //
		return false;
	}


	CEGUI::TabControl* convtabs = dynamic_cast<CEGUI::TabControl*>(obj);
	if(convtabs == NULL){

		// Failed the conversion //
		return false;
	}


	// Load the file to memory //
	string filecontents;

	FileSystem::ReadFileEntirely(targetfile, filecontents);

	if(filecontents.empty())
		return false;

	// Replace the thing //
	filecontents = StringOperations::Replace<string>(filecontents, lookfor, replacer);

	if(filecontents.empty())
		return false;

	auto newwindow = CEGUI::WindowManager::getSingleton().loadLayoutFromString(filecontents);

	if(!newwindow)
		return false;

	// Add as a tab //
	convtabs->addTab(newwindow);


	// This makes sure that the name is right //
	convtabs->getChild(CEGUI::String("__auto_TabPane__Buttons/__auto_btn")+
        newwindow->getName())->setText(tabname);

	// Succeeded //
	return true;
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

	if(engine->RegisterObjectBehaviour("GuiCollection", asBEHAVE_ADDREF, "void f()",
            asMETHOD(GUI::GuiCollection, AddRefProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("GuiCollection", asBEHAVE_RELEASE, "void f()",
            asMETHOD(GUI::GuiCollection, ReleaseProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("GuiCollection", "string GetName()",
            asMETHOD(GUI::GuiCollection, GetNameProxy), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	// GuiManager needed to use some functionality, registered so that it cannot be stored //
	if(engine->RegisterObjectType("GuiManager", 0, asOBJ_REF | asOBJ_NOHANDLE) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("GuiManager",
            "bool SetCollectionState(const string &in name, bool state = false)",
            asMETHOD(GUI::GuiManager, SetCollectionState), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}


	// bind GuiObject //
	if(engine->RegisterObjectType("GuiObject", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	// no factory function to prevent scripts from creating these functions //

	if(engine->RegisterObjectBehaviour("GuiObject", asBEHAVE_ADDREF, "void f()",
            asMETHOD(GUI::BaseGuiObject, AddRefProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("GuiObject", asBEHAVE_RELEASE, "void f()",
            asMETHOD(GUI::BaseGuiObject, ReleaseProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("GuiObject", "int GetID()", asMETHOD(GUI::BaseGuiObject,
                GetID), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("GuiObject", "ScriptSafeVariableBlock@ GetAndPopFirstUpdated()", asMETHOD(
                GUI::BaseGuiObject, GetAndPopFirstUpdated), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("GuiObject", "GuiManager& GetOwningManager()", asMETHOD(GUI::BaseGuiObject,
                GetOwningManager), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("GuiObject", "string GetName()", asMETHOD(GUI::BaseGuiObject,
                GetName), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	
	if(engine->RegisterObjectMethod("GuiObject", "void PrintWindowsRecursive()", asMETHOD(GUI::BaseGuiObject,
                PrintWindowsRecursiveProxy), asCALL_THISCALL) < 0)
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

	if(engine->RegisterObjectMethod("Window", "void SetText(const string &in newtext)",
            asFUNCTION(CEGUIWindowSetTextProxy), asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("Window", "string GetText()", asFUNCTION(CEGUIWindowGetTextProxy),
            asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("Window", "Window& GetChildWindow(const string &in namepath)",
            asFUNCTION(CEGUIWindowGetChildWindowProxy), asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("Window", "void SetSize(float width, float widthpixels, float height, float heightpixels)", 
		asFUNCTION(CEGUIWindowSetSizeProxy), asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("Window", "void Invalidate(bool recursive = false)",
            asFUNCTION(CEGUIWindowInvalidateProxy), asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("Window", "bool SetSelectedTabIndex(int index)",
            asFUNCTION(CEGUITabControlSetActiveTabIndex), asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
    
	if(engine->RegisterObjectMethod("Window", "void SetDisabledState(bool disabled)",
            asFUNCTION(CEGUIWindowSetDisabledState), asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

    
    if(engine->RegisterObjectMethod("Window", "bool AddItem(const string &in text)", asFUNCTION(CEGUIComboboxAddItem),
            asCALL_CDECL_OBJFIRST) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Window", "bool ClearItems()", asFUNCTION(CEGUIComboboxClearItems),
            asCALL_CDECL_OBJFIRST) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Window", "bool SetSelectedItem(const string &in text)",
            asFUNCTION(CEGUIComboboxSetSelectedItem), asCALL_CDECL_OBJFIRST) < 0)
        {
            ANGELSCRIPT_REGISTERFAIL;
        }

	// Quite an expensive method //
	if(engine->RegisterObjectMethod("Window", 
            "bool LoadAndCustomizeTabFromFile(const string &in filename, const string &in tabname, const string &in lookfor, const string &in replacer)",
		asFUNCTION(CEGUIAdvancedCreateTabFromFile), asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}



	// Restore the namespace //
	if(engine->SetDefaultNamespace("") < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("GuiManager",
            "CEGUI::Window& GetWindowByName(const string &in namepath)",
            asMETHODPR(GUI::GuiManager, GetWindowByStringName, (const string&), CEGUI::Window*),
            asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
    
	if(engine->RegisterObjectMethod("GuiObject", "CEGUI::Window& GetTargetElement()", asMETHOD(GUI::BaseGuiObject, GetTargetWindow), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("GuiObject", "CEGUI::Window& GetTargetWindow()", asMETHOD(GUI::BaseGuiObject, GetTargetWindow), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	return true;
}


void RegisterGUIScriptTypeNames(asIScriptEngine* engine, std::map<int, string> &typeids){

	typeids.insert(make_pair(engine->GetTypeIdByDecl("GuiCollection"), "GuiCollection"));
	typeids.insert(make_pair(engine->GetTypeIdByDecl("GuiObject"), "GuiObject"));
}


