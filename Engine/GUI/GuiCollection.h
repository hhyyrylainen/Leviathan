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
		GuiCollection(const wstring &name, GuiManager* manager, int id, const wstring &toggle, std::vector<unique_ptr<wstring>> &inanimations, 
			std::vector<unique_ptr<wstring>> &outanimations,bool strict = false, bool enabled = true, 
			bool keepgui = false, bool allowenable = true, const wstring &autotarget = L"", bool applyanimstochildren = false);
		~GuiCollection();

		//! \todo Allow script listeners to be executed even if custom animations are used
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

		DLLEXPORT static bool LoadCollection(GuiManager* gui, const ObjectFileObject &data);
	private:

		void _PlayAnimations(const std::vector<unique_ptr<wstring>> &anims);

		// ------------------------------------ //

		wstring Name;
		int ID;
		bool Enabled;
		bool Strict;
		bool KeepsGuiOn;
		bool AllowEnable;

		//! The CEGUI window that is automatically controlled
		//! \todo Allow script to be called after this
		wstring AutoTarget;

		//! The CEGUI animations that are automatically used
		std::vector<unique_ptr<wstring>> AutoAnimationOnEnable;
		std::vector<unique_ptr<wstring>> AutoAnimationOnDisable;

		//! Flag for using the same animations in AutoAnimationOnEnable for their child windows
		bool ApplyAnimationsToChildren;




		GKey Toggle;
		GuiManager* OwningManager;

		shared_ptr<ScriptScript> Scripting;
	};

}}
#endif