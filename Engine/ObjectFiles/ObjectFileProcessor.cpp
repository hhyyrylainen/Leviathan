#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_OBJECTFILEPROCESSOR
#include "ObjectFileProcessor.h"
#endif
#include "FileSystem.h"
#include <boost/assign/list_of.hpp>
#include "Common/DataStoring/DataStore.h"
using namespace Leviathan;
// ------------------------------------ //

ObjectFileProcessor::ObjectFileProcessor(){}

Leviathan::ObjectFileProcessor::~ObjectFileProcessor(){}

// quick macro to make this shorter //
#ifdef _MSC_VER
#define ADDDATANAMEINTDEFINITION(x) (WIDEN(#x), new VariableBlock(new IntBlock(x)))
#else
#define ADDDATANAMEINTDEFINITION(x) (WIDEN(#x), shared_ptr<VariableBlock>(new VariableBlock(new IntBlock(x))))
#endif

map<wstring, shared_ptr<VariableBlock>> Leviathan::ObjectFileProcessor::RegisteredValues = boost::assign::map_list_of
	ADDDATANAMEINTDEFINITION(DATAINDEX_TICKTIME)
	ADDDATANAMEINTDEFINITION(DATAINDEX_TICKCOUNT)
	ADDDATANAMEINTDEFINITION(DATAINDEX_FRAMETIME)
	ADDDATANAMEINTDEFINITION(DATAINDEX_FPS)
	ADDDATANAMEINTDEFINITION(DATAINDEX_WIDTH)
	ADDDATANAMEINTDEFINITION(DATAINDEX_HEIGHT)
	ADDDATANAMEINTDEFINITION(DATAINDEX_FRAMETIME_MIN)
	ADDDATANAMEINTDEFINITION(DATAINDEX_FRAMETIME_MAX)
	ADDDATANAMEINTDEFINITION(DATAINDEX_FRAMETIME_AVERAGE)
	ADDDATANAMEINTDEFINITION(DATAINDEX_FPS_AVERAGE)
	ADDDATANAMEINTDEFINITION(DATAINDEX_FPS_MIN)
	ADDDATANAMEINTDEFINITION(DATAINDEX_FPS_MAX)
	//ADDDATANAMEINTDEFINITION(QUAD_FILLSTYLE_UPPERLEFT_0_BOTTOMRIGHT_1)
	//ADDDATANAMEINTDEFINITION(QUAD_FILLSTYLE_UPPERLEFT_1_BOTTOMRIGHT_0)
	;

// ------------------------------------ //
void Leviathan::ObjectFileProcessor::Initialize(){
	// register basic types //

	// Just out of curiosity check this //
	auto iter = RegisteredValues.find(L"DATAINDEX_TICKTIME");

	if(iter == RegisteredValues.end()){

		Logger::Get()->Error(L"ObjectFileProcessor: RegisteredValues are messed up, DATAINDEX_TICKTIME is not defined, check the macros!");
		return;
	}

}
void Leviathan::ObjectFileProcessor::Release(){
	// release memory //

	RegisteredValues.clear();
}

