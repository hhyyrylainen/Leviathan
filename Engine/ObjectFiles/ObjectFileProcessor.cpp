#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_OBJECTFILEPROCESSOR
#include "ObjectFileProcessor.h"
#endif
#include "FileSystem.h"
#include <boost\assign\list_of.hpp>
#include "Common\DataStoring\DataStore.h"
using namespace Leviathan;
// ------------------------------------ //

ObjectFileProcessor::ObjectFileProcessor(){}

Leviathan::ObjectFileProcessor::~ObjectFileProcessor(){}

// quick macro to make this shorter //
#define ADDDATANAMEINTDEFINITION(x) (L#x , new VariableBlock(new IntBlock(x)))

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

}
void Leviathan::ObjectFileProcessor::Release(){
	// release memory //

	RegisteredValues.clear();
}

DLLEXPORT  void Leviathan::ObjectFileProcessor::RegisterValue(const wstring &name, VariableBlock* valuetokeep){
	RegisteredValues[name] = shared_ptr<VariableBlock>(valuetokeep);
}
// ------------------------------------ //
DLLEXPORT vector<shared_ptr<ObjectFileObject>> Leviathan::ObjectFileProcessor::ProcessObjectFile(const wstring &file, vector<shared_ptr<NamedVariableList>> &HeaderVars){
	//QUICKTIME_THISSCOPE;
	vector<shared_ptr<ObjectFileObject>> returned;

	// read the file entirely //
	wstring filecontents;

	try{
		FileSystem::ReadFileEntirely(file, filecontents);
	}
	catch (const ExceptionInvalidArguement &e){

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

	// TODO: REMOVE COMMENTS ALSO HERE ----------------------------------------------------------------------------------------------------------------
	for(unsigned int i = 0; i < Lines.size(); i++){
		//Misc::WstringRemovePreceedingTrailingSpaces(Lines[i]);
		WstringIterator::StripPreceedingAndTrailingWhitespaceComments(Lines[i]);
	}

	// set line //
	UINT Line = 0;
	
	for(;;){
		// check is still valid //
		if(Line >= (int)Lines.size()){
			// not valid, "file" ended //
			// can be valid //
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
#ifdef _DEBUG
			//Logger::Get()->Info(L"ObjectFileProcessor: Header ended because of line: "+Lines[Line]);
#endif // _DEBUG
			break;
		}

		Line++;
	}

	// read objects // // move to next line, on first iteration skips "objects {" part //
	while(++Line < (int)Lines.size() && (!Misc::WstringStartsWith(Lines[Line], L"-!-"))){
		
		// skip empty //
		if(Lines[Line].size() == 0){ 
			continue;
		}

		// test for possible objects/structs and stuff //
		wstring deftype;
		Misc::WstringGetFirstWord(Lines[Line], deftype);

		if(deftype == L"o"){
			// there's a object //
			shared_ptr<ObjectFileObject> objs = ReadObjectBlock(Line, Lines, file);
			if(objs.get() != NULL){

				returned.push_back(objs);
			} else {
				Logger::Get()->Error(L"FileLoader: ProcessObjectFile: ReadObjectBlock returned NULL object", true);
			}
			continue;
		}
		if(deftype == L"s"){
			// script related //

			wstring second;
			Misc::WstringGetSecondWord(Lines[Line], second);

			if(second == L"run:"){
				// run a script, like right now! //
				wstring scriptinstructions = Misc::WstringRemoveFirstWords(Lines[Line], 2);

				ScriptInterface* sinterface = ScriptInterface::Get();

				// create module for this script //
				weak_ptr<ScriptModule> inlscript(sinterface->GetExecutor()->CreateNewModule(L"inl: "+file+L" line: "+Convert::IntToWstring(Line),
					"inline on file: "+Convert::WstringToString(file)+" on line: "+Convert::ToString(Line)));

				shared_ptr<ScriptScript> tmpscrpptr = inlscript.lock()->GetScriptInstance();

				ScriptModule* tmpmodule = tmpscrpptr->GetModule();
				// add sections to the module //
				tmpmodule->GetBuilder().AddSectionFromMemory(("void Do(int Line){\n"+Convert::WstringToString(scriptinstructions)+"\nreturn;\n}"
					).c_str(), Convert::WstringToString(file+L":"+Convert::IntToWstring(Line)).c_str(), Line);

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
					sargs.SetEntrypoint("void Do(int Line)").SetArguements(Args).SetUseFullDeclaration(true);

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
	// monitoring //

	// this object's definition should be on the Line in Lines parameter //
	shared_ptr<ObjectFileObject> obj(NULL);

	// split definitions from first line //
	vector<wstring> lineparts;
	Misc::CutWstring(Lines[Line], L" ", lineparts);

	// used for error reporting //
	int StartLine = Line;

	wstring Name(L"");

	vector<wstring*> Prefixes;

	wstring TypeN(L"");

	WstringIterator itr(NULL, false);

	for(size_t i = 0; i < lineparts.size(); i++){
		if(lineparts[i].size() == 0){

			Logger::Get()->Warning(L"FileLoader: ReadObjectBlock: empty object prefix, prefixes/line: "+Lines[Line], false);
			continue;
		}
		if(lineparts[i].length() == 1){
			// check is it o //
			if(lineparts[i] == L"o"){
				// skip it //
				continue;
			}
		}
		if(lineparts[i][0] == L'{'){
			break;
		}
		if(lineparts[i][0] == L'"'){
			// must be the name //
			if(Name.size() != 0){
				Logger::Get()->Error(L"FileLoader: ReadObjectBlock: object has multiple names! "+Name);
				continue;
			}
			// get the text inside " marks //
			itr.ReInit(&lineparts[i], false);

			// wstring iterator can be used to skip all sorts of junk outside quotes //
			Name = *itr.GetStringInQuotes(QUOTETYPE_BOTH);
			continue;
		}
		// if no type, must be it //
		if(TypeN.size() == 0){
			TypeN = lineparts[i];
			continue;
		}

		// just a prefix //
		Prefixes.push_back(new wstring(lineparts[i]));
	}
	if(Name.size() == 0){
		// don't leak memory //
		SAFE_DELETE_VECTOR(Prefixes);

		Logger::Get()->Error(L"ScriptInterface: ReadObjectBlock: object doesn't have a name! prefixes "+ Misc::WstringStitchTogether(Prefixes, L" , ")
			+L" line ", Line, true);
		return NULL;
	}

	obj = shared_ptr<ObjectFileObject>(new ObjectFileObject(Name, TypeN));
	obj->Prefixes = Prefixes;
	// pointers have been copied to the object's vector //
	Prefixes.clear();

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
		
		shared_ptr<wstring> start = itr.GetNextCharacterSequence(UNNORMALCHARACTER_TYPE_LOWCODES_WHITESPACE);

		if(*start == L"l"){
			// data list //
			Something = 1;
			Level++;

			// handle first line of object //
			obj->Contents.push_back(new ObjectFileList(*itr.GetNextCharacterSequence(UNNORMALCHARACTER_TYPE_LOWCODES_WHITESPACE)));
			Handleindex = obj->Contents.size()-1;

			continue;
		}
		if((*start == L"t") || (*start == L"l<t>")){
			// text block //
			Something = 4;

			Level++;

			// handle first line of object //
			obj->TextBlocks.push_back(new ObjectFileTextBlock(*itr.GetNextCharacterSequence(UNNORMALCHARACTER_TYPE_LOWCODES_WHITESPACE)));
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
		Logger::Get()->Error(L"ScriptInterface: ReadObjectBlock: no matching bracket found in o "+Name+L" line ", StartLine, true);
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

	unique_ptr<wstring> linegot = itr.GetNextCharacterSequence(UNNORMALCHARACTER_TYPE_LOWCODES_WHITESPACE);

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
	catch (const ExceptionInvalidArguement &e){

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
				// check for script definition //
				if(Misc::WstringStartsWith(Lines[Line], L"inl")){
					// process script block definition//
					vector<wstring*> Tokens;
					// use token separator here //
					LineTokeNizer::TokeNizeLine(Lines[Line], Tokens);

					if(Tokens.size() == 0){
						// invalid //
						DEBUG_BREAK;
					}

					// first token is "inl" and can be skipped //

					// get script type from this line //
					for(size_t a = 1; a < Tokens.size(); a++){
						if(Misc::WstringStartsWith(*Tokens[a], L"type")){
							// type specification //
							vector<Token*> linetokens;

							LineTokeNizer::SplitTokenToRTokens(*Tokens[a], linetokens);

							// token size should be 2 //
							if(linetokens.size() != 2){
								DEBUG_BREAK;
								Logger::Get()->Error(L"ScriptInterface: ReadObjectBlock: inline has invalid type "+*Tokens[a]);
								continue;
							}
							// second token is script type name //
							ScriptType = linetokens[1]->GetChangeableData();
							SAFE_DELETE_VECTOR(linetokens);
							continue;
						}
						if(Misc::WstringStartsWith(*Tokens[a], L"{")){
							// can't be anything important after this //
							break;
						}
					}

					// release tokens //
					SAFE_DELETE_VECTOR(Tokens);

					// go to next level //
					IntendLevel++;
					continue;
				}

				// check for block end //
				if(Misc::WstringStartsWith(Lines[Line], L"}")){
					Working = false;
					//Logger::Get()->Error(L"ScriptInterface: ReadObjectBlock: Script body blob contained no definitions ");
					break;
				}
			}
		break;
		case 1:
			{
				if(Misc::WstringStartsWith(Lines[Line], L"name")){

					// this line should be a NamedVar object //
					try{
						// use NamedVar constructor to parse this line //

						// we first need to get just the value //
						WstringIterator itr(&Lines[Line], false);

						itr.GetUntilEqualityAssignment(EQUALITYCHARACTER_TYPE_ALL);
						// skip whitespace and we should be at right spot //
						itr.SkipWhiteSpace();

						unique_ptr<wstring> nameval = itr.GetUntilNextCharacterOrAll(L';');

						VariableBlock tmpnamedvar(*nameval, NULL);

						// get variable value to name //

						if(!tmpnamedvar.ConvertAndAssingToVariable<wstring>(Name)){

							Logger::Get()->Error(L"ScriptInterface: ReadObjectBlock: script definition invalid name line: "+Convert::IntToWstring(Line)+
								L" in file"+sourcefile+L". Cannot be cast to wstring!", true);
							Name = L"Invalid name";
						}
					}
					catch(const ExceptionInvalidArguement &e){
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

	// create script //
	weak_ptr<ScriptModule> tscript = ScriptInterface::Get()->GetExecutor()->CreateNewModule(Name, source);

	ScriptModule* tmpmod = tscript.lock().get();

	// "build" the script //
	tmpmod->GetBuilder().AddSectionFromMemory(Instructions.c_str(), Convert::WstringToString(sourcefile+L":"+Convert::ToWstring(CodeStartLine)
		).c_str(), CodeStartLine);

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

// ------------------------------------ //

DLLEXPORT  int Leviathan::ObjectFileProcessor::WriteObjectFile(vector<shared_ptr<ObjectFileObject>> &objects, const wstring &file, vector<shared_ptr<NamedVariableList>> &headervars,bool UseBinary /*= false*/){
	// open file for writing //
	wofstream writer;
	writer.open(file);
	if(!writer.is_open()){
		// write fail //
		return 11;
	}
	// start by writing in the header variables
	for(unsigned int i = 0; i < headervars.size(); i++){
		writer << headervars[i]->ToText(0) << endl;
	}
	// objects start //
	writer << L"objects {" << endl;

	for(unsigned int i = 0; i < objects.size(); i++){
		// put object data //
		ObjectFileObject* temp = objects[i].get();

		// starting line //
		if(temp->Prefixes.size() != 0){
			writer << L"	o " << temp->TName << L" " << Misc::VectorValuesToSingle<wstring>(temp->Prefixes, L" ", true) << L" \"" 
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






