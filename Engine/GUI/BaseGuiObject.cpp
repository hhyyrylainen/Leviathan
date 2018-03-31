#include "Include.h"
// ------------------------------------ //
#include "BaseGuiObject.h"

#include "../CEGUIInclude.h"
#include "../Handlers/IDFactory.h"
#include "Common/StringOperations.h"
#include "GUI/GuiManager.h"
#include "Iterators/StringIterator.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "Script/ScriptExecutor.h"
#include "Script/ScriptModule.h"
#include "Script/ScriptRunningSetup.h"
using namespace Leviathan;
using namespace GUI;
// ------------------------------------ //
DLLEXPORT BaseGuiObject::BaseGuiObject(GuiManager* owner, const std::string& name, int fakeid,
    std::shared_ptr<ScriptScript> script /*= NULL*/) :
    EventableScriptObject(script),
    ID(IDFactory::GetID()), FileID(fakeid), Name(name), OwningInstance(owner)
{
}

DLLEXPORT BaseGuiObject::~BaseGuiObject() {}

DLLEXPORT void BaseGuiObject::ReleaseData()
{

    // Unregister all non-CEGUI events //
    UnRegisterAllEvents();

    GUARD_LOCK();

    // Unregister events to avoid access violations //
    _UnsubscribeAllEvents(guard);

    _LeaveBondBridge();

    // Make sure that the module doesn't use us //
    if(Scripting) {

        auto module = Scripting->GetModuleSafe();

        if(module) {

            module->SetAsInvalid();

            auto directptr = module.get();
            Scripting.reset();
            module.reset();

            // The module should be destroyed sometime //
            ScriptExecutor::Get()->DeleteModule(directptr);
        }
    }
}
// ------------------------------------ //
DLLEXPORT std::string BaseGuiObject::GetName()
{

    return Name;
}

