#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GAMEMODULE
#include "GameModule.h"
#endif
#include "ObjectFiles/ObjectFileProcessor.h"
#include "FileSystem.h"
#include "Common/StringOperations.h"
#include "Script/ScriptExecutor.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::GameModule::GameModule(const wstring &modulename, const wstring &ownername,
    const wstring &extension /*= L"txt|levgm"*/) : OwnerName(ownername), LoadedFromFile(modulename)
{
	// Find the actual file //
	wstring file = FileSystem::Get()->SearchForFile(FILEGROUP_SCRIPT, modulename, extension, false);

	if(file.size() == 0){
		// Couldn't find file //

		throw InvalidArgument("File not found");
	}

	// Load the file //
	auto ofile = ObjectFileProcessor::ProcessObjectFile(file);

	if(!ofile){

		throw InvalidArgument("File is invalid");
	}

	// Process the objects //
	if(ofile->GetTotalObjectCount() != 1){

		throw InvalidArgument("File contains invalid number of objects, single GameModule "
            "expected");
	}

	// Get various data from the header //
	ObjectFileProcessor::LoadValueFromNamedVars<wstring>(ofile->GetVariables(), L"Version", Name,
        L"-1", true, L"GameModule:");

	auto gmobject = ofile->GetObjectFromIndex(0);

	Name = gmobject->GetName();

	// handle the single object //
	ObjectFileList* properties = gmobject->GetListWithName(L"properties");
	ObjectFileTextBlock* sources = gmobject->GetTextBlockWithName(L"sourcefiles");

	if(!properties || !sources){

		throw InvalidArgument("File contains invalid GameModule, properties or sourcefiles not found");
	}

	// Copy data //
	if(sources->GetLineCount() < 1){

		throw InvalidArgument("At least one source file expected in sourcefiles");
	}


	wstring sourcefilename = StringOperations::RemoveExtensionWstring(sources->GetLine(0), true);
	wstring extensions = StringOperations::GetExtensionWstring(sources->GetLine(0));

	SourceFile = FileSystem::Get()->SearchForFile(FILEGROUP_SCRIPT, sourcefilename, extensions, false);
}

DLLEXPORT Leviathan::GameModule::~GameModule(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::GameModule::Init(){
	// Compile a new module //
	ScriptModule* mod = NULL;

	if(!Scripting){

		Scripting = shared_ptr<ScriptScript>(new
            ScriptScript(ScriptExecutor::Get()->CreateNewModule(
                    L"GameModule("+Name+L") ScriptModule", Convert::WstringToString(SourceFile))));

		// Get the newly created module //
		mod = Scripting->GetModule();

		mod->AddScriptSegmentFromFile(Convert::WstringToString(SourceFile));
		mod->SetBuildState(SCRIPTBUILDSTATE_READYTOBUILD);


	} else {
		// Get already created module //
		mod = Scripting->GetModule();
	}

	// Build the module (by creating a callback list) //
	std::vector<shared_ptr<ValidListenerData>> containedlisteners;

	mod->GetListOfListeners(containedlisteners);

	if(mod->GetModule() == NULL){
		// Fail to build //
		Logger::Get()->Error("GameModule: Init: failed to build AngelScript module");
		return false;
	}

	for(size_t i = 0; i < containedlisteners.size(); i++){
		// Bind generic event //
		if(containedlisteners[i]->GenericTypeName &&
            containedlisteners[i]->GenericTypeName->size() > 0)
        {

			// custom event listener //
			RegisterForEvent(*containedlisteners[i]->GenericTypeName);
			continue;
		}

		// look for global events //
		EVENT_TYPE etype = ResolveStringToType(*containedlisteners[i]->ListenerName);
		if(etype != EVENT_TYPE_ERROR){

			RegisterForEvent(etype);
			continue;
		}

		Logger::Get()->Warning(L"GameModule: unknown event type "+
            *containedlisteners[i]->ListenerName+L", did you intent to use Generic type?");
	}

	// Call init callbacks //

	// fire an event //
	Event* tmpevent = new Event(EVENT_TYPE_INIT, NULL);

	OnEvent(&tmpevent);

	tmpevent->Release();
	return true;
}

DLLEXPORT void Leviathan::GameModule::ReleaseScript(){
	// Call release callback and destroy everything //
	// fire an event //
	Event* tmpevent = new Event(EVENT_TYPE_RELEASE, NULL);

	OnEvent(&tmpevent);

	tmpevent->Release();

	// Remove our reference //
	int tmpid = Scripting->GetModule()->GetID();
	Scripting.reset();

	ScriptExecutor::Get()->DeleteModuleIfNoExternalReferences(tmpid);
}
// ------------------------------------ //
DLLEXPORT wstring Leviathan::GameModule::GetDescriptionForError(bool full /*= false*/){
	return L"GameModule("+Name+(full ? L" v"+Version+L") ": L") ")+L" owned by: "+OwnerName+
        (full ? L", loaded from file: "+LoadedFromFile+L".": L".");
}

DLLEXPORT string Leviathan::GameModule::GetDescriptionProxy(bool full){
	return Convert::WstringToString(GetDescriptionForError(full));
}
// ------------------------------------ //
void Leviathan::GameModule::_CallScriptListener(Event** pEvent, GenericEvent** event2){

	ScriptModule* mod = Scripting->GetModule();

	if(pEvent){
		// Get the listener name from the event type //
		wstring listenername = GetListenerNameFromType((*pEvent)->GetType());

		// check does the script contain right listeners //
		if(mod->DoesListenersContainSpecificListener(listenername)){
			// setup parameters //
			vector<shared_ptr<NamedVariableBlock>> Args = boost::assign::list_of(new
                NamedVariableBlock(new VoidPtrBlock(this), L"GameModule"))
				(new NamedVariableBlock(new VoidPtrBlock(*pEvent), L"Event"));
			// we are returning ourselves so increase refcount
			AddRef();
			(*pEvent)->AddRef();

			ScriptRunningSetup sargs;
			sargs.SetEntrypoint(mod->GetListeningFunctionName(listenername)).SetArguments(Args);
            
			// run the script //
			shared_ptr<VariableBlock> result = ScriptExecutor::Get()->RunSetUp(
                Scripting.get(), &sargs);
            
			// Do something with the result //
		}
	} else {
		// generic event is passed //
		if(mod->DoesListenersContainSpecificListener(L"", (*event2)->GetTypePtr())){
			// setup parameters //
			vector<shared_ptr<NamedVariableBlock>> Args = boost::assign::list_of(new
                NamedVariableBlock(new VoidPtrBlock(this), L"GameModule"))
				(new NamedVariableBlock(new VoidPtrBlock(*event2), L"GenericEvent"));
			// we are returning ourselves so increase refcount
			AddRef();
			(*event2)->AddRef();

			ScriptRunningSetup sargs;
			sargs.SetEntrypoint(mod->GetListeningFunctionName(L"", (*event2)->GetTypePtr())).
                SetArguments(Args);
            
			// run the script //
			shared_ptr<VariableBlock> result = ScriptExecutor::Get()->RunSetUp(
                Scripting.get(), &sargs);
			// do something with result //
		}
	}
}
// ------------------ Being an actual module ------------------ //
DLLEXPORT shared_ptr<VariableBlock> Leviathan::GameModule::ExecuteOnModule(const string &entrypoint,
    std::vector<shared_ptr<NamedVariableBlock>> &otherparams, bool &existed,
    bool fulldeclaration /*= false*/)
{
	// Add this as parameter //
	otherparams.insert(otherparams.begin(), shared_ptr<NamedVariableBlock>(new
            NamedVariableBlock(new VoidPtrBlock(this), L"GameModule")));

	// we are returning ourselves so increase refcount
	AddRef();

	ScriptRunningSetup setup;
	setup.SetArguments(otherparams).SetEntrypoint(entrypoint).SetUseFullDeclaration(
        fulldeclaration);

	auto result = ScriptExecutor::Get()->RunSetUp(Scripting.get(), &setup);

	existed = setup.ScriptExisted;

	return result;
}
// ------------------ Script proxies ------------------ //