DLLEXPORT  void Leviathan::ObjectFileProcessor::RegisterValue(const wstring &name, VariableBlock* valuetokeep){
	RegisteredValues[name] = shared_ptr<VariableBlock>(valuetokeep);
}
// ------------------------------------ //
DLLEXPORT vector<shared_ptr<ObjectFileObject>> Leviathan::ObjectFileProcessor::ProcessObjectFile(const wstring &file,
	vector<shared_ptr<NamedVariableList>> &HeaderVars)
{
	vector<shared_ptr<ObjectFileObject>> returned;

	// read the file entirely //
	wstring filecontents;

	try{
		FileSystem::ReadFileEntirely(file, filecontents);
	}
	catch(const ExceptionInvalidArgument &e){

		Logger::Get()->Error(L"ObjectFileProcessor: ProcessObjectFile: file could not be read, exception:");
		e.PrintToLog();
		return returned;
	}


	// file needs to be split to lines //
	vector<wstring> Lines;

	if(Misc::CutWstring(filecontents, L"\n", Lines)){
		// failed //
		DEBUG_BREAK;
	}

	// remove excess spaces //
	for(unsigned int i = 0; i < Lines.size(); i++){

		WstringIterator::StripPreceedingAndTrailingWhitespaceComments(Lines[i]);
	}

	// set line //
	UINT Line = 0;

	for(;;){
		// check is still valid //
		if(Line >= (int)Lines.size()){
			// not valid, "file" ended //
			break;
		}

		// skip empty lines //
		if(Lines[Line].size() == 0){
			Line++;
			continue;
		}
		// try to create a named var from this line //
		try{
			shared_ptr<NamedVariableList> namevar(new NamedVariableList(Lines[Line], &RegisteredValues));
			// didn't cause an exception, is valid add //
			HeaderVars.push_back(namevar);
		}
		catch(...){

			// end found //
			break;
		}
		Line++;
	}

	WstringIterator itr(NULL, false);

	// read objects // // move to next line, on first iteration skips "objects {" part //
	while(++Line < (int)Lines.size() && (!Misc::WstringStartsWith(Lines[Line], L"-!-"))){

		// skip empty //
		if(Lines[Line].size() == 0){
			continue;
		}

		itr.ReInit(&Lines[Line], false);

		// test for possible objects/structs and stuff //
		unique_ptr<wstring> deftype = itr.GetNextCharacterSequence(UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS | UNNORMALCHARACTER_TYPE_WHITESPACE);


		if(*deftype == L"o"){
			// there's a object //
			shared_ptr<ObjectFileObject> objs = ReadObjectBlock(Line, Lines, file);
			if(objs.get() != NULL){

				returned.push_back(objs);
			} else {
				Logger::Get()->Error(L"FileLoader: ProcessObjectFile: ReadObjectBlock returned NULL object", true);
			}
			continue;
		}
		if(*deftype == L"s"){
			// script related //
			deftype = itr.GetNextCharacterSequence(UNNORMALCHARACTER_TYPE_WHITESPACE);

			if(*deftype == L"run:"){
				// run a script, like right now! //
				unique_ptr<wstring> scriptinstructions = itr.GetUntilEnd();

				ScriptInterface* sinterface = ScriptInterface::Get();

				// create module for this script //
				weak_ptr<ScriptModule> inlscript(sinterface->GetExecutor()->CreateNewModule(L"inl: "+file+L" line: "+Convert::IntToWstring(Line),
					"inline on file: "+Convert::WstringToString(file)+" on line: "+Convert::ToString(Line)));

				shared_ptr<ScriptScript> tmpscrpptr = inlscript.lock()->GetScriptInstance();

				ScriptModule* tmpmodule = tmpscrpptr->GetModule();
				// add sections to the module //

				tmpmodule->GetBuilder().AddSectionFromMemory(Convert::WstringToString(file+L":"+Convert::IntToWstring(Line)).c_str(),
					("void Do(int Line){\n"+Convert::WstringToString(*scriptinstructions)+"\nreturn;\n}").c_str(), Line);

				// compile the script //
				int result = tmpmodule->GetBuilder().BuildModule();
				if(result < 0){
					// failed to compile //

					Logger::Get()->Error(L"ObjectFileProcessor: ProcessObjectFile: inline script failed to compile"+tmpmodule->GetInfoWstring());
					continue;

				} else {
					// set state to built //
					tmpmodule->SetBuildState(SCRIPTBUILDSTATE_BUILT);

					vector<shared_ptr<NamedVariableBlock>> Args = boost::assign::list_of(new NamedVariableBlock(new IntBlock(Line), L"FileLine"));

					ScriptRunningSetup sargs;
					sargs.SetEntrypoint("void Do(int Line)").SetArguments(Args).SetUseFullDeclaration(true);

					ScriptInterface::Get()->ExecuteScript(tmpscrpptr.get(), &sargs);
				}

				// release the module //
				tmpmodule->DeleteThisModule();

				continue;
			}

			continue;
		}
	}

	return returned;
}
shared_ptr<ObjectFileObject> Leviathan::ObjectFileProcessor::ReadObjectBlock(UINT &Line, vector<wstring> &Lines, const wstring& sourcefile){
	// this object's definition should be on the Line in Lines parameter //
	shared_ptr<ObjectFileObject> obj(NULL);

	// used for error reporting //
	int StartLine = Line;

	// split definitions from first line //
	WstringIterator itr(&Lines[Line], false);

	unique_ptr<wstring> str(nullptr);

	// cut ending {
	str = itr.GetUntilNextCharacterOrNothing(L'{');

	if(str->size() == 0){

		Logger::Get()->Error(L"ObjectFileProcessor: ReadObjectBlock: no starting '{' found on line "+Convert::ToWstring(Line));
		return NULL;
	}
	// re init to not have the brace (actually deleting the string after being done) //
	itr.ReInit(str.release(), true);


	unique_ptr<wstring> Name(nullptr);
	unique_ptr<wstring> TypeN(nullptr);

	vector<shared_ptr<wstring>> Prefixes;

	while((str = itr.GetNextCharacterSequence(UNNORMALCHARACTER_TYPE_WHITESPACE))->size() > 0){

		if(str->length() == 1){
			if(*str == L"o"){
				// skip it //
				continue;
			}
		}

		if(str->find_first_of(L'"') == 0){
						// wstring iterator can be used to skip all sorts of junk outside quotes //
			WstringIterator quoteget(str.get(), false);

			// if name is already found this is a prefix //
			if(Name.get() != NULL){

				Prefixes.push_back(shared_ptr<wstring>(quoteget.GetStringInQuotes(QUOTETYPE_BOTH).release()));
			} else {
				Name = quoteget.GetStringInQuotes(QUOTETYPE_BOTH);
			}
			continue;
		}
		// if no type, must be it //
		if(TypeN.get() == NULL){
			TypeN = unique_ptr<wstring>(str.release());
			continue;
		}

		// just a prefix //
		Prefixes.push_back(shared_ptr<wstring>(str.release()));
	}
	if(Name.get() == NULL || Name->size() == 0){

		Logger::Get()->Error(L"ScriptInterface: ReadObjectBlock: object doesn't have a name! prefixes "+ Misc::WstringStitchTogether(Prefixes, L" , ")
			+L" line ", Line, true);
		return NULL;
	}

	obj = shared_ptr<ObjectFileObject>(new ObjectFileObject(*Name, *TypeN, Prefixes));

	// process blocks contents //
	int Level = 0, Something = 0, Handleindex = 0;

	while((Level > -1) && (++Line < (int)Lines.size())){
		// skip empty lines //
		if(Lines[Line].size() == 0){
			continue;
		}

		// check is this inside block //
		if(Something != 0){
			// handle the block //
			switch(Something){
			case 1:
				{
					// list //
					if(ProcessObjectFileBlockListBlock(Line, Lines, sourcefile, Level, obj, Handleindex, itr)){
						// block ended //
						Something = 0;
						Level--;
					}
				}
			break;
			case 3:
				{
					if(ProcessObjectFileBlockScriptBlock(Line, Lines, sourcefile, Level, obj, Handleindex, itr)){
						// block ended //
						Something = 0;
						Level--;
					}
				}
			break;
			case 4:
				{
					// text block //
					if(ProcessObjectFileBlockTextBlock(Line, Lines, sourcefile, Level, obj, Handleindex, itr)){
						// block ended //
						Something = 0;
						Level--;
					}
				}
			break;
			default:
				// won't be hit //
				__assume(0);
			}

			continue;
		}

		if(Misc::WstringStartsWith(Lines[Line], L"}")){
			// object ended //
			Level--;
			// could as well be break here //
			continue;
		}

		// update iterator //
		itr.ReInit(&Lines[Line], false);

		shared_ptr<wstring> start = itr.GetNextCharacterSequence(UNNORMALCHARACTER_TYPE_LOWCODES | UNNORMALCHARACTER_TYPE_WHITESPACE);

		if(*start == L"l"){
			// data list //
			Something = 1;
			Level++;

			// handle first line of object //
			obj->Contents.push_back(new ObjectFileList(*itr.GetNextCharacterSequence(UNNORMALCHARACTER_TYPE_LOWCODES
				| UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS)));
			Handleindex = obj->Contents.size()-1;

			continue;
		}
		if((*start == L"t") || (*start == L"l<t>")){
			// text block //
			Something = 4;

			Level++;

			// handle first line of object //
			obj->TextBlocks.push_back(new ObjectFileTextBlock(*itr.GetNextCharacterSequence(UNNORMALCHARACTER_TYPE_LOWCODES
				| UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS)));
			Handleindex = obj->TextBlocks.size()-1;

			continue;
		}
		if(*start == L"s"){
			Level++;

			Something = 3;
			continue;
		}
	}

	if(Level != -1){
		Logger::Get()->Error(L"ScriptInterface: ReadObjectBlock: no matching bracket found in o "+*Name+L" line ", StartLine, true);
	}

	// returning smart pointer //
	return obj;
}
// ------------------------------------ //
bool Leviathan::ObjectFileProcessor::ProcessObjectFileBlockListBlock(UINT &Line, vector<wstring> &Lines, const wstring& sourcefile, int &Level,
	shared_ptr<ObjectFileObject> obj, int &Handleindex, WstringIterator &itr)
{
	// update iterator //
	itr.ReInit(&Lines[Line], false);

	unique_ptr<wstring> linegot = itr.GetNextCharacterSequence(UNNORMALCHARACTER_TYPE_LOWCODES | UNNORMALCHARACTER_TYPE_WHITESPACE);

	// check for end //
	if(*linegot == L"}"){
		// object ended //
		//Level--; // no need to change level here

		// ended //
		return true;
	}
	// if begins with <t> is plain text //
	if(*linegot == L"<t>"){
		// store plain text //

		linegot = itr.GetUntilEnd();

		if(linegot->back() == L';'){

			linegot->erase(linegot->begin()+linegot->size()-1);
		}

		// remember to release it or it will delete //
		wstring* tmpptr = linegot.release();

		obj->Contents[Handleindex]->Lines.push_back(tmpptr);
		// don't delete tmpptr or bad things will happen //
		return false;
	}

	// parse variable //

	try{
		obj->Contents[Handleindex]->Variables.GetVec()->push_back(shared_ptr<NamedVariableList>(new NamedVariableList(Lines[Line], &RegisteredValues)));
	}
	catch (const ExceptionInvalidArgument &e){

		Logger::Get()->Error(L"ObjectFileProcessor: ProcessObjectFileBlockListBlock: invalid variable on line "+Convert::IntToWstring(Line)+
			L" caused an exception:", false);
		e.PrintToLog();

		return false;
	}
	return false;
}

