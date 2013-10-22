#ifndef LEVIATHAN_GUICOLLECTION
#define LEVIATHAN_GUICOLLECTION
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Input\Key.h"
#include "Rocket\Core\ElementDocument.h"
#include "Script\ScriptScript.h"
#include "ObjectFiles\ObjectFileObject.h"
#include "Common\ReferenceCounted.h"

namespace Leviathan{ namespace Gui{

	// usability type define //
	typedef Key<OIS::KeyCode> GKey;

	class GuiManager;

	// this is used to pass Rocket pages to collections //
	class GuiLoadedSheet : public ReferenceCounted{
	public:
		GuiLoadedSheet(Rocket::Core::Context* context, const string &documentfile);
		~GuiLoadedSheet();

		DLLEXPORT Rocket::Core::Element* GetElementByID(const string &id);
		// warning this method increases reference count //
		DLLEXPORT Rocket::Core::Element* GetElementByIDProxy(string id){
			auto tmp = GetElementByID(id);
			if(tmp)
				tmp->AddReference();
			return tmp;
		}
		// makes the document the topmost one //
		DLLEXPORT void PullSheetToFront();
		// makes the document bottom most one //
		DLLEXPORT void PushSheetToBack();

		DLLEXPORT inline int GetID(){
			return ID;
		}

		REFERENCECOUNTED_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(GuiLoadedSheet);

	private:
		// used for finding by ID //
		Rocket::Core::ElementDocument* Document;
		// automatically generated ID //
		int ID;
	};


	class GuiCollection : public Object, public ReferenceCounted{
	public:
		GuiCollection(const wstring &name, GuiLoadedSheet* sheet, GuiManager* manager, int id, const wstring &toggle, bool strict = false, 
			bool enabled = true, bool keepgui = false);
		~GuiCollection();

		DLLEXPORT void UpdateState(bool newstate);
		DLLEXPORT inline bool GetState(){
			return Enabled;
		}
		DLLEXPORT inline void ToggleState(){
			UpdateState(!Enabled);
		}

		DLLEXPORT inline bool KeepsGUIActive(){
			return Enabled && KeepsGuiOn;
		}

		DLLEXPORT inline const GKey& GetTogglingKey(){
			return Toggle;
		}
		DLLEXPORT inline int GetID(){
			return ID;
		}
		DLLEXPORT inline const wstring& GetName(){
			return Name;
		}

		string GetNameProxy(){
			return Convert::WstringToString(Name);
		}

		DLLEXPORT GuiLoadedSheet* GetOwningSheet(){
			return ContainedInSheet;
		}
		// warning increases reference count //
		GuiLoadedSheet* GetOwningSheetProxy(){
			ContainedInSheet->AddRef();
			return ContainedInSheet;
		}


		REFERENCECOUNTED_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(GuiCollection);

		DLLEXPORT static bool LoadCollection(GuiManager* gui, const ObjectFileObject &data, GuiLoadedSheet* sheet);
	private:
		wstring Name;
		int ID;
		bool Enabled;
		bool Strict;
		bool KeepsGuiOn;

		GKey Toggle;
		GuiManager* OwningManager;

		shared_ptr<ScriptScript> Scripting;
		GuiLoadedSheet* ContainedInSheet;
	};

}}
#endif