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
#define LISTENERNAME_ONCLICK			L"OnClick"
#define LISTENERNAME_ONINIT				L"OnInit"
#define LISTENERNAME_ONRELEASE			L"OnRelease"	

#define LISTENERVALUE_ONSHOW			100
#define LISTENERVALUE_ONHIDE			101
#define LISTENERVALUE_ONLISTENUPDATE	102
#define LISTENERVALUE_ONCLICK			103
#define LISTENERVALUE_ONINIT			104
#define LISTENERVALUE_ONRELEASE			105


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
		ValidListenerData(asIScriptFunction* funcptr, wstring* name, wstring* metadataend);
		ValidListenerData(asIScriptFunction* funcptr, wstring* name, wstring* metadataend, wstring* generictypename);
		~ValidListenerData();

		asIScriptFunction* FuncPtr;
		unique_ptr<wstring> ListenerName;
		unique_ptr<wstring> RestOfMeta;
		unique_ptr<wstring> GenericTypeName;
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

		DLLEXPORT bool DoesListenersContainSpecificListener(const wstring &listenername, const wstring* generictype = NULL);
		DLLEXPORT void GetListOfListeners(std::vector<shared_ptr<ValidListenerData>> &receiver);
		DLLEXPORT string GetListeningFunctionName(const wstring &listenername, const wstring* generictype = NULL);

		DLLEXPORT wstring GetInfoWstring();

		DLLEXPORT inline void SetBuildState(const SCRIPTBUILDSTATE &state){
			ScriptState = state;
		}


		DLLEXPORT void PrintFunctionsInModule();

		// static map that contains listener names //
		static const std::map<wstring, int> ListenerNameType;

		// static include resolver for scripts //
		DLLEXPORT static int ScriptModuleIncludeCallback(const char* include, const char* from, CScriptBuilder* builder, void* userParam);

	private:

		void _FillParameterDataObject(int typeofas, asUINT* paramtypeid, wstring* paramdecl, int* datablocktype);
		void _BuildListenerList();
		void _ProcessMetadataForFunc(asIScriptFunction* func, asIScriptModule* mod);
		std::map<wstring, shared_ptr<ValidListenerData>>::iterator _GetIteratorOfListener(const wstring &listenername, const wstring* generictype = NULL);
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