bool Leviathan::ObjectFileProcessor::ProcessObjectFileBlockScriptBlock(UINT &Line, vector<wstring> &Lines, const wstring& sourcefile, int &Level,
	shared_ptr<ObjectFileObject> obj, int &Handleindex, WstringIterator &itr)
{
	bool Working = true;

	int IntendLevel = 0;

	// create the script here so that stuff can be added to it //
	string Instructions("");
	wstring Name(L"");
	string source = Convert::WstringToString(sourcefile+L":OBJ:"+obj->Name);

	int CodeStartLine = 0;
	wstring ScriptType(L"");

	IntendLevel = 0;

	// go back 1 line before going back to the line we are currently on //
	Line -= 1;

	bool Incode = false;
	while(++Line < (int)Lines.size() && Working){
		// if we are inside code add lines to the code body //
		if(Incode){
			if(Lines[Line] == L"@%};"){
				Incode = false;
				IntendLevel = 0;
				continue;
			}
			// add to script //
			Instructions += Convert::WstringToString(Lines[Line]+L"\n");
			continue;
		}
		switch(IntendLevel){
		case 0:
			{
				if(Misc::WstringStartsWith(Lines[Line], L"name")){

					// this line should be a NamedVar object //
					try{
						// use NamedVar constructor to parse this line //

						// we first need to get just the value //
						WstringIterator nameitr(&Lines[Line], false);

						nameitr.GetUntilEqualityAssignment(EQUALITYCHARACTER_TYPE_ALL);
						// skip whitespace and we should be at right spot //
						nameitr.SkipWhiteSpace();

						unique_ptr<wstring> nameval = nameitr.GetUntilNextCharacterOrAll(L';');

						VariableBlock tmpnamedvar(*nameval, NULL);

						// get variable value to name //

						if(!tmpnamedvar.ConvertAndAssingToVariable<wstring>(Name)){

							Logger::Get()->Error(L"ScriptInterface: ReadObjectBlock: script definition invalid name line: "+Convert::IntToWstring(Line)+
								L" in file"+sourcefile+L". Cannot be cast to wstring!", true);
							Name = L"Invalid name";
						}
					}
					catch(const ExceptionInvalidArgument &e){
						// invalid definition //
						Logger::Get()->Error(L"ScriptInterface: ReadObjectBlock: script definition invalid name line: "+Convert::IntToWstring(Line)+
							L" in file"+sourcefile+L" see exception: ", true);
						e.PrintToLog();
						Name = L"Invalid name";
					}

					continue;
				}

				if(Misc::WstringStartsWith(Lines[Line], L"body")){
					IntendLevel = 2;
					Incode = true;
					CodeStartLine = Line+1;
					continue;
				}

				if(Misc::WstringStartsWith(Lines[Line], L"}")){
					// go back to level 0 for processing lowest level end //
					IntendLevel = 0;
					continue;
				}
				// check for script definition //
				if(Misc::WstringStartsWith(Lines[Line], L"inl")){
					// This is deprecated now //
					Logger::Get()->Warning(L"ObjectFileProcessor: file has old s scripts block format!, file: "+sourcefile);
					continue;
				}
				DEBUG_BREAK;
			}
		break;
		}
	}

	if(Incode){
		// darn, no end for script body end //
		Logger::Get()->Error(L"ObjectFileProcessor: ProcessObjectFileBlockScriptBlock: script block script body has leaked, no ending \"@%};\" "
			L"was found, began on line "+Convert::IntToWstring(CodeStartLine), true);

		return true;
	}

	// We can create a generic name if one wasn't provided //
	if(Name.size() == 0){

		Name = L"Script for object "+obj->Name;
	}

	// create script //
	weak_ptr<ScriptModule> tscript = ScriptInterface::Get()->GetExecutor()->CreateNewModule(Name, source);

	ScriptModule* tmpmod = tscript.lock().get();

	// "build" the script //
	tmpmod->GetBuilder().AddSectionFromMemory(Convert::WstringToString(sourcefile+L":"+Convert::ToWstring(CodeStartLine)).c_str(),
		Instructions.c_str(), CodeStartLine);

	// ensure right state //
	tmpmod->SetBuildState(SCRIPTBUILDSTATE_READYTOBUILD);

	// set script to object //
	obj->Script = tscript.lock()->GetScriptInstance();

	// always fully processed //
	return true;
}

