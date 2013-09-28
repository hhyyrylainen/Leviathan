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

	enum GUIOBJECTHAS {
		GUIOBJECTHAS_BASE = 0x1, 
		GUIOBJECTHAS_RENDERABLE = 0x2, 
		GUIOBJECTHAS_EVENTABLE = 0x4, 
		GUIOBJECTHAS_ANIMATEABLE = 0x8,
		GUIOBJECTHAS_POSITIONABLE = 0x10, 
		GUIOBJECTHAS_BASEGRAPHICAL = 0x20
		//0x40
		//0x80 // first byte full
		//0x100
		//0x200
		//0x400
		//0x800
		//0x1000
		//0x2000
		//0x4000
		//0x8000 // second byte full (int range might stop here(
		//0x10000
		//0x20000
		//0x40000
		//0x80000
		//0x100000
		//0x200000
		//0x400000
		//0x800000 // third byte full
		//0x1000000
		//0x2000000
		//0x4000000
		//0x8000000
		//0x10000000
		//0x20000000
		//0x40000000
		//0x80000000 // fourth byte full (will need QWORD here)
		//0x100000000
	};

	class GuiManager;
	// this class' functions are thread safe (*should* be) //// mutex for reference counting and possibly for other functions for thread safety //
	class BaseGuiObject : public Object, public ReferenceCounted, public AutoUpdateableObject, public Rocket::Core::EventListener{
	public:
		DLLEXPORT BaseGuiObject(GuiManager* owner, const wstring &name, int flags, int fakeid, shared_ptr<ScriptScript> script = NULL);
		DLLEXPORT virtual ~BaseGuiObject();

		REFERENCECOUNTED_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(BaseGuiObject);

		DLLEXPORT virtual int OnEvent(Event** pEvent);
		DLLEXPORT virtual bool OnUpdate(const shared_ptr<NamedVariableList> &updated);
		// rocket events //
		virtual void ProcessEvent(Rocket::Core::Event& event);
		virtual void OnDetach(Rocket::Core::Element* element);

		// this should be const, but it isn't because I like assignment operators //
		/*const*/ int ObjectFlags;
		
		DLLEXPORT inline int GetID(){
			return ID;
		}

		DLLEXPORT bool SetInternalRMLWrapper(string rmlcode){

			return false;
		}

		DLLEXPORT VariableBlock* GetAndPopFirstUpdated(){

			return UpdatedValues[0]->GetValueDirect();
		}

		static std::map<wstring, Rocket::Core::String> LeviathanToRocketEventTranslate;

		DLLEXPORT static bool LoadFromFileStructure(GuiManager* owner, vector<BaseGuiObject*> &tempobjects,	ObjectFileObject& dataforthis,
			shared_ptr<GuiLoadedSheet> sheet);
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