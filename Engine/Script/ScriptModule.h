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
#include "Common/ThreadSafe.h"
#include "boost/thread/mutex.hpp"
#include "ScriptArgumentsProvider.h"

#define SCRIPTMODULE_LISTENFORFILECHANGES



namespace Leviathan{

	enum SCRIPTBUILDSTATE{
        
        SCRIPTBUILDSTATE_EMPTY, SCRIPTBUILDSTATE_READYTOBUILD, SCRIPTBUILDSTATE_BUILT, SCRIPTBUILDSTATE_FAILED,
		//! Only set when the module can no longer be retrieved, and the whole ScriptModule needs to be recreated
		SCRIPTBUILDSTATE_DISCARDED
	};


#define LISTENERNAME_ONSHOW					L"OnShow"
#define LISTENERNAME_ONHIDE					L"OnHide"
#define LISTENERNAME_ONLISTENUPDATE			L"OnListenUpdate"
#define LISTENERNAME_ONCLICK				L"OnClick"
#define LISTENERNAME_ONINIT					L"OnInit"
#define LISTENERNAME_ONRELEASE				L"OnRelease"
#define LISTENERNAME_ONVALUECHANGE			L"OnValueChange"
#define LISTENERNAME_ONSUBMIT				L"OnSubmit"
#define LISTENERNAME_ONTICK					L"OnTick"
#define LISTENERNAME_ONCLOSECLICKED			L"OnCloseClicked"
#define LISTENERNAME_LISTSELECTIONACCEPTED  L"OnListSelectionAccepted"

#define LISTENERVALUE_ONSHOW				100
#define LISTENERVALUE_ONHIDE				101
#define LISTENERVALUE_ONLISTENUPDATE		102
#define LISTENERVALUE_ONCLICK				103
#define LISTENERVALUE_ONINIT				104
#define LISTENERVALUE_ONRELEASE				105
#define LISTENERVALUE_ONVALUECHANGE			106
#define LISTENERVALUE_ONSUBMIT				107
#define LISTENERVALUE_ONTICK				108
#define LISTENERVALUE_ONCLOSECLICKED		109
#define LISTENERVALUE_LISTSELECTIONACCEPTED 110


	//! used to store function's parameter info
	struct FunctionParameterInfo{
		FunctionParameterInfo(int id, int sizes) :
            FunctionID(id), ParameterTypeIDS(sizes), ParameterDeclarations(sizes),
			MatchingDataBlockTypes(sizes), ReturnMatchingDataBlock(-1)
        {
            
        };

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

        //! \param line The line to start from. First line in a file is 1
		ScriptSourceFileData(const string &file, int line, const string &code);


		string SourceFile;
		int StartLine;

		//! The source is stored here to allow saving it to a file
		//! This being a shared pointer allows for more efficient copying
		shared_ptr<string> SourceCode;
	};


	// \brief Holds everything related to "one" script needed to run it an build it
	class ScriptModule : public ThreadSafe{
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


		//! \brief Releases the internal resources
		DLLEXPORT void Release();

		//! \brief Gets the name of the internal AngelScript module
		DLLEXPORT const string& GetModuleName() const;

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


		//! \brief Rebuilds the module and tries to restore data
		//!
		//! Not all data can be restored (things changing types, application owned handles etc...)
		//! \todo Add calls to OnRelease and OnInit events, just needs a way for the script owner to provide parameters
		DLLEXPORT bool ReLoadModuleCode();

		//! \brief Sets the module as invalid to avoid usage
		DLLEXPORT void SetAsInvalid();


		DLLEXPORT void PrintFunctionsInModule();

		// static map that contains listener names //
		static const std::map<wstring, int> ListenerNameType;

		// static include resolver for scripts //
		DLLEXPORT static int ScriptModuleIncludeCallback(const char* include, const char* from, CScriptBuilder* builder, void* userParam);


		//! \brief Call when this module is added to a bridge
		//! \return True when this is not in a bridge and it is added, false if this is removed from the bridge
		DLLEXPORT bool OnAddedToBridge(shared_ptr<ScriptArgumentsProviderBridge> bridge);

	private:
		
		ScriptModule(const ScriptModule &other){}
		
		void _FillParameterDataObject(int typeofas, asUINT* paramtypeid, wstring* paramdecl, int* datablocktype);
		void _BuildListenerList(ObjectLock &guard);
		void _ProcessMetadataForFunc(asIScriptFunction* func, asIScriptModule* mod);
		std::map<wstring, shared_ptr<ValidListenerData>>::iterator _GetIteratorOfListener(ObjectLock &guard, const wstring &listenername, const wstring* generictype = NULL);


		//! \brief Tries to build the module and sets the state accordingly
		void _BuildTheModule();

#ifdef SCRIPTMODULE_LISTENFORFILECHANGES

		//! \brief Starts monitoring for changes to all script segments and all included files
		//! \see ReLoadModuleCode
		void _StartMonitoringFiles();

		//! \brief Releases file listeners
		//! \note This will need to be called before deleting this object unless the user wants access violations
		void _StopFileMonitoring();


		//! \brief Adds a new file to monitor, if required
		void _AddFileToMonitorIfNotAlready(const string &file);

#endif // SCRIPTMODULE_LISTENFORFILECHANGES


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


		//! THe direct pointer to the module, this is stored to avoid searching
		asIScriptModule* ASModule;


		std::vector<FunctionParameterInfo*> FuncParameterInfos;


		//! Map of found listener functions
		std::map<wstring, shared_ptr<ValidListenerData>> FoundListenerFunctions;


		//! Last ID of a ScriptModule, used to generate unique IDs for modules
		static int LatestAssigned;


		//! Only one module can build code at a time so this mutex has to be locked while building
		static boost::mutex ModuleBuildingMutex;

		//! A connection to an object that provides us with parameters for automatically called script functions
		shared_ptr<ScriptArgumentsProviderBridge> ArgsBridge;


		// Data for file listening //
#ifdef SCRIPTMODULE_LISTENFORFILECHANGES

		//! The list of file listeners used to release them when this module is about to be deleted
		std::vector<int> FileListeners;


		//! \brief Holds data related to a monitored script file
		//! \todo Cache the file path to allow faster lookup
		struct AutomonitoredFile{

			AutomonitoredFile(const string &file) : File(new wstring(Convert::StringToWstring(file))), Added(false){
			}

			unique_ptr<wstring> File;
			bool Added;
		};

		//! List of files that are already monitored, used to avoid duplicates
		std::vector<unique_ptr<AutomonitoredFile>> AlreadyMonitoredFiles;


		//! The function that is called when one of our files change
		void _FileChanged(const wstring &file, ResourceFolderListener &caller);

#endif // SCRIPTMODULE_LISTENFORFILECHANGES

	};

}
#endif