bool Leviathan::ObjectFileProcessor::ProcessObjectFileBlockTextBlock(UINT &Line, vector<wstring> &Lines, const wstring& sourcefile, int &Level,
	shared_ptr<ObjectFileObject> obj, int &Handleindex, WstringIterator &itr)
{
	// check for end //
	if(Misc::WstringStartsWith(Lines[Line], L"}")){
		// object ended //
		//Level--; // no need to change here //

		// ended //
		return true;
	}
	// it is always text, just add a new line //

	// store plain text //
	obj->TextBlocks[Handleindex]->Lines.push_back(new wstring(Lines[Line]));

	return false;
}
// ------------------------------------ //
DLLEXPORT  int Leviathan::ObjectFileProcessor::WriteObjectFile(vector<shared_ptr<ObjectFileObject>> &objects, const wstring &file, vector<shared_ptr<NamedVariableList>> &headervars,bool UseBinary /*= false*/){
	// open file for writing //
	wofstream writer;
#ifdef _WIN32
	writer.open(file);
#else
	writer.open(Convert::WstringToString(file));
#endif
	if(!writer.is_open()){
		// write fail //
		return 11;
	}
	// start by writing in the header variables
	for(unsigned int i = 0; i < headervars.size(); i++){
		writer << headervars[i]->ToText(0) << endl;
	}

	// Quit if no objects //
	if(objects.size() < 1)
		return true;

	// objects start //
	writer << L"objects {" << endl;

	for(unsigned int i = 0; i < objects.size(); i++){
		// put object data //
		ObjectFileObject* temp = objects[i].get();

		// starting line //
		if(temp->Prefixes.size() != 0){
			writer << L"	o " << temp->TName << L" " << Misc::VectorValuesToSingleSmartPTR<wstring>(temp->Prefixes, L" ", true) << L" \""
				<< temp->Name << L"\" {" << endl;
		} else {
			writer << L"	o " << temp->TName << L" \"" << temp->Name << L"\" {" << endl;
		}

		// contents //
		for(unsigned int a = 0;  a < temp->Contents.size(); a++){
			ObjectFileList* tmp = temp->Contents[a];

			// start //
			writer << L"		l " << tmp->Name << L" {" << endl;
			// add text lines first //
			for(unsigned int ind = 0; ind < tmp->Lines.size(); ind++){
				writer << L"		<t> " << *tmp->Lines[ind] << endl;
			}
			// variable lines //
			vector<shared_ptr<NamedVariableList>>* tempvals = tmp->Variables.GetVec();
			for(unsigned int ind = 0; ind < tempvals->size(); ind++){
				writer << L"			" << tempvals->at(ind)->ToText(0) << endl;
			}
			// end //
			writer << L"		}" << endl;
		}
		// plain text lines //
		for(unsigned int a = 0;  a < temp->TextBlocks.size(); a++){
			ObjectFileTextBlock* tmp = temp->TextBlocks[a];

			if(tmp->Name.size() == 0){
				// invalid block, don't save //
				Logger::Get()->Info(L"ObjectFileProcessor: WriteObjectFile: invalid textblock (no name) skipping...", false);
				continue;
			}

			// start //
			writer << L"		t " << tmp->Name << L" {" << endl;
			// add text lines //
			for(unsigned int ind = 0; ind < tmp->Lines.size(); ind++){
				writer << L"            " << *tmp->Lines[ind] << endl;
			}
			// end //
			writer << L"		}" << endl;
		}
		// script block //
		if(temp->Script != NULL){
			ScriptModule* scrpt = temp->Script.get()->GetModule();

			// start //
			writer << L"		s scripts {" << endl;
			writer << L"			inl type: \"script\" {" << endl;
			writer << L"				name = " << scrpt->GetName() << L";" << endl;
			writer << L"				body {" << endl;
			// write instructions //
			vector<wstring> SplitInstrLines;
			Misc::CutWstring(Convert::StringToWstring(scrpt->GetIncompleteSourceCode()), L"\n", SplitInstrLines);
			for(unsigned int e = 0; e < SplitInstrLines.size(); e++){
				// write to file //
				writer << L"					" << SplitInstrLines[i] << endl;
			}
			// body end //
			writer << L"				@%};" << endl;
			writer << L"			}" << endl;
			// end //
			writer << L"		}" << endl;
		}

		// ending bracket //
		writer << L"	}" << endl;
	}

	// end //
	writer << L"}"/* << endl*/;
	return true;
}






