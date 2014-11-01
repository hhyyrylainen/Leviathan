#ifndef LEVIATHAN_GUI_BASEOBJECT
#define LEVIATHAN_GUI_BASEOBJECT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Script/ScriptScript.h"
#include "ObjectFiles/ObjectFileObject.h"
#include "Common/ReferenceCounted.h"
#include "Events/Event.h"
#include "GuiCollection.h"
#include "Events/EventableScriptObject.h"
#include "boost/thread/mutex.hpp"
#include "Script/ScriptArgumentsProvider.h"

namespace Leviathan{ namespace Gui{


        //! \brief Represents a single GUI element that can use scripts to react to events
        //! \todo Add a script module destruction queue
        class BaseGuiObject : public ReferenceCounted, public EventableScriptObject,
                                public ScriptArgumentsProvider, public ThreadSafe{
            friend GuiManager;
        public:
            DLLEXPORT BaseGuiObject(GuiManager* owner, const wstring &name, int fakeid,
                shared_ptr<ScriptScript> script = NULL);
            DLLEXPORT virtual ~BaseGuiObject();

            REFERENCECOUNTED_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(BaseGuiObject);

            DLLEXPORT ScriptSafeVariableBlock* GetAndPopFirstUpdated(){
                if(UpdatedValues.empty())
                    return NULL;

                auto tmp = new ScriptSafeVariableBlock(UpdatedValues[0]->GetValueDirect(), UpdatedValues[0]->GetName());
                UpdatedValues.erase(UpdatedValues.begin());

                return tmp;
            }

            DLLEXPORT inline int GetID(){
                return ID;
            }


            DLLEXPORT GuiManager* GetOwningManager(){
                return OwningInstance;
            }
		
            DLLEXPORT static bool LoadFromFileStructure(GuiManager* owner, vector<BaseGuiObject*>
                &tempobjects,	ObjectFileObject& dataforthis);


            //! \brief Sets this objects target CEGUI widget
            //!
            //! This will also register the widget for unconnect events to not use deleted pointers
            DLLEXPORT void ConnectElement(CEGUI::Window* windojb);

            //! \brief Gets the name of this object as a string
            DLLEXPORT string GetNameAsString();


            //! \brief Releases the data held onto by this object
            DLLEXPORT void ReleaseData();

            //! \brief Returns the TargetElement CEGUI window which might be NULL
            DLLEXPORT CEGUI::Window* GetTargetWindow() const;


            //! \brief Prints the window layout starting from TargetElement
            //! \param target The target window or NULL if TargetElement should be used
            //! \note Passing only the guard will work if you want to start from the target element
            DLLEXPORT void PrintWindowsRecursive(ObjectLock &guard, CEGUI::Window* target = NULL,
                size_t level = 0) const;

            //! \brief No parameters version of PrintWindowsRecursive
            DLLEXPORT FORCE_INLINE void PrintWindowsRecursive() const{
                GUARD_LOCK_THIS_OBJECT();
                PrintWindowsRecursive(guard);
            }

            //! \brief Proxy for PrintWindowsRecursive
            DLLEXPORT void PrintWindowsRecursiveProxy();
		
            DLLEXPORT virtual unique_ptr<ScriptRunningSetup> GetParametersForInit();

            DLLEXPORT virtual unique_ptr<ScriptRunningSetup> GetParametersForRelease();


        protected:

            // this function will try to hook all wanted listeners to CEGUI elements //
            void _HookListeners();
            virtual void _CallScriptListener(Event** pEvent, GenericEvent** event2);

            //! \brief Registers for an event if it is a CEGUI event
            bool _HookCEGUIEvent(const wstring &listenername);


            //! \brief Clears CEGUIRegisteredEvents and unsubscribes from all
            void _UnsubscribeAllEvents();


            //! \brief Calls the script for a specific CEGUI event listener
            //! \return The scripts return value changed to an int
            bool _CallCEGUIListener(const wstring &name);

            unique_ptr<ScriptRunningSetup> _GetArgsForAutoFunc();


            // ------------------------------------ //


            int ID;
            int FileID;

            wstring Name;

            GuiManager* OwningInstance;

            //! The element that this script wrapper targets
            CEGUI::Window* TargetElement;


            //! List of registered CEGUI events. This is used for unsubscribing
            std::vector<CEGUI::Event::Connection> CEGUIRegisteredEvents;

            // ------------------------------------ //
            //! This map collects all the available CEGUI events which can be hooked into
            static std::map<wstring, const CEGUI::String*> CEGUIEventNames;

        public:

            //! \brief Frees CEGUIEventNames
            //!
            //! Only call this right before the Engine shuts down
            static void ReleaseCEGUIEventNames();

            //! \brief Constructs CEGUIEventNames
            //!
            //! This is safe to call at any time since the map is only filled once
            static void MakeSureCEGUIEventsAreFine(boost::strict_lock<boost::mutex> &locked);


            //! The mutex required for MakeSureCEGUIEventsAreFine
            static boost::mutex CEGUIEventMutex;


        protected:


            bool EventDestroyWindow(const CEGUI::EventArgs &args);

            bool EventOnClick(const CEGUI::EventArgs &args);

            bool EventOnCloseClicked(const CEGUI::EventArgs &args);
        };

}}
#endif
