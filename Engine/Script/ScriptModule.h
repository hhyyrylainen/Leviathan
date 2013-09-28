#ifndef LEVIATHAN_SCRIPTMODULE
#define LEVIATHAN_SCRIPTMODULE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Angelscript.h"
#include "add_on\scriptbuilder\scriptbuilder.h"


namespace Leviathan{

	class ScriptScript;
	class ScriptExecutor;

	enum SCRIPTBUILDSTATE{SCRIPTBUILDSTATE_EMPTY, SCRIPTBUILDSTATE_READYTOBUILD, SCRIPTBUILDSTATE_BUILT};


#define LISTENERNAME_ONSHOW				L"OnShow"
#define LISTENERNAME_ONHIDE				L"OnHide"
#define LISTENERNAME_ONLISTENUPDATE		L"OnListenUpdate"
	

#define LISTENERVALUE_ONSHOW			100
#define LISTENERVALUE_ONHIDE			101
#define LISTENERVALUE_ONLISTENUPDATE	102


	// used to store function's parameter info //
	struct FunctionParameterInfo{
		FunctionParameterInfo(int id, int sizes) : FunctionID(id), ParameterTypeIDS(sizes), ParameterDeclarations(sizes), 
			MatchingDataBlockTypes(sizes){};

		int FunctionID;

		vector<asUINT> ParameterTypeIDS;
		//vector<string> ParameterDeclarations;
		vector<wstring> ParameterDeclarations;
		vector<int> MatchingDataBlockTypes;

		asUINT ReturnTypeID;
		wstring ReturnTypeDeclaration;
		int ReturnMatchingDataBlock;
	};
	// some data that is stored when a listener is found //
	struct ValidListenerData{
		ValidListenerData(asIScriptFunction* funcptr, unique_ptr<wstring> metadataend);
		~ValidListenerData();

		asIScriptFunction* FuncPtr;
		unique_ptr<wstring> RestOfMeta;
	};

	// holds everything related to "one" script needed to run it an build it //
	class ScriptModule{
		// friend to be able to delete static objects //
		friend ScriptExecutor;
	public:
		DLLEXPORT ScriptModule(asIScriptEngine* engine, const wstring &name, int id, const string &source);
		DLLEXPORT ~ScriptModule();

		DLLEXPORT FunctionParameterInfo* GetParamInfoForFunction(asIScriptFunction* func);
		// builds the script if applicable and returns the associated module //
		DLLEXPORT asIScriptModule* GetModule();
		DLLEXPORT shared_ptr<ScriptScript> GetScriptInstance();

		DLLEXPORT inline CScriptBuilder& GetBuilder(){
			return *ScriptBuilder;
		}
		DLLEXPORT inline wstring GetName(){
			return Name;
		}
		DLLEXPORT inline int GetID(){
			return ID;
		}

		DLLEXPORT inline const string& GetIncompleteSourceCode(){
			return ObjectFileLoadedScriptCode;
		}


		DLLEXPORT void DeleteThisModule();

		DLLEXPORT bool DoesListenersContainSpecificListener(const wstring &listenername);
		DLLEXPORT void GetListOfListeners(std::vector<wstring> &receiver);

		DLLEXPORT wstring GetInfoWstring();

		DLLEXPORT inline void SetBuildState(const SCRIPTBUILDSTATE &state){
			ScriptState = state;
		}


		DLLEXPORT void PrintFunctionsInModule();

		// static map that contains listener names //
		static const std::map<wstring, int> ListenerNameType;

	private:

		void _FillParameterDataObject(int typeofas, asUINT* paramtypeid, wstring* paramdecl, int* datablocktype);
		void _BuildListenerList();
		void _ProcessMetadataForFunc(asIScriptFunction* func, asIScriptModule* mod);
		// ------------------------------------ //
		wstring Name;
		string ModuleName;
		string Source;
		string ObjectFileLoadedScriptCode;
		int ID;

		SCRIPTBUILDSTATE ScriptState;
		// flag for determining do we need to update listener data //
		bool ListenerDataBuilt;

		CScriptBuilder* ScriptBuilder; 

		std::vector<FunctionParameterInfo*> FuncParameterInfos;
		// ------------------------------------ //

		// map of found listener functions //
		std::map<wstring, shared_ptr<ValidListenerData>> FoundListenerFunctions;

		static int LatestAssigned;
	};

}
#endif



