#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include <memory>

namespace Leviathan{

	//! holds a reference of script module
    //! \todo Remove this and replace with direct access to ScriptModule
	class ScriptScript{
	public:
		DLLEXPORT ScriptScript(std::weak_ptr<ScriptModule> wptr);
		DLLEXPORT ScriptScript(const int &MID, std::weak_ptr<ScriptModule> wptr);
		DLLEXPORT ScriptScript(const ScriptScript &other);
		DLLEXPORT ~ScriptScript();

		//! \brief Returns an unsafe pointer to the module
		//! \deprecated GetModuleSafe should be used instead, will probably be around for a while
		DLLEXPORT inline ScriptModule* GetModule(){
			return ScriptsModule.lock().get();
		}


		//! \brief Locks the weak pointer and returns the std::shared_ptr
		//! \note This should be used instead of GetModule if some methods are called on the pointer
		DLLEXPORT inline std::shared_ptr<ScriptModule> GetModuleSafe(){

			return ScriptsModule.lock();
		}



	private:
		// reference to module //
        std::weak_ptr<ScriptModule> ScriptsModule;
		int ModuleID;
	};

}

