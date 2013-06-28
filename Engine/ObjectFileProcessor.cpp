#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_OBJECTFILEPROCESSOR
#include "ObjectFileProcessor.h"
#endif
#include "FileSystem.h"
using namespace Leviathan;
// ------------------------------------ //


ObjectFileProcessor::ObjectFileProcessor(){}

vector<IntWstring*> ObjectFileProcessor::ObjectTypes = vector<IntWstring*>();
// ------------------------------------ //
void Leviathan::ObjectFileProcessor::Initialize(){
	// register basic types //

}
void Leviathan::ObjectFileProcessor::Release(){
	// release memory //
	SAFE_DELETE_VECTOR(ObjectTypes);

	SAFE_DELETE_VECTOR(RegisteredValues);
}
void Leviathan::ObjectFileProcessor::RegisterObjectType(wstring name, int value){
	ObjectTypes.push_back(new IntWstring(name, value));
}
int Leviathan::ObjectFileProcessor::GetObjectTypeID(wstring &name){
	for(unsigned int i = 0; i < ObjectTypes.size(); i++){
		if(*ObjectTypes[i]->Wstr == name)
			return ObjectTypes[i]->Value;
	}

	return -1;
}
DLLEXPORT  void Leviathan::ObjectFileProcessor::RegisterValue(const wstring &signature, int value){
	RegisteredValues.push_back(new IntWstring(signature, value));
}
// ------------------------------------ //
DLLEXPORT vector<shared_ptr<ObjectFileObject>> Leviathan::ObjectFileProcessor::ProcessObjectFile(const wstring &file, vector<shared_ptr<NamedVar>> &HeaderVars){
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
			shared_ptr<NamedVar> namevar(new NamedVar(Lines[Line], &RegisteredValues));
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

				unique_ptr<ScriptScript> inlscript(new ScriptScript());
				// set variables //
				inlscript->Name = L"inl: "+file+L" line: "+Convert::IntToWstring(Line);
				inlscript->Instructions = L"void Do(int Line){\n"+scriptinstructions+L"\nreturn;\n}";
				inlscript->Source = L"inline on file: "+file+L" on line: "+Convert::IntToWstring(Line);

				vector<shared_ptr<NamedVariableBlock>> Args;
				Args.push_back(shared_ptr<NamedVariableBlock>(new NamedVariableBlock(new IntBlock(Line), L"FileLine")));

				ScriptInterface::Get()->ExecuteScript(inlscript.get(), L"void Do(int Line)", Args, true);

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

	vector<shared_ptr<wstring>> Prefixes;

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
			Name = *itr.GetStringInQuotes(QUOTETYPE_BOTH, true).get();
			continue;
		}
		// if no type, must be it //
		if(TypeN.size() == 0){
			TypeN = lineparts[i];
			continue;
		}

		// just a prefix //
		Prefixes.push_back(shared_ptr<wstring>(new wstring(lineparts[i])));
	}
	if(Name.size() == 0){

		Logger::Get()->Error(L"ScriptInterface: ReadObjectBlock: object doesn't have a name! prefixes "+ Misc::WstringStitchTogether(Prefixes, L" , ")
			+L" line ", Line, true);
		return NULL;
	}

	obj = shared_ptr<ObjectFileObject>(new ObjectFileObject(Name, GetObjectTypeID(TypeN), TypeN));
	obj->Prefixes = Prefixes;

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
			//case 2:
			//	{
			//		DEBUG_BREAK;
			//		if(ProcessObjectFileBlockVariableBlock(Line, Lines, sourcefile, Level, obj, Handleindex, itr)){
			//			// block ended //
			//			Something = 0;
			//		}
			//	}
			//break;
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
		obj->Contents[Handleindex]->Variables->GetVec()->push_back(shared_ptr<NamedVar>(new NamedVar(Lines[Line], &RegisteredValues)));
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
	ScriptScript* tscript = new ScriptScript();
	tscript->Instructions = L"";
	tscript->Source = sourcefile+L":OBJ:"+obj->Name;

	wstring ScriptType;
	int CodeStartLine = 0;

	IntendLevel = 0;

	// go back 1 line before going back to the line we are currently on //
	Line -= 1;

	bool Incode = false;
	while(++Line < (int)Lines.size() && Working){
		// skip empty lines //
		if(Lines[Line].size() == 0){
			// scripts should have proper line numbers inside them //
			if(Incode){
				// add empty line to have this work, but outside files should also be supported //
				tscript->Instructions += L"\n";
			}
			continue;
		}

		if(Incode){
			if(Lines[Line] == L"@%};"){
				Incode = false;
				IntendLevel = 0;
				continue;
			}
			// add to script //
			tscript->Instructions += Lines[Line]+L"\n";
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
							// type specification
							// split token //
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
						NamedVar tmpnamedvar(Lines[Line]);

						// get variable value to name //
						tmpnamedvar.GetValue(tscript->Name);
					}
					catch(const ExceptionInvalidArguement &e){
						// invalid definition //
						Logger::Get()->Error(L"ScriptInterface: ReadObjectBlock: script definition invalid name line: "+Convert::IntToWstring(Line)+
							L" in file"+sourcefile+L" see exception: ", true);
						e.PrintToLog();
						tscript->Name = L"Invalid name";
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
		// darn //
		// no end for script body end //
		Logger::Get()->Error(L"ObjectFileProcessor: ProcessObjectFileBlockScriptBlock: script block script body has leaked, no ending \"@%};\" "
			L"was found, began on line "+Convert::IntToWstring(CodeStartLine), true);
		SAFE_DELETE(tscript);
		return true;
	}

	// set script to object //
	obj->Script = shared_ptr<ScriptScript>(tscript);

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

DLLEXPORT  int Leviathan::ObjectFileProcessor::WriteObjectFile(vector<shared_ptr<ObjectFileObject>> &objects, const wstring &file, vector<shared_ptr<NamedVar>> &headervars,bool UseBinary /*= false*/){
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
			vector<shared_ptr<NamedVar>>* tempvals = tmp->Variables->GetVec();
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
			ScriptScript* scrpt = temp->Script.get();

			// start //
			writer << L"		s scripts {" << endl;
			writer << L"			inl type: \"script\" {" << endl;
			writer << L"				name = " << scrpt->Name << L";" << endl;
			writer << L"				body {" << endl;
			// write instructions //
			vector<wstring> SplitInstrLines;
			Misc::CutWstring(scrpt->Instructions, L"\n", SplitInstrLines);
			for(unsigned int e = 0; e < SplitInstrLines.size(); e++){
				// check for adding spaces //
				if(SplitInstrLines.size() > 10){
					// no spaces //
					// just the instruction //
					writer << SplitInstrLines[e] << endl;

					continue;
				}
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



vector<IntWstring*> Leviathan::ObjectFileProcessor::RegisteredValues;
