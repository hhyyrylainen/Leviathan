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

namespace Leviathan{ namespace Gui{


	class GuiManager;
	// this class' functions are thread safe (*should* be) //// mutex for reference counting and possibly for other functions for thread safety //
	// \todo make the above statement true...
	class BaseGuiObject : public ReferenceCounted, public EventableScriptObject{
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

		DLLEXPORT inline int GetID(){
			return ID;
		}


		DLLEXPORT GuiManager* GetOwningManager(){
			return OwningInstance;
		}

		//! \warning this function increases the reference count //
		DLLEXPORT GuiLoadedSheet* GetOwningSheetProxy(){
			ContainedInSheet->AddRef();
			return ContainedInSheet;
		}
		DLLEXPORT GuiLoadedSheet* GetOwningSheet(){
			return ContainedInSheet;
		}

		DLLEXPORT static bool LoadFromFileStructure(GuiManager* owner, vector<BaseGuiObject*> &tempobjects,	ObjectFileObject& dataforthis,
			GuiLoadedSheet* sheet);
	protected:
		// this function will try to hook all wanted listeners to Rocket element //
		void _HookListeners();
		virtual void _CallScriptListener(Event** pEvent, GenericEvent** event2);
		// ------------------------------------ //

		int ID;
		int FileID;

		wstring Name;

		GuiManager* OwningInstance;
		// Stores the owning sheet for easy access //
		GuiLoadedSheet* ContainedInSheet;
	};

}}
#endif