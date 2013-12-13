#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GAMEMODULE
#include "GameModule.h"
#endif
#include "ObjectFiles/ObjectFileProcessor.h"
#include "FileSystem.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::GameModule::GameModule(const wstring &modulename, const wstring &ownername, const wstring &extension /*= L"txt|levgm"*/) : OwnerName(ownername), LoadedFromFile(modulename){
	// Find the actual file //
	wstring file = FileSystem::Get()->SearchForFile(FILEGROUP_SCRIPT, modulename, extension, false);

	if(file.size() == 0){
		// Couldn't find file //

		throw ExceptionInvalidArgument(L"File not found", 0, __WFUNCTION__, L"modulename", modulename);
	}

	// Load the file //
	std::vector<shared_ptr<NamedVariableList>> headervars;

	std::vector<shared_ptr<ObjectFileObject>> objects = ObjectFileProcessor::ProcessObjectFile(file, headervars);

	// Process the objects //
	if(objects.size() != 1){

		throw ExceptionInvalidArgument(L"File contains invalid number of objects, single GameModule expected", objects.size(), __WFUNCTION__, L"modulename", modulename);
	}

	// Get various data from the header //
	NamedVars tmpvars(headervars);


	ObjectFileProcessor::LoadValueFromNamedVars<wstring>(tmpvars, L"Version", Name, L"-1", true, L"GameModule:");

	Name = objects[0]->Name;

	// handle the single object //
	ObjectFileList* properties = NULL;
	ObjectFileTextBlock* sources = NULL;

	for(size_t i = 0; i < objects[0]->Contents.size(); i++){

		if(objects[0]->Contents[i]->Name == L"properties"){

			properties = objects[0]->Contents[i];
			break;
		}
	}

	for(size_t i = 0; i < objects[0]->TextBlocks.size(); i++){

		if(objects[0]->TextBlocks[i]->Name == L"sourcefiles"){

			sources = objects[0]->TextBlocks[i];
			break;
		}
	}

	if(!properties || !sources){

		throw ExceptionInvalidArgument(L"File contains invalid GameModule, properties or sourcefiles not found", 0, __WFUNCTION__, L"modulename", modulename);
	}

	// Copy data //
	if(sources->Lines.size() < 1){

		throw ExceptionInvalidArgument(L"At least one source file expected in sourcefiles", sources->Lines.size(), __WFUNCTION__, L"modulename", modulename);
	}

	wstring sourcefilename = FileSystem::RemoveExtension(*sources->Lines[0], true);
	wstring extensions = FileSystem::GetExtension(*sources->Lines[0]);

	SourceFile = FileSystem::Get()->SearchForFile(FILEGROUP_SCRIPT, sourcefilename, extensions, false);
}

DLLEXPORT Leviathan::GameModule::~GameModule(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::GameModule::Init(){
	// Compile a new module //
	ScriptModule* mod = NULL;

	if(!Scripting){

		Scripting = shared_ptr<ScriptScript>(new ScriptScript(ScriptInterface::Get()->GetExecutor()->CreateNewModule(
			L"GameModule("+Name+L") ScriptModule", Convert::WstringToString(SourceFile))));

		// Get the newly created module //
		mod = Scripting->GetModule();

		mod->GetBuilder().AddSectionFromFile(Convert::WstringToString(SourceFile).c_str());
		mod->SetBuildState(SCRIPTBUILDSTATE_READYTOBUILD);


	} else{
		// Get already created module //
		mod = Scripting->GetModule();
	}

	// Build the module (by creating a callback list) //
	std::vector<shared_ptr<ValidListenerData>> containedlisteners;

	mod->GetListOfListeners(containedlisteners);

	if(mod->GetModule() == NULL){
		// Fail to build //
		Logger::Get()->Error(L"GameModule: Init: failed to build AngelScript module");
		return false;
	}

	for(size_t i = 0; i < containedlisteners.size(); i++){
		// Bind generic event //
		if(containedlisteners[i]->GenericTypeName && containedlisteners[i]->GenericTypeName->size() > 0){

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

		Logger::Get()->Warning(L"GameModule: unknown event type "+*containedlisteners[i]->ListenerName+L", did you intent to use Generic type?");
	}

	// Call init callbacks //

	// fire an event //
	Event* tmpevent = new Event(EVENT_TYPE_INIT, NULL, false);

	OnEvent(&tmpevent);

	tmpevent->Release();
	return true;
}

DLLEXPORT void Leviathan::GameModule::ReleaseScript(){
	// Call release callback and destroy everything //
	// fire an event //
	Event* tmpevent = new Event(EVENT_TYPE_RELEASE, NULL, false);

	OnEvent(&tmpevent);

	tmpevent->Release();

	// Remove our reference //
	int tmpid = Scripting->GetModule()->GetID();
	Scripting.reset();

	ScriptInterface::Get()->GetExecutor()->DeleteModuleIfNoExternalReferences(tmpid);
}
// ------------------------------------ //
DLLEXPORT wstring Leviathan::GameModule::GetDescriptionForError(bool full /*= false*/){
	return L"GameModule("+Name+(full ? L" v"+Version+L") ": L") ")+L" owned by: "+OwnerName+(full ? L", loaded from file: "+LoadedFromFile+L".": L".");
}

DLLEXPORT string Leviathan::GameModule::GetDescriptionProxy(bool full){
    return GetDescriptionForError(full);
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
			vector<shared_ptr<NamedVariableBlock>> Args = boost::assign::list_of(new NamedVariableBlock(new VoidPtrBlock(this), L"GameModule"))
				(new NamedVariableBlock(new VoidPtrBlock(*pEvent), L"Event"));
			// we are returning ourselves so increase refcount
			AddRef();
			(*pEvent)->AddRef();

			ScriptRunningSetup sargs;
			sargs.SetEntrypoint(mod->GetListeningFunctionName(listenername)).SetArguments(Args);
			// run the script //
			shared_ptr<VariableBlock> result = ScriptInterface::Get()->ExecuteScript(Scripting.get(), &sargs);
			// do something with result //
		}
	} else {
		// generic event is passed //
		if(mod->DoesListenersContainSpecificListener(L"", (*event2)->TypeStr)){
			// setup parameters //
			vector<shared_ptr<NamedVariableBlock>> Args = boost::assign::list_of(new NamedVariableBlock(new VoidPtrBlock(this), L"GameModule"))
				(new NamedVariableBlock(new VoidPtrBlock(*event2), L"GenericEvent"));
			// we are returning ourselves so increase refcount
			AddRef();
			(*event2)->AddRef();

			ScriptRunningSetup sargs;
			sargs.SetEntrypoint(mod->GetListeningFunctionName(L"", (*event2)->TypeStr)).SetArguments(Args);
			// run the script //
			shared_ptr<VariableBlock> result = ScriptInterface::Get()->ExecuteScript(Scripting.get(), &sargs);
			// do something with result //
		}
	}
}
// ------------------ Being an actual module ------------------ //
DLLEXPORT shared_ptr<VariableBlock> Leviathan::GameModule::ExecuteOnModule(const string &entrypoint, std::vector<shared_ptr<NamedVariableBlock>>
	&otherparams, bool &existed, bool fulldeclaration /*= false*/)
{
	// Add this as parameter //
	otherparams.insert(otherparams.begin(), shared_ptr<NamedVariableBlock>(new NamedVariableBlock(new VoidPtrBlock(this), L"GameModule")));

	// we are returning ourselves so increase refcount
	AddRef();

	ScriptRunningSetup setup;
	setup.SetArguments(otherparams).SetEntrypoint(entrypoint).SetUseFullDeclaration(fulldeclaration);

	auto result = ScriptInterface::Get()->ExecuteScript(Scripting.get(), &setup);

	existed = setup.ScriptExisted;

	return result;
}
// ------------------ Script proxies ------------------ //
DLLEXPORT string Leviathan::GameModule::GetDescriptionProxy(bool full){

	return Convert::WstringToString(GetDescriptionForError(full));
}





