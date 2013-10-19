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

namespace Leviathan{ namespace Gui{
	// object types //
#define GUIOBJECTTYPE_TEXTLABEL		1000

	enum GUIOBJECT_LISTENERTYPE{GUIOBJECT_LISTENERTYPE_LISTENERVALUE};


	class GuiManager;
	// this class' functions are thread safe (*should* be) //// mutex for reference counting and possibly for other functions for thread safety //
	class BaseGuiObject : public Object, public ReferenceCounted, public AutoUpdateableObject, public Rocket::Core::EventListener{
		friend GuiManager;
	public:
		DLLEXPORT BaseGuiObject(GuiManager* owner, const wstring &name, int fakeid, shared_ptr<ScriptScript> script = NULL);
		DLLEXPORT virtual ~BaseGuiObject();

		REFERENCECOUNTED_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(BaseGuiObject);

		DLLEXPORT virtual int OnEvent(Event** pEvent);
		DLLEXPORT virtual bool OnUpdate(const shared_ptr<NamedVariableList> &updated);
		// rocket events //
		virtual void ProcessEvent(Rocket::Core::Event& receivedevent);
		virtual void OnDetach(Rocket::Core::Element* element);
		virtual void OnAttach(Rocket::Core::Element* element);

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
		void _HookRocketListeners();
		void _UnhookAllRocketListeners();
		void _CallScriptListener(GUIOBJECT_LISTENERTYPE type, Event** pEvent);
		// ------------------------------------ //

		int ID;
		int FileID;

		wstring Name;

		Rocket::Core::Element* Element;
		GuiManager* OwningInstance;
		shared_ptr<ScriptScript> Scripting;

		std::list<Rocket::Core::String> HookedRocketEvents;
	};

}}
#endif