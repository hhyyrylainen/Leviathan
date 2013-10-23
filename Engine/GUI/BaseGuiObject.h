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
#include "Events\AutoUpdateable.h"
#include "Events\CallableObject.h"

namespace Leviathan{ namespace Gui{


	class GuiManager;
	// this class' functions are thread safe (*should* be) //// mutex for reference counting and possibly for other functions for thread safety //
	// TODO: make the above statement true...
	class BaseGuiObject : public ReferenceCounted, public AutoUpdateableObject, public Rocket::Core::EventListener, public CallableObject{
		friend GuiManager;
	public:
		DLLEXPORT BaseGuiObject(GuiManager* owner, const wstring &name, int fakeid, int sheetid, shared_ptr<ScriptScript> script = NULL);
		DLLEXPORT virtual ~BaseGuiObject();

		REFERENCECOUNTED_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(BaseGuiObject);

		DLLEXPORT virtual int OnEvent(Event** pEvent);
		DLLEXPORT virtual int OnGenericEvent(GenericEvent** pevent);
		DLLEXPORT virtual bool OnUpdate(const shared_ptr<NamedVariableList> &updated);
		// rocket events //
		virtual void ProcessEvent(Rocket::Core::Event& receivedevent);
		virtual void OnDetach(Rocket::Core::Element* element);
		virtual void OnAttach(Rocket::Core::Element* element);

		// should be called by GuiManager when objects in sheets have been updated //
		DLLEXPORT bool CheckObjectLinkage();

		DLLEXPORT inline int GetSheetID(){
			return SheetID;
		}
		DLLEXPORT inline int GetID(){
			return ID;
		}

		DLLEXPORT bool SetInternalRMLWrapper(string rmlcode);

		DLLEXPORT ScriptSafeVariableBlock* GetAndPopFirstUpdated(){

			auto tmp = new ScriptSafeVariableBlock(UpdatedValues[0]->GetValueDirect(), UpdatedValues[0]->GetName());
			UpdatedValues.erase(UpdatedValues.begin());

			return tmp;
		}

		DLLEXPORT GuiManager* GetOwningManager(){
			return OwningInstance;
		}

		static std::map<wstring, Rocket::Core::String> LeviathanToRocketEventTranslate;

		DLLEXPORT static bool LoadFromFileStructure(GuiManager* owner, vector<BaseGuiObject*> &tempobjects,	ObjectFileObject& dataforthis,
			GuiLoadedSheet* sheet);
	protected:
		// this function will try to hook all wanted listeners to Rocket element //
		void _HookListeners(bool onlyrocket = false);
		void _UnhookAllListeners();
		void _CallScriptListener(Event** pEvent, GenericEvent** event2);
		// ------------------------------------ //

		int ID;
		int FileID;
		// stores the sheet ID for fetching it again //
		int SheetID;

		bool ManualDetach;

		wstring Name;
		wstring RocketObjectName;

		Rocket::Core::Element* Element;
		GuiManager* OwningInstance;
		shared_ptr<ScriptScript> Scripting;

		std::list<Rocket::Core::String> HookedRocketEvents;
	};

}}
#endif