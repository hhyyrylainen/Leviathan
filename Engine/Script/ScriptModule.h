// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "AccessMask.h"

#include "Common/ThreadSafe.h"
#include "ScriptArgumentsProvider.h"

#include "add_on/scriptbuilder/scriptbuilder.h"
#include "angelscript.h"

#include "boost/thread/mutex.hpp"

#define SCRIPTMODULE_LISTENFORFILECHANGES

namespace Leviathan {

enum SCRIPTBUILDSTATE {

    SCRIPTBUILDSTATE_EMPTY,
    SCRIPTBUILDSTATE_READYTOBUILD,
    SCRIPTBUILDSTATE_BUILT,
    SCRIPTBUILDSTATE_FAILED,
    //! Only set when the module can no longer be retrieved,
    //! and the whole ScriptModule needs to be recreated
    SCRIPTBUILDSTATE_DISCARDED
};


//! some data that is stored when a listener is found
struct ValidListenerData {
    ValidListenerData(asIScriptFunction* funcptr, std::string* name, std::string* metadataend);
    ValidListenerData(asIScriptFunction* funcptr, std::string* name, std::string* metadataend,
        std::string* generictypename);
    ~ValidListenerData();

    asIScriptFunction* FuncPtr;
    std::unique_ptr<std::string> ListenerName;
    std::unique_ptr<std::string> RestOfMeta;
    std::unique_ptr<std::string> GenericTypeName;
};

//! \brief Represents a section of script source file
struct ScriptSourceFileData {

    //! \param line The line to start from. First line in a file is 1
    DLLEXPORT ScriptSourceFileData(const std::string& file, int line, const std::string& code);


    std::string SourceFile;
    int StartLine;

    //! The source is stored here to allow saving it to a file
    //! This being a shared pointer allows for more efficient copying
    std::shared_ptr<std::string> SourceCode;
};


// \brief Holds everything related to "one" script needed to run it an build it
class ScriptModule : public ThreadSafe, public std::enable_shared_from_this<ScriptModule> {
    // friend to be able to delete static objects //
    friend ScriptExecutor;

public:
    DLLEXPORT ScriptModule(
        asIScriptEngine* engine, const std::string& name, int id, const std::string& source);
    DLLEXPORT ~ScriptModule();

    //! \brief Sets the access mask to be used for this script
    //!
    //! This only takes effect before building the module
    DLLEXPORT inline void SetAccessMask(AccessFlags access)
    {
        AccessMask = access;
    }

    //! \brief Adds flags to the AccessMask
    //! \see SetAccessMask
    DLLEXPORT void AddAccessRight(AccessFlags newaccess)
    {
        AccessMask |= newaccess;
    }

    //! \brief Builds the script if applicable
    //! \return The associated module or NULL if build fails
    DLLEXPORT asIScriptModule* GetModule(Lock& guard);

    DLLEXPORT inline asIScriptModule* GetModule()
    {
        GUARD_LOCK();
        return GetModule(guard);
    }

    DLLEXPORT std::shared_ptr<ScriptScript> GetScriptInstance();


    DLLEXPORT inline std::string GetName()
    {
        return Name;
    }


    //! \brief Releases the internal resources
    DLLEXPORT void Release();

    //! \brief Gets the name of the internal AngelScript module
    DLLEXPORT const std::string& GetModuleName() const;

    DLLEXPORT inline int GetID()
    {
        return ID;
    }

    DLLEXPORT const std::string& GetSource() const
    {
        return Source;
    }

    DLLEXPORT inline const std::string& GetIncompleteSourceCode()
    {
        return ObjectFileLoadedScriptCode;
    }


    DLLEXPORT void DeleteThisModule();

    DLLEXPORT bool DoesListenersContainSpecificListener(
        const std::string& listenername, const std::string* generictype = NULL);

    DLLEXPORT void GetListOfListeners(
        std::vector<std::shared_ptr<ValidListenerData>>& receiver);

    DLLEXPORT std::string GetListeningFunctionName(
        const std::string& listenername, const std::string* generictype = NULL);

    DLLEXPORT std::string GetInfoString();

    DLLEXPORT inline void SetBuildState(const SCRIPTBUILDSTATE& state)
    {
        ScriptState = state;
    }

    //! \brief Gets the number of code segments
    //! \see GetScriptSegment
    DLLEXPORT size_t GetScriptSegmentCount() const;

    //! \brief Gets the data associated with a code segment
    //! \param index The index of the segment in the vector,
    //! use GetScriptSegmentCount to get max index
    //! \return The segments data or NULL
    DLLEXPORT std::shared_ptr<ScriptSourceFileData> GetScriptSegment(size_t index) const;


    //! \brief Adds a new script section
    //! \return True when the file is not included already (and it got added),
    //! false otherwise
    DLLEXPORT FORCE_INLINE bool AddScriptSegment(
        const std::string& file, int line, const std::string& code)
    {
        return AddScriptSegment(std::make_shared<ScriptSourceFileData>(file, line, code));
    }

    //! \brief The actual implementation of AddScriptSegment
    DLLEXPORT bool AddScriptSegment(std::shared_ptr<ScriptSourceFileData> data);


    //! \brief Adds an entire file as a script segment
    //! \return True when the file is added, false if the file was already added
    DLLEXPORT bool AddScriptSegmentFromFile(const std::string& file);


    //! \brief Rebuilds the module and tries to restore data
    //!
    //! Not all data can be restored (things changing types,
    //! application owned handles etc...)
    DLLEXPORT bool ReLoadModuleCode();

    //! \brief Sets the module as invalid to avoid usage
    DLLEXPORT void SetAsInvalid();


    DLLEXPORT void PrintFunctionsInModule();

    // static map that contains listener names //
    static const std::map<std::string, int> ListenerNameType;

    // static include resolver for scripts //
    DLLEXPORT static int ScriptModuleIncludeCallback(
        const char* include, const char* from, CScriptBuilder* builder, void* userParam);

    //! Finds a path to source file or returns an empty string
    DLLEXPORT static std::string ResolvePathToScriptFile(const std::string& inputfilename,
        const std::string& relativepath, bool checkworkdirrelative = true);

    //! \brief Call when this module is added to a bridge
    //! \return True when this is not in a bridge and it is added,
    //! false if this is removed from the bridge
    DLLEXPORT bool OnAddedToBridge(std::shared_ptr<ScriptArgumentsProviderBridge> bridge);

private:
    ScriptModule(const ScriptModule& other) {}

    void _FillParameterDataObject(
        int typeofas, int* paramtypeid, std::string* paramdecl, int* datablocktype);

    void _BuildListenerList(Lock& guard);

    //! \todo This needs handling of multiple metadata entries that are now allowed by
    //! AngelScript
    void _ProcessMetadataForFunc(asIScriptFunction* func, asIScriptModule* mod);

    std::map<std::string, std::shared_ptr<ValidListenerData>>::iterator _GetIteratorOfListener(
        Lock& guard, const std::string& listenername, const std::string* generictype = NULL);

    //! \brief Tries to build the module and sets the state accordingly
    void _BuildTheModule(Lock& guard);

#ifdef SCRIPTMODULE_LISTENFORFILECHANGES

    //! \brief Starts monitoring for changes to all script segments and all included files
    //! \see ReLoadModuleCode
    void _StartMonitoringFiles(Lock& guard);

    //! \brief Releases file listeners
    //! \note This will need to be called before deleting this object
    //! unless the user wants access violations
    void _StopFileMonitoring(Lock& guard);


    //! \brief Adds a new file to monitor, if required
    //! \note The object needs to be locked before this call
    void _AddFileToMonitorIfNotAlready(const std::string& file);

#endif // SCRIPTMODULE_LISTENFORFILECHANGES


    // ------------------------------------ //

    std::string Name;
    std::string ModuleName;
    std::string Source;
    std::string ObjectFileLoadedScriptCode;
    int ID;


    //! Flag for determining if we need to update listener data
    bool ListenerDataBuilt;

    //! The access flags to set to the angelscript before building it
    AccessFlags AccessMask = DefaultAccessFlags;

    SCRIPTBUILDSTATE ScriptState = SCRIPTBUILDSTATE_EMPTY;
    CScriptBuilder* ScriptBuilder;


    //! The raw script source code for returning to writing to files
    std::vector<std::shared_ptr<ScriptSourceFileData>> ScriptSourceSegments;


    //! THe direct pointer to the module, this is stored to avoid searching
    asIScriptModule* ASModule = nullptr;


    //! Map of found listener functions
    std::map<std::string, std::shared_ptr<ValidListenerData>> FoundListenerFunctions;


    //! Last ID of a ScriptModule, used to generate unique IDs for modules
    static int LatestAssigned;


    //! Only one module can build code at a time so this mutex has to be locked
    //! while building
    static Mutex ModuleBuildingMutex;

    //! A connection to an object that provides us with parameters for automatically called
    //! script functions
    std::shared_ptr<ScriptArgumentsProviderBridge> ArgsBridge;

    // Data for file listening //
#ifdef SCRIPTMODULE_LISTENFORFILECHANGES

    //! The list of file listeners used to release them when this module is about to
    //! be deleted
    std::vector<int> FileListeners;


    //! \brief Holds data related to a monitored script file
    //! \todo Cache the file path to allow faster lookup
    struct AutomonitoredFile {

        AutomonitoredFile(const std::string& file) : File(new std::string(file)), Added(false)
        {}

        std::unique_ptr<std::string> File;
        bool Added;
    };

    //! List of files that are already monitored, used to avoid duplicates
    std::vector<std::unique_ptr<AutomonitoredFile>> AlreadyMonitoredFiles;


    //! The function that is called when one of our files change
    void _FileChanged(const std::string& file, ResourceFolderListener& caller);

#endif // SCRIPTMODULE_LISTENFORFILECHANGES
};

} // namespace Leviathan
