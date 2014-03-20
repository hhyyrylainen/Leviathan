#ifndef LEVIATHAN_GUICOLLECTION
#define LEVIATHAN_GUICOLLECTION
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Input/Key.h"
#include "Script/ScriptScript.h"
#include "ObjectFiles/ObjectFileObject.h"
#include "Common/ReferenceCounted.h"

namespace Leviathan{ namespace Gui{


	class GuiCollection : public Object, public ReferenceCounted{
	public:
		GuiCollection(const wstring &name, View* sheet, GuiManager* manager, int id, const wstring &toggle, bool strict = false, 
			bool enabled = true, bool keepgui = false, bool allowenable = true);
		~GuiCollection();

		DLLEXPORT void UpdateState(bool newstate);
		DLLEXPORT inline bool GetState(){
			return Enabled;
		}
		DLLEXPORT inline void ToggleState(){
			UpdateState(!Enabled);
		}

		DLLEXPORT void UpdateAllowEnable(bool newstate);
		DLLEXPORT inline bool GetAllowEnable(){
			return AllowEnable;
		}
		DLLEXPORT inline void ToggleAllowEnable(){
			UpdateAllowEnable(!AllowEnable);
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


		REFERENCECOUNTED_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(GuiCollection);

		DLLEXPORT static bool LoadCollection(GuiManager* gui, const ObjectFileObject &data, View* sheet);
	private:
		wstring Name;
		int ID;
		bool Enabled;
		bool Strict;
		bool KeepsGuiOn;
		bool AllowEnable;

		GKey Toggle;
		GuiManager* OwningManager;

		shared_ptr<ScriptScript> Scripting;
		View* ContainedInSheet;
	};

}}
#endif