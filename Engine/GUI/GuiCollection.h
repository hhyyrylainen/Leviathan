#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "../Input/Key.h"
#include "../Script/ScriptScript.h"
#include "../ObjectFiles/ObjectFile.h"
#include "../Common/ReferenceCounted.h"

namespace Leviathan{
namespace GUI{


class GuiCollection : public ReferenceCounted{
public:
    GuiCollection(const std::string &name, GuiManager* manager, int id,
        const std::string &toggle, std::vector<std::unique_ptr<std::string>> &inanimations, 
        std::vector<std::unique_ptr<std::string>> &outanimations,bool strict = false,
        bool enabled = true, bool keepgui = false, bool allowenable = true,
        const std::string &autotarget = "", bool applyanimstochildren = false);
        
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
    DLLEXPORT inline const std::string& GetName(){
        return Name;
    }

    std::string GetNameProxy(){
        return Name;
    }


    REFERENCECOUNTED_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(GuiCollection);

    DLLEXPORT static bool LoadCollection(GuiManager* gui, const ObjectFileObject &data);
private:

    void _PlayAnimations(const std::vector<std::unique_ptr<std::string>> &anims);

    // ------------------------------------ //

    std::string Name;
    int ID;
    bool Enabled;
    bool Strict;
    bool KeepsGuiOn;
    bool AllowEnable;

    //! The CEGUI window that is automatically controlled
    //! \todo Allow script to be called after this
    std::string AutoTarget;

    //! The CEGUI animations that are automatically used
    std::vector<std::unique_ptr<std::string>> AutoAnimationOnEnable;
    std::vector<std::unique_ptr<std::string>> AutoAnimationOnDisable;

    //! Flag for using the same animations in AutoAnimationOnEnable for their child windows
    bool ApplyAnimationsToChildren;

    GKey Toggle;
    GuiManager* OwningManager;

    std::shared_ptr<ScriptScript> Scripting;
};

}}

