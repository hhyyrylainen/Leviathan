// ------------------------------------ //
#include "GuiScriptBind.h"

#include "GUI/BaseGuiObject.h"
#include "GUI/GuiManager.h"

#include "CEGUI/widgets/ProgressBar.h"
#include "CEGUIInclude.h"

#include "FileSystem.h"
#include "add_on/autowrapper/aswrappedcall.h"

#include "GUI/Widgets/CEGUIVideoPlayer.h"

#ifdef _WIN32
#include "WindowsInclude.h"
#endif // _WIN32

using namespace Leviathan;
// ------------------------------------ //

// Proxies etc.
// ------------------------------------ //
void CEGUIWindowSetPropertyProxy(
    CEGUI::Window* obj, const std::string& propertyname, const std::string& propertyvalue)
{
    obj->setProperty(propertyname, propertyvalue);
}

void CEGUIWindowSetTextProxy(CEGUI::Window* obj, const std::string& text)
{
    obj->setText(text);
}

std::string CEGUIWindowGetTextProxy(CEGUI::Window* obj, const std::string& text)
{
    return std::string(obj->getText().c_str());
}

CEGUI::Window* CEGUIWindowGetChildWindowProxy(CEGUI::Window* obj, const std::string& text)
{
    return obj->getChild(text);
}

// TODO: version with UDim and USize types
void CEGUIWindowSetSizeProxy(
    CEGUI::Window* obj, float width, float widthpixels, float height, float heightpixels)
{
    obj->setSize(
        CEGUI::USize(CEGUI::UDim(width, widthpixels), CEGUI::UDim(height, heightpixels)));
}

void CEGUIWindowInvalidateProxy(CEGUI::Window* obj, bool recursive)
{
    obj->invalidate(recursive);
}

void CEGUIWindowSetDisabledState(CEGUI::Window* obj, bool disabled)
{
    obj->setEnabled(!disabled);
}

bool CEGUIComboboxSetSelectedItem(CEGUI::Combobox* obj, const std::string& text)
{
    CEGUI::StandardItem* wanted = obj->findItemWithText(CEGUI::String(text), NULL);

    if(!wanted)
        return false;

    // Skip setting it as selected if it already is //
    if(wanted == obj->getSelectedItem())
        return true;

    obj->clearAllSelections();
    obj->setItemSelectState(wanted, true);

    return true;
}

void CEGUIComboboxAddItem(CEGUI::Combobox* obj, const std::string& text)
{
    obj->addItem(new CEGUI::StandardItem(CEGUI::String(text)));
}

bool CEGUIAdvancedCreateTabFromFile(CEGUI::Window* obj, const std::string& filename,
    const std::string& tabname, const std::string& lookfor, const std::string& replacer)
{
    // Find the file //
    const std::string onlyname =
        StringOperations::RemoveExtension<std::string>(filename, true);

    const std::string& targetfile = FileSystem::Get()->SearchForFile(FILEGROUP_SCRIPT,
        onlyname, StringOperations::GetExtension<std::string>(filename), true);

    if(targetfile.empty()) {

        // The file wasn't found //
        return false;
    }


    CEGUI::TabControl* convtabs = dynamic_cast<CEGUI::TabControl*>(obj);
    if(convtabs == NULL) {

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
    convtabs
        ->getChild(CEGUI::String("__auto_TabPane__Buttons/__auto_btn") + newwindow->getName())
        ->setText(tabname);

    // Succeeded //
    return true;
}

CEGUI::Window* CEGUIWindowCreateAndAddChild(
    CEGUI::Window* parent, const std::string& widgettype, const std::string& widgetname)
{
    if(!parent)
        return nullptr;

    CEGUI::Window* newWindow = CEGUI::WindowManager::getSingleton().createWindow(
        widgettype.c_str(), widgetname.c_str());

    if(!newWindow) {

        LOG_ERROR("CreateAndAddChild: failed to create widget of type: " + widgettype);
        return nullptr;
    }

    parent->addChild(newWindow);

    return newWindow;
}

void CEGUIWindowDestroyChild(CEGUI::Window* parent, const std::string& widgetname)
{
    if(!parent)
        return;

    parent->destroyChild(widgetname);
}

// ------------------------------------ //
// Start of the actual bind
namespace Leviathan {

bool BindGuiCollection(asIScriptEngine* engine)
{
    ANGELSCRIPT_REGISTER_REF_TYPE("GuiCollection", GUI::GuiCollection);

    if(engine->RegisterObjectMethod("GuiCollection", "const string& GetName()",
           asMETHOD(GUI::GuiCollection, GetName), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }


    return true;
}

bool BindGuiObject(asIScriptEngine* engine)
{
    ANGELSCRIPT_REGISTER_REF_TYPE("GuiObject", GUI::BaseGuiObject);

    if(engine->RegisterObjectMethod("GuiObject", "int GetID()",
           asMETHOD(GUI::BaseGuiObject, GetID), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("GuiObject",
           "void ConnectElement(CEGUI::Window@ windowobj)",
           asMETHOD(GUI::BaseGuiObject, ConnectElement), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("GuiObject", "CEGUI::Window@ GetTargetWindow() const",
           asMETHOD(GUI::BaseGuiObject, GetTargetWindow), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("GuiObject", "bool IsCEGUIEventHooked()",
           asMETHOD(GUI::BaseGuiObject, IsCEGUIEventHooked), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("GuiObject",
           "ScriptSafeVariableBlock@ GetAndPopFirstUpdated()",
           asMETHOD(GUI::BaseGuiObject, GetAndPopFirstUpdated), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("GuiObject", "string GetName()",
           asMETHOD(GUI::BaseGuiObject, GetName), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("GuiObject", "void PrintWindowsRecursive() const",
           asMETHOD(GUI::BaseGuiObject, PrintWindowsRecursiveProxy), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }


    return true;
}

//! Binds everything in the CEGUI namespace
bool BindCEGUI(asIScriptEngine* engine)
{
    if(engine->SetDefaultNamespace("CEGUI") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectType("Window", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Window",
           "void SetProperty(const string &in propertyname, const string &in propertyvalue)",
           asFUNCTION(CEGUIWindowSetPropertyProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Window", "void SetText(const string &in newtext)",
           asFUNCTION(CEGUIWindowSetTextProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Window", "string GetText()",
           asFUNCTION(CEGUIWindowGetTextProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Window",
           "Window@ GetChildWindow(const string &in namepath)",
           asFUNCTION(CEGUIWindowGetChildWindowProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Alias for GetChildWindow
    if(engine->RegisterObjectMethod("Window", "Window@ GetChild(const string &in namepath)",
           asFUNCTION(CEGUIWindowGetChildWindowProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Window",
           "Window@ CreateAndAddChild(const string &in widgettype, "
           "const string &in name = \"\")",
           asFUNCTION(CEGUIWindowCreateAndAddChild), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Window", "void DestroyChild(const string &in name)",
           asFUNCTION(CEGUIWindowDestroyChild), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Window",
           "void SetSize(float width, float widthpixels, float height, float heightpixels)",
           asFUNCTION(CEGUIWindowSetSizeProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Window", "void Invalidate(bool recursive = false)",
           asFUNCTION(CEGUIWindowInvalidateProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Window", "void SetDisabledState(bool disabled)",
           asFUNCTION(CEGUIWindowSetDisabledState), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------------------------ //
    // Tabcontrol
    if(engine->RegisterObjectType("TabControl", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    ANGLESCRIPT_BASE_CLASS_CASTS_NO_REF(
        CEGUI::Window, "Window", CEGUI::TabControl, "TabControl");

    ANGELSCRIPT_ASSUMED_SIZE_T;
    if(engine->RegisterObjectMethod("TabControl", "void SetSelectedTabIndex(uint64 index)",
           asMETHOD(CEGUI::TabControl, setSelectedTabAtIndex), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------------------------ //
    // Combobox stuff
    if(engine->RegisterObjectType("Combobox", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    ANGLESCRIPT_BASE_CLASS_CASTS_NO_REF(CEGUI::Window, "Window", CEGUI::Combobox, "Combobox");

    if(engine->RegisterObjectMethod("Combobox", "bool AddItem(const string &in text)",
           asFUNCTION(CEGUIComboboxAddItem), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Combobox", "void ClearItems()",
           asMETHOD(CEGUI::Combobox, resetList), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Combobox", "void SetSelectedItem(const string &in text)",
           asFUNCTION(CEGUIComboboxSetSelectedItem), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------------------------ //
    // ProgressBar
    if(engine->RegisterObjectType("ProgressBar", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    ANGLESCRIPT_BASE_CLASS_CASTS_NO_REF(
        CEGUI::Window, "Window", CEGUI::ProgressBar, "ProgressBar");

    if(engine->RegisterObjectMethod("ProgressBar", "void SetProgress(float progress)",
           asMETHOD(CEGUI::ProgressBar, setProgress), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------------------------ //
    // Quite an expensive method //
    if(engine->RegisterObjectMethod("Window",
           "bool LoadAndCustomizeTabFromFile(const string &in filename, "
           "const string &in tabname, const string &in lookfor, const string &in replacer)",
           asFUNCTION(CEGUIAdvancedCreateTabFromFile), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }


    // Restore the namespace //
    if(engine->SetDefaultNamespace("") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

//! Binds specialised widget types
bool BindWidgetTypes(asIScriptEngine* engine)
{

    if(engine->RegisterObjectType("CEGUIVideoPlayer", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    ANGLESCRIPT_BASE_CLASS_CASTS_NO_REF(
        CEGUI::Window, "CEGUI::Window", GUI::CEGUIVideoPlayer, "CEGUIVideoPlayer");


    if(engine->RegisterObjectMethod("CEGUIVideoPlayer",
           "bool Play(const string &in videofile)", asMETHOD(GUI::CEGUIVideoPlayer, Play),
           asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("CEGUIVideoPlayer", "void Stop()",
           asMETHOD(GUI::CEGUIVideoPlayer, Stop), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("CEGUIVideoPlayer", "float GetCurrentTime() const",
           asMETHOD(GUI::CEGUIVideoPlayer, GetCurrentTime), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("CEGUIVideoPlayer", "Delegate@ get_OnPlaybackEnded()",
           asMETHOD(GUI::CEGUIVideoPlayer, GetOnPlaybackEnded), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

} // namespace Leviathan

bool Leviathan::BindGUI(asIScriptEngine* engine)
{
    // Needed by base gui object and widget types
    if(!BindCEGUI(engine))
        return false;

    // bind GuiCollection action, this is released by gui object //
    if(!BindGuiCollection(engine))
        return false;

    if(!BindGuiObject(engine))
        return false;

    if(!BindWidgetTypes(engine))
        return false;

    // GuiManager needed to use some functionality, registered so that it cannot be stored //
    if(engine->RegisterObjectType("GuiManager", 0, asOBJ_REF | asOBJ_NOHANDLE) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("GuiManager",
           "bool SetCollectionState(const string &in name, bool state = false)",
           asMETHOD(GUI::GuiManager, SetCollectionState), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("GuiObject", "GuiManager& GetOwningManager()",
           asMETHOD(GUI::BaseGuiObject, GetOwningManager), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("GuiManager",
           "CEGUI::Window@ GetWindowByName(const string &in namepath)",
           asMETHODPR(
               GUI::GuiManager, GetWindowByStringName, (const std::string&), CEGUI::Window*),
           asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("GuiManager", "CEGUI::Window@ GetRootWindow()",
           asMETHOD(GUI::GuiManager, GetRootWindow), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}
