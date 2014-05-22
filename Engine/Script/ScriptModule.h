#ifndef LEVIATHAN_SCRIPTMODULE
#define LEVIATHAN_SCRIPTMODULE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "angelscript.h"
#include "add_on/scriptbuilder/scriptbuilder.h"


namespace Leviathan{

	enum SCRIPTBUILDSTATE{SCRIPTBUILDSTATE_EMPTY, SCRIPTBUILDSTATE_READYTOBUILD, SCRIPTBUILDSTATE_BUILT, SCRIPTBUILDSTATE_FAILED};


#define LISTENERNAME_ONSHOW				L"OnShow"
#define LISTENERNAME_ONHIDE				L"OnHide"
#define LISTENERNAME_ONLISTENUPDATE		L"OnListenUpdate"
#define LISTENERNAME_ONCLICK			L"OnClick"
#define LISTENERNAME_ONINIT				L"OnInit"
#define LISTENERNAME_ONRELEASE			L"OnRelease"
#define LISTENERNAME_ONVALUECHANGE		L"OnValueChange"
#define LISTENERNAME_ONSUBMIT			L"OnSubmit"
#define LISTENERNAME_ONTICK				L"OnTick"

#define LISTENERVALUE_ONSHOW			100
#define LISTENERVALUE_ONHIDE			101
#define LISTENERVALUE_ONLISTENUPDATE	102
#define LISTENERVALUE_ONCLICK			103
#define LISTENERVALUE_ONINIT			104
#define LISTENERVALUE_ONRELEASE			105
#define LISTENERVALUE_ONVALUECHANGE		106
#define LISTENERVALUE_ONSUBMIT			107
#define LISTENERVALUE_ONTICK			108


	//! used to store function's parameter info
	struct FunctionParameterInfo{
		FunctionParameterInfo(int id, int sizes) : FunctionID(id), ParameterTypeIDS(sizes), ParameterDeclarations(sizes),
			MatchingDataBlockTypes(sizes), ReturnMatchingDataBlock(-1){};

		int FunctionID;

		vector<asUINT> ParameterTypeIDS;
		//vector<string> ParameterDeclarations;
		vector<wstring> ParameterDeclarations;
		vector<int> MatchingDataBlockTypes;

		asUINT ReturnTypeID;
		wstring ReturnTypeDeclaration;
		int ReturnMatchingDataBlock;
	};

	//! some data that is stored when a listener is found
	struct ValidListenerData{
		ValidListenerData(asIScriptFunction* funcptr, wstring* name, wstring* metadataend);
		ValidListenerData(asIScriptFunction* funcptr, wstring* name, wstring* metadataend, wstring* generictypename);
		~ValidListenerData();

		asIScriptFunction* FuncPtr;
		unique_ptr<wstring> ListenerName;
		unique_ptr<wstring> RestOfMeta;
		unique_ptr<wstring> GenericTypeName;
	};

	//! \brief Represents a section of script source file
	struct ScriptSourceFileData{

		ScriptSourceFileData(const string &file, int line, const string &code);


		string SourceFile;
		int StartLine;

		//! The source is stored here to allow saving it to a file
		//! This being a shared pointer allows for more efficient copying
		shared_ptr<string> SourceCode;
	};


	// \brief Holds everything related to "one" script needed to run it an build it
	class ScriptModule{
		// friend to be able to delete static objects //
		friend ScriptExecutor;
	public:
		DLLEXPORT ScriptModule(asIScriptEngine* engine, const wstring &name, int id, const string &source);
		DLLEXPORT ~ScriptModule();

		DLLEXPORT FunctionParameterInfo* GetParamInfoForFunction(asIScriptFunction* func);

		//! \brief Builds the script if applicable
		//! \return The associated module or NULL if build fails
		DLLEXPORT asIScriptModule* GetModule();

		DLLEXPORT shared_ptr<ScriptScript> GetScriptInstance();


		DLLEXPORT inline wstring GetName(){
			return Name;
		}
		DLLEXPORT inline int GetID(){
			return ID;
		}

		DLLEXPORT const string& GetSource() const{
			return Source;
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
		
		//! \brief Gets the number of code segments
		//! \see GetScriptSegment
		DLLEXPORT size_t GetScriptSegmentCount() const;

		//! \brief Gets the data associated with a code segment
		//! \param index The index of the segment in the vector, use GetScriptSegmentCount to get max index
		//! \return The segments data or NULL
		DLLEXPORT shared_ptr<ScriptSourceFileData> GetScriptSegment(size_t index) const;


		//! \brief Adds a new script section
		//! \return True when the file is not included already (and it got added), false otherwise
		DLLEXPORT FORCE_INLINE bool AddScriptSegment(const string &file, int line, const string &code){

			return AddScriptSegment(shared_ptr<ScriptSourceFileData>(new ScriptSourceFileData(file, line, code)));
		}

		//! \brief The actual implementation of AddScriptSegment
		DLLEXPORT bool AddScriptSegment(shared_ptr<ScriptSourceFileData> data);

		//! \brief Adds an entire file as a script segment
		//! \return True when the file is added, false if the file was already added
		DLLEXPORT bool AddScriptSegmentFromFile(const string &file);


		DLLEXPORT void PrintFunctionsInModule();

		// static map that contains listener names //
		static const std::map<wstring, int> ListenerNameType;

		// static include resolver for scripts //
		DLLEXPORT static int ScriptModuleIncludeCallback(const char* include, const char* from, CScriptBuilder* builder, void* userParam);

	private:
		
		ScriptModule(const ScriptModule &other){}
		
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


		//! Flag for determining if we need to update listener data
		bool ListenerDataBuilt;

		SCRIPTBUILDSTATE ScriptState;
		CScriptBuilder* ScriptBuilder;


		//! The raw script source code for returning to writing to files
		std::vector<shared_ptr<ScriptSourceFileData>> ScriptSourceSegments;



		std::vector<FunctionParameterInfo*> FuncParameterInfos;


		//! Map of found listener functions
		std::map<wstring, shared_ptr<ValidListenerData>> FoundListenerFunctions;


		//! Last ID of a ScriptModule, used to generate unique IDs for modules
		static int LatestAssigned;
	};

}
#endif