DLLEXPORT CEGUI::Window* BaseGuiObject::GetTargetWindow() const
{
    return TargetElement;
}
// ------------------------------------ //
DLLEXPORT bool BaseGuiObject::LoadFromFileStructure(GuiManager* owner,
    std::vector<BaseGuiObject*>& tempobjects, ObjectFileObject& dataforthis,
    const ExtraParameters& extra)
{
    // parse fake id from prefixes //
    int fakeid = 0;

    for(size_t i = 0; i < dataforthis.GetPrefixesCount(); i++) {

        auto str = dataforthis.GetPrefix(i);

        if(StringOperations::StringStartsWith(str, std::string("ID"))) {
            // get id number //
            StringIterator itr(str);

            auto tempnumber = itr.GetNextNumber<std::string>(DECIMALSEPARATORTYPE_NONE);

            fakeid = Convert::StringTo<int>(*tempnumber);

            // Nothing more to find //
            break;
        }
    }

    auto tmpptr = std::make_unique<BaseGuiObject>(
        owner, dataforthis.GetName(), fakeid, dataforthis.GetScript());

    // Apply extra flags //
    if(tmpptr->Scripting) {

        if(auto mod = tmpptr->Scripting->GetModule(); mod) {

            mod->AddAccessRight(extra.ExtraAccess);
        }
    }


    std::shared_ptr<NamedVariableList> listenon;

    // Get listeners //
    auto paramlist = dataforthis.GetListWithName("params");

    if(paramlist) {

        listenon = paramlist->GetVariables().GetValueDirect("ListenOn");
    }

    if(!tmpptr) {

        return false;
    }

    // Listening start //
    if(listenon) {
        tmpptr->StartMonitoring(listenon->GetValues());
    }

    // Find the CEGUI object //
    CEGUI::Window* foundobject = NULL;
    try {
        // Names starting with '_' are not considered to be targeting specific CEGUI windows //
        if(tmpptr->Name.find_first_of('_') != 0)
            foundobject = owner->GetMainContext()->getRootWindow()->getChild(tmpptr->Name);

    } catch(const CEGUI::UnknownObjectException& e) {

        // Not found //
        Logger::Get()->Error(
            "BaseGuiObject: couldn't find a CEGUI window with name: " + tmpptr->Name + ":");

        Logger::Get()->Write("\t> " + std::string(e.what()));
    }

    if(foundobject) {
        // Set the object //
        tmpptr->ConnectElement(foundobject);
    }

    tmpptr->_HookListeners();

    // Call the Init function //
    tmpptr->OnEvent(new Event(EVENT_TYPE_INIT, nullptr));

    tempobjects.push_back(tmpptr.release());
    return true;
}
// ------------------------------------ //
void BaseGuiObject::_HookListeners()
{
    // first we need to get probably right listeners and register with them //
    if(!Scripting)
        return;
    ScriptModule* mod = Scripting->GetModule();

    std::vector<std::shared_ptr<ValidListenerData>> containedlisteners;

    mod->GetListOfListeners(containedlisteners);

    // Allow the module to hot-reload our files //
    _BondWithModule(mod);

    RegisterStandardScriptEvents(containedlisteners);

    for(size_t i = 0; i < containedlisteners.size(); i++) {
        // Generics cannot be CEGUI events //
        if(containedlisteners[i]->GenericTypeName &&
            containedlisteners[i]->GenericTypeName->size() > 0) {
            // custom event listener //
            // Registered in RegisterStandardScriptEvents
            continue;
        }

        // Check is this a CEGUI event which will be registered //
        if(_HookCEGUIEvent(*containedlisteners[i]->ListenerName))
            continue;

        // Skip global events
        EVENT_TYPE etype = ResolveStringToType(*containedlisteners[i]->ListenerName);

        // Skip types that we handle ourselves //
        if(etype == EVENT_TYPE_ERROR)
            etype = GetCommonEventType(*containedlisteners[i]->ListenerName);

        if(etype != EVENT_TYPE_ERROR) {

            continue;
        }

        Logger::Get()->Warning("BaseGuiObject: _HookListeners: unknown event type " +
                               *containedlisteners[i]->ListenerName +
                               ", did you intent to use Generic type?");
    }
}
// ------------------------------------ //
DLLEXPORT bool BaseGuiObject::IsCEGUIEventHooked() const
{

    GUARD_LOCK();
    return !CEGUIRegisteredEvents.empty();
}
// ------------------------------------ //
ScriptRunResult<int> BaseGuiObject::_DoCallWithParams(
    ScriptRunningSetup& sargs, Event* event, GenericEvent* event2)
{
    if(event)
        return ScriptExecutor::Get()->RunScript<int>(
            Scripting->GetModuleSafe(), sargs, this, event);
    else
        return ScriptExecutor::Get()->RunScript<int>(
            Scripting->GetModuleSafe(), sargs, this, event2);
}
// ------------------------------------ //
DLLEXPORT std::unique_ptr<ScriptRunningSetup> BaseGuiObject::GetParametersForInit()
{
    return _GetArgsForAutoFunc();
}

DLLEXPORT std::unique_ptr<ScriptRunningSetup> BaseGuiObject::GetParametersForRelease()
{
    return _GetArgsForAutoFunc();
}

std::unique_ptr<ScriptRunningSetup> BaseGuiObject::_GetArgsForAutoFunc()
{
    // Setup the parameters //
    // This needs to be redone for the new style
    DEBUG_BREAK;
    // std::vector<std::shared_ptr<NamedVariableBlock>> Args = {
    //     std::make_shared<NamedVariableBlock>(new VoidPtrBlock(this), "GuiObject"),
    //     std::make_shared<NamedVariableBlock>(new VoidPtrBlock((Event*)nullptr), "Event")};

    // // we are returning ourselves so increase refcount
    // AddRef();

    std::unique_ptr<ScriptRunningSetup> sargs(new ScriptRunningSetup);
    // sargs->SetArguments(Args);

    return sargs;
}
// ------------------------------------ //
std::map<std::string, const CEGUI::String*> BaseGuiObject::CEGUIEventNames;

void BaseGuiObject::ReleaseCEGUIEventNames()
{

    Lock lockthis(CEGUIEventMutex);

    CEGUIEventNames.clear();
}

