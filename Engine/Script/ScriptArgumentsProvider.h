#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "ScriptRunningSetup.h"


namespace Leviathan{


	//! \brief Classes implementing this interface allow script modules to run automatic OnInit and
    //! OnRelease function during release
	class ScriptArgumentsProvider{
	public:


		DLLEXPORT virtual std::unique_ptr<ScriptRunningSetup> GetParametersForInit() = 0;
		DLLEXPORT virtual std::unique_ptr<ScriptRunningSetup> GetParametersForRelease() = 0;

	protected:


		DLLEXPORT void _BondWithModule(ScriptModule* module);

		DLLEXPORT void _LeaveBondBridge();


        std::shared_ptr<ScriptArgumentsProviderBridge> _ArgumentBridge;
	};



	//! \brief Forms a connection between a ScriptModule and a ScriptArgumentsProvider which either can disconnect
	//!
	//! This is used to allow either of the objects to be destructed before the other.
    //! Another option would have been using Notifiers, but this might be a cleaner solution
	class ScriptArgumentsProviderBridge{
	public:

		ScriptArgumentsProviderBridge() : OwningModule(NULL), OwningProvider(NULL){}

		DLLEXPORT inline void SetModule(ScriptModule* module){

			OwningModule = module;
		}

		DLLEXPORT inline ScriptModule* GetModule(){

			return OwningModule;
		}

		DLLEXPORT inline void LeaveModule(){

			OwningModule = NULL;
		}

		DLLEXPORT inline void SetProvider(ScriptArgumentsProvider* provider){

			OwningProvider = provider;
		}

		DLLEXPORT inline ScriptArgumentsProvider* GetProvider(){

			return OwningProvider;
		}

		DLLEXPORT inline void LeaveProvider(){

			OwningProvider = NULL;
		}

	protected:


		ScriptModule* OwningModule;
		ScriptArgumentsProvider* OwningProvider;
	};

}

