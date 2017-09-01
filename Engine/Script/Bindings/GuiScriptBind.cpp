// ------------------------------------ //
#include "GuiScriptBind.h"

#include "GUI/BaseGuiObject.h"
#include "GUI/GuiManager.h"

#include "add_on/autowrapper/aswrappedcall.h"
#include "CEGUIInclude.h"
#include "FileSystem.h"

using namespace Leviathan;
// ------------------------------------ //

// Proxies etc.
// ------------------------------------ //
void CEGUIWindowSetTextProxy(CEGUI::Window* obj, const std::string &text){
    
    obj->setText(text);
}

std::string CEGUIWindowGetTextProxy(CEGUI::Window* obj, const std::string &text){

    return std::string(obj->getText().c_str());
}

CEGUI::Window* CEGUIWindowGetChildWindowProxy(CEGUI::Window* obj, const std::string &text){

    return obj->getChild(text);
}

// TODO: version with UDim and USize types
void CEGUIWindowSetSizeProxy(CEGUI::Window* obj, float width, float widthpixels, float height,
    float heightpixels)
{

    obj->setSize(CEGUI::USize(CEGUI::UDim(width, widthpixels),
            CEGUI::UDim(height, heightpixels)));
}

void CEGUIWindowInvalidateProxy(CEGUI::Window* obj, bool recursive){

    obj->invalidate(recursive);
}

void CEGUIWindowSetDisabledState(CEGUI::Window* obj, bool disabled){

    obj->setEnabled(!disabled);
}

// TODO: all these methods but allow converting to the derived classes and then call these
bool CEGUITabControlSetActiveTabIndex(CEGUI::Window* obj, int index){

    CEGUI::TabControl* convtabs = dynamic_cast<CEGUI::TabControl*>(obj);
    if(convtabs != NULL){
        
        convtabs->setSelectedTabAtIndex(index);
        return true;
    }

    return false;
}

bool CEGUIComboboxSetSelectedItem(CEGUI::Window* obj, const std::string &text){

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

bool CEGUIComboboxAddItem(CEGUI::Window* obj, const std::string &text){

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

bool CEGUIAdvancedCreateTabFromFile(CEGUI::Window* obj, const std::string &filename,
    const std::string &tabname,
    const std::string &lookfor, const std::string &replacer)
{

    // Find the file //
    const std::string onlyname = StringOperations::RemoveExtensionString(filename, true);

    const std::string& targetfile = FileSystem::Get()->SearchForFile(FILEGROUP_SCRIPT,
        onlyname, StringOperations::GetExtensionString(filename), true);

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
    std::string filecontents;

    FileSystem::ReadFileEntirely(targetfile, filecontents);

    if(filecontents.empty())
        return false;

    // Replace the thing //
    filecontents = StringOperations::Replace<std::string>(filecontents, lookfor, replacer);

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

// ------------------------------------ //
// Start of the actual bind
namespace Leviathan{

bool BindGuiCollection(asIScriptEngine* engine){

    ANGELSCRIPT_REGISTER_REF_TYPE("GuiCollection", GUI::GuiCollection);

    if(engine->RegisterObjectMethod("GuiCollection", "const string& GetName()",
            asMETHOD(GUI::GuiCollection, GetName), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    
    return true;
}

bool BindGuiObject(asIScriptEngine* engine){

    ANGELSCRIPT_REGISTER_REF_TYPE("GuiObject", GUI::BaseGuiObject);

    if(engine->RegisterObjectMethod("GuiObject", "int GetID()", asMETHOD(GUI::BaseGuiObject,
                GetID), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("GuiObject",
            "ScriptSafeVariableBlock@ GetAndPopFirstUpdated()", asMETHOD(
                GUI::BaseGuiObject, GetAndPopFirstUpdated), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("GuiObject", "string GetName()",
            asMETHOD(GUI::BaseGuiObject, GetName), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }
    
    if(engine->RegisterObjectMethod("GuiObject", "void PrintWindowsRecursive() const",
            asMETHOD(GUI::BaseGuiObject, PrintWindowsRecursiveProxy), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }
    
    
    return true;
}

//! Binds everything in the CEGUI namespace
bool BindCEGUI(asIScriptEngine* engine){

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

    if(engine->RegisterObjectMethod("Window", "string GetText()",
            asFUNCTION(CEGUIWindowGetTextProxy),
            asCALL_CDECL_OBJFIRST) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Window",
            "Window& GetChildWindow(const string &in namepath)",
            asFUNCTION(CEGUIWindowGetChildWindowProxy), asCALL_CDECL_OBJFIRST) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Window",
            "void SetSize(float width, float widthpixels, float height, float heightpixels)", 
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

    
    if(engine->RegisterObjectMethod("Window", "bool AddItem(const string &in text)",
            asFUNCTION(CEGUIComboboxAddItem),
            asCALL_CDECL_OBJFIRST) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Window", "bool ClearItems()",
            asFUNCTION(CEGUIComboboxClearItems),
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
            "bool LoadAndCustomizeTabFromFile(const string &in filename, "
            "const string &in tabname, const string &in lookfor, const string &in replacer)",
            asFUNCTION(CEGUIAdvancedCreateTabFromFile), asCALL_CDECL_OBJFIRST) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }


    // Restore the namespace //
    if(engine->SetDefaultNamespace("") < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

}

bool Leviathan::BindGUI(asIScriptEngine* engine){

    // bind GuiCollection action, this is released by gui object //
    if(!BindGuiCollection(engine))
        return false;

    if(!BindGuiObject(engine))
        return false;

    if(!BindCEGUI(engine))
        return false;

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

    if(engine->RegisterObjectMethod("GuiObject", "GuiManager& GetOwningManager()",
            asMETHOD(GUI::BaseGuiObject,
                GetOwningManager), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("GuiManager",
            "CEGUI::Window& GetWindowByName(const string &in namepath)",
            asMETHODPR(GUI::GuiManager, GetWindowByStringName, (const std::string&),
                CEGUI::Window*),
            asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }
    
    if(engine->RegisterObjectMethod("GuiObject", "CEGUI::Window& GetTargetWindow() const",
            asMETHOD(GUI::BaseGuiObject, GetTargetWindow), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;    
}

void Leviathan::RegisterGUI(asIScriptEngine* engine, std::map<int, std::string> &typeids){

    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("GuiCollection"), "GuiCollection"));
    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("GuiObject"), "GuiObject"));
    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("CEGUI::Window"), "CEGUI::Window")); 
}