void BaseGuiObject::MakeSureCEGUIEventsAreFine(Lock& locked)
{
    // Return if it already has data //
    if(!CEGUIEventNames.empty())
        return;


    // Fill the map //
    CEGUIEventNames.insert(
        std::make_pair(LISTENERNAME_ONCLICK, &CEGUI::PushButton::EventClicked));
    CEGUIEventNames.insert(
        std::make_pair(LISTENERNAME_ONCLOSECLICKED, &CEGUI::FrameWindow::EventCloseClicked));
    CEGUIEventNames.insert(std::make_pair(
        LISTENERNAME_LISTSELECTIONACCEPTED, &CEGUI::Combobox::EventListSelectionAccepted));
}

Mutex BaseGuiObject::CEGUIEventMutex;
// ------------------------------------ //
DLLEXPORT void BaseGuiObject::PrintWindowsRecursive(
    Lock& guard, CEGUI::Window* target /*= NULL*/, size_t level /*= 0*/) const
{

    VerifyLock(guard);

    // Print the heading if this is the first level //
    if(level == 0)
        Logger::Get()->Write(
            "\n// ------------------ Layout of object \"" + Name + "\" ------------------ //");

    CEGUI::Window* actualtarget = target != NULL ? target : TargetElement;

    if(!actualtarget)
        return;

    // Print this window //
    for(size_t i = 0; i < level; i++) {

        Logger::Get()->DirectWriteBuffer("    ");
    }

    Logger::Get()->Write("    > " + std::string(actualtarget->getName().c_str()));

    // Recurse to child windows //
    for(size_t i = 0; i < actualtarget->getChildCount(); i++) {
        auto newtarget = actualtarget->getChildAtIdx(i);

        PrintWindowsRecursive(guard, newtarget, level + 1);
    }
}

DLLEXPORT void BaseGuiObject::PrintWindowsRecursiveProxy()
{
    GUARD_LOCK();
    PrintWindowsRecursive(guard);
}
// ------------------------------------ //
DLLEXPORT void BaseGuiObject::ConnectElement(CEGUI::Window* windojb)
{
    GUARD_LOCK();
    // Unconnect from previous ones //
    if(TargetElement) {

        DEBUG_BREAK;
    }

    // Store the pointer //
    TargetElement = windojb;

    // Register for the destruction event //
    auto unhookevent = TargetElement->subscribeEvent(CEGUI::Window::EventDestructionStarted,
        CEGUI::Event::Subscriber(&BaseGuiObject::EventDestroyWindow, this));

    // Apparently this also has to be disconnected //
    CEGUIRegisteredEvents.push_back(unhookevent);
}

bool BaseGuiObject::_HookCEGUIEvent(const std::string& listenername)
{

    {
        // This should be fine to be only locked during the next call as
        // it is the only place where the map could be updated
        Lock lockthis(CEGUIEventMutex);

        MakeSureCEGUIEventsAreFine(lockthis);
    }

    // Try to match the name //

    auto iter = CEGUIEventNames.find(listenername);

    if(iter == CEGUIEventNames.end())
        return false;

    // It is a valid event name //
    GUARD_LOCK();


    // Return if we have no element //
    if(!TargetElement) {

        // It is a CEGUI event so return true even though it isn't handled //
        return true;
    }

    // Switch on special cases where special handling can be used //
    CEGUI::Event::Connection createdconnection;


    if(iter->second == &CEGUI::PushButton::EventClicked) {
        createdconnection = TargetElement->subscribeEvent(
            *iter->second, CEGUI::Event::Subscriber(&BaseGuiObject::EventOnClick, this));

    } else if(iter->second == &CEGUI::FrameWindow::EventCloseClicked) {

        createdconnection = TargetElement->subscribeEvent(*iter->second,
            CEGUI::Event::Subscriber(&BaseGuiObject::EventOnCloseClicked, this));

    } else if(iter->second == &CEGUI::Combobox::EventListSelectionAccepted) {

        createdconnection = TargetElement->subscribeEvent(*iter->second,
            CEGUI::Event::Subscriber(&BaseGuiObject::EventOnListSelectionAccepted, this));

    } else {

        // Generic listeners aren't supported since we would have no way of knowing which
        // script listener would have to be called
        Logger::Get()->Error("BaseGuiObject: _HookCEGUIEvent: event is missing a specific "
                             "clause, name: " +
                             listenername);

        Logger::Get()->Save();
        LEVIATHAN_ASSERT(0, "Unsupported CEGUI listener type is being called");
        return true;
    }

    CEGUIRegisteredEvents.push_back(createdconnection);
    return true;
}

void BaseGuiObject::_UnsubscribeAllEvents(Lock& guard)
{

    // Loop an disconnect them all //
    for(auto iter = CEGUIRegisteredEvents.begin(); iter != CEGUIRegisteredEvents.end();
        ++iter) {
        (*iter)->disconnect();
    }

    CEGUIRegisteredEvents.clear();
}
// ------------------------------------ //
bool BaseGuiObject::_CallCEGUIListener(const std::string& name,
    std::function<void(std::vector<std::shared_ptr<NamedVariableBlock>>&)> extraparam
    /*= NULL*/)
{
    if(!Scripting) {
        return false;
    }

    // There should be one as it is registered //
    ScriptModule* mod = Scripting->GetModule();

    if(!mod) {
        Logger::Get()->Error("BaseGuiObject: CallCEGUIListener: the script module is "
                             "no longer valid");
        return false;
    }

    // A calling function might want to add extra parameters //
    if(extraparam) {

        // TODO: similarly to ScriptModule args bridge this needs redoing
        DEBUG_BREAK;
        // extraparam(Args);
    }

    ScriptRunningSetup sargs(mod->GetListeningFunctionName(name));

    // Run the script //
    auto result =
        ScriptExecutor::Get()->RunScript<bool>(Scripting->GetModuleSafe(), sargs, this);

    if(result.Result != SCRIPT_RUN_RESULT::Success) {

        return false;
    }

    // Return the value returned by the script //
    return result.Value;
}
// ------------------------------------ //
bool BaseGuiObject::EventDestroyWindow(const CEGUI::EventArgs& args)
{
    GUARD_LOCK();

    // This should be safe //
    auto res = static_cast<const CEGUI::WindowEventArgs&>(args);

    LEVIATHAN_ASSERT(res.window == TargetElement,
        "BaseGuiObject received destruction notification for unsubscribed window");

    // This might be required to not leak memory //
    _UnsubscribeAllEvents(guard);

    TargetElement = nullptr;

    return false;
}

bool BaseGuiObject::EventOnClick(const CEGUI::EventArgs& args)
{
    // Pass the click event to the script //

    return _CallCEGUIListener(LISTENERNAME_ONCLICK);
}

bool BaseGuiObject::EventOnCloseClicked(const CEGUI::EventArgs& args)
{
    // Pass the event to the script //

    return _CallCEGUIListener(LISTENERNAME_ONCLOSECLICKED);
}

bool BaseGuiObject::EventOnListSelectionAccepted(const CEGUI::EventArgs& args)
{

    auto res = static_cast<const CEGUI::WindowEventArgs&>(args);

    auto targetwind = dynamic_cast<CEGUI::Combobox*>(res.window);

    if(!targetwind) {

        Logger::Get()->Error("BaseGuiObject: CEGUI listener for ListSelectionAccepted "
                             "didn't get a Combobox as argument");
        return false;
    }

    auto item = targetwind->getSelectedItem();

    if(!item) {

        return false;
    }

    std::string selectedtext(item->getText().c_str());

    return _CallCEGUIListener(LISTENERNAME_LISTSELECTIONACCEPTED,
        std::bind<void>(
            [](const std::string& selecttext,
                std::vector<std::shared_ptr<NamedVariableBlock>>& paramlist) -> void {

                paramlist.push_back(
                    std::make_shared<NamedVariableBlock>(new StringBlock(selecttext), "Text"));

            },
            selectedtext, std::placeholders::_1));
}
