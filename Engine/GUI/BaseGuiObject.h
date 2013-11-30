#ifndef LEVIATHAN_GUI_BASEOBJECT
#define LEVIATHAN_GUI_BASEOBJECT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Script\ScriptScript.h"
#include "ObjectFiles\ObjectFileObject.h"
#include "Common\ReferenceCounted.h"
#include "Events\Event.h"
#include "GuiCollection.h"
#include "Events\EventableScriptObject.h"

namespace Leviathan{ namespace Gui{


	class GuiManager;
	// this class' functions are thread safe (*should* be) //// mutex for reference counting and possibly for other functions for thread safety //
	// TODO: make the above statement true...
	class BaseGuiObject : public ReferenceCounted, public EventableScriptObject, public Rocket::Core::EventListener{
		friend GuiManager;
	public:
		DLLEXPORT BaseGuiObject(GuiManager* owner, const wstring &name, int fakeid, GuiLoadedSheet* sheet, shared_ptr<ScriptScript> script = NULL);
		DLLEXPORT virtual ~BaseGuiObject();

		REFERENCECOUNTED_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(BaseGuiObject);
		//AUTOUPDATEABLEOBJECT_SCRIPTPROXIES;

		DLLEXPORT ScriptSafeVariableBlock* GetAndPopFirstUpdated(){

			auto tmp = new ScriptSafeVariableBlock(UpdatedValues[0]->GetValueDirect(), UpdatedValues[0]->GetName());
			UpdatedValues.erase(UpdatedValues.begin());

			return tmp;
		}


		// rocket events //
		virtual void ProcessEvent(Rocket::Core::Event& receivedevent);
		virtual void OnDetach(Rocket::Core::Element* element);
		virtual void OnAttach(Rocket::Core::Element* element);

		// should be called by GuiManager when objects in sheets have been updated //
		DLLEXPORT bool CheckObjectLinkage();

		DLLEXPORT inline int GetID(){
			return ID;
		}

		DLLEXPORT bool SetInternalRMLWrapper(string rmlcode);



		DLLEXPORT GuiManager* GetOwningManager(){
			return OwningInstance;
		}

		// Warning this function increases the reference count! //
		DLLEXPORT GuiLoadedSheet* GetOwningSheetProxy(){
			ContainedInSheet->AddRef();
			return ContainedInSheet;
		}
		DLLEXPORT GuiLoadedSheet* GetOwningSheet(){
			return ContainedInSheet;
		}


		static std::map<wstring, Rocket::Core::String> LeviathanToRocketEventTranslate;
		static std::map<Rocket::Core::String, wstring> RocketEventToLeviathanListenerTranslate;

		// Call this function before using the above //
		DLLEXPORT static void VerifyInversedEventTranslateMap();

		DLLEXPORT static bool LoadFromFileStructure(GuiManager* owner, vector<BaseGuiObject*> &tempobjects,	ObjectFileObject& dataforthis,
			GuiLoadedSheet* sheet);
	protected:
		// this function will try to hook all wanted listeners to Rocket element //
		void _HookListeners(bool onlyrocket = false);
		void _UnhookAllListeners();
		virtual void _CallScriptListener(Event** pEvent, GenericEvent** event2);
		// ------------------------------------ //

		int ID;
		int FileID;

		bool ManualDetach;

		wstring Name;
		wstring RocketObjectName;

		Rocket::Core::Element* Element;
		GuiManager* OwningInstance;
		// Stores the owning sheet for easy access //
		GuiLoadedSheet* ContainedInSheet;

		std::list<Rocket::Core::String> HookedRocketEvents;
	};

}}
#endif