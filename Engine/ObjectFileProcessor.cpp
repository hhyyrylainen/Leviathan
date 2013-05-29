#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_OBJECTFILEPROCESSOR
#include "ObjectFileProcessor.h"
#endif
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
	QUICKTIME_THISSCOPE;
	vector<shared_ptr<ObjectFileObject>> returned;

	wifstream reader;
	reader.open(file);

	if(!reader.good()){

		Logger::Get()->Error(L"FileLoader: ProcessObjectFile 404, file not found", true);
		return returned;
	}

	// read through heading stuff //
	wchar_t Read = L' ';
	while(Read == L' '){
		reader.read(&Read, 1);
	}

	// gather values //
	wstring readline = L"";
	wchar_t Buff[400];

	int Line = 0;

	// last Read character needs to be added to header //
	wstring header(&Read, 1);

	bool first = true;

	// get first line //
	reader.getline(Buff, 400);
	Line++;
	readline = Buff;


	while((Misc::CountOccuranceWstring(readline, L"objects") == 0) && (reader.good())){
		if(!first){
			if(readline.size() != 0)
				header += L"\n" + readline;
		} else {
			header += readline;
			first = false;
		}

		reader.getline(Buff, 400);
		Line++;
		readline = Buff;
	}
	

	NamedVar::ProcessDataDump(header, HeaderVars);

	readline = L"!";

	// read objects //
	while((reader.good()) && (!Misc::WstringStartsWith(readline, L"-!-"))){
		reader.getline(Buff, 400);
		Line++;
		wstring readline(Buff);
		
		Misc::WstringRemovePreceedingTrailingSpaces(readline);

		// skip empty //


		if(readline.size() < 2) // can't really be anything
			continue;

		// test for possible objects/structs and stuff //
		wstring deftype;
		Misc::WstringGetFirstWord(readline, deftype);

		if(deftype == L"o"){
			// there's a object //
			shared_ptr<ObjectFileObject> objs = ReadObjectBlock(reader, readline, Line, file);
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
			Misc::WstringGetSecondWord(readline, second);

			if(second == L"run:"){
				// run a script, like right now! //
				wstring scriptinstructions = Misc::WstringRemoveFirstWords(readline, 2);

				unique_ptr<ScriptScript> inlscript(new ScriptScript());
				// set variables //
				inlscript->Name = L"inl: "+file+L" line: "+Convert::IntToWstring(Line);
				inlscript->Instructions = L"void Do(int Line){\n"+scriptinstructions+L"\nreturn;\n}";
				inlscript->Source = L"inline on file: "+file+L" on line: "+Convert::IntToWstring(Line);

				shared_ptr<ScriptVariableHolder> varhold(new ScriptVariableHolder());
				//varhold->Vars.push_back(new ScriptNamedArguement(L"FileName", new WstringBlock(file), DATABLOCK_TYPE_WSTRING, false, true));
				//varhold->Vars.push_back(new ScriptNamedArguement(L"Line", new IntBlock(Line), DATABLOCK_TYPE_INT, false, true));
				vector<shared_ptr<ScriptVariableHolder>> vars;
				vars.push_back(varhold);

				vector<ScriptNamedArguement*> Args;
				Args.push_back(new ScriptNamedArguement(L"FileLine", new IntBlock(Line), DATABLOCK_TYPE_INT, false, true));

				ScriptInterface::Get()->ExecuteScript(inlscript.get(),vars, L"void Do(int Line)", Args, NULL, true);


				SAFE_DELETE_VECTOR(Args);
				// delete allocated memory //
				vars.clear();
				varhold.reset();

				continue;
			}

			continue;
		}
	}

	reader.close();
	return returned;
}
shared_ptr<ObjectFileObject> Leviathan::ObjectFileProcessor::ReadObjectBlock(wifstream &reader, wstring firstline, int &Line, const wstring& sourcefile){
	// monitoring //

	// this object's definition should be in firstline parameter //
	shared_ptr<ObjectFileObject> obj(NULL);

	// split definitions from first line //
	vector<wstring> lines;
	Misc::CutWstring(firstline, L" ", lines);

	// used for error reporting //
	int StartLine = Line;

	wstring Name = L"ERROR-NAME";
	vector<shared_ptr<wstring>> Prefixes;

	wstring TypeN = L"ETYPE";

	for(unsigned int i = 0; i < lines.size(); i++){
		if(lines[i].size() == 0){

			Logger::Get()->Info(L"FileLoader: ReadObjectBlock: empty object prefix, prefixes/line: "+firstline);
			continue;
		}
		if(lines[i].length() == 1){
			// check is it o //
			if(lines[i] == L"o"){
				// skip it //
				continue;
			}
		}
		if(lines[i][0] == L'{'){
			break;
		}
		if(lines[i][0] == L'"'){
			// must be the name //
			if(Name != L"ERROR-NAME"){
				Logger::Get()->Error(L"FileLoader: ReadObjectBlock: object has multiple names! "+Name);
				continue;
			}
			// remove " marks //
			Name = Misc::Replace(lines[i], L"\"", L"");
			continue;
		}
		// if no type, must be it //
		if(TypeN == L"ETYPE"){
			TypeN = lines[i];
			continue;
		}

		// just a prefix //
		Prefixes.push_back(shared_ptr<wstring>(new wstring(lines[i])));
	}
	if(Name == L"ERROR-NAME"){

		Logger::Get()->Error(L"ScriptInterface: ReadObjectBlock: object doesn't have a name! prefixes "+ Misc::WstringStitchTogether(Prefixes, L" , ")
			+L" line ", Line, true);
		return NULL;
	}

	obj = shared_ptr<ObjectFileObject>(new ObjectFileObject(Name, GetObjectTypeID(TypeN), TypeN));
	obj->Prefixes = Prefixes;

	// load it's contents //
	wchar_t Buff[400];
	wstring currentline = L"!";

	int Level = 0;

	bool Insomething = false;
	bool Read = true;
	int Something = -1;

	wstring SpesLines = L"";

	int Handleindex = 0;

	while((Level > -1) && (reader.good())){
		if(Read){
			reader.getline(Buff, 400);
			Line++;
			currentline = Buff;

			if(currentline.size() < 1)
				continue;

			// skip empty //
			Misc::WstringRemovePreceedingTrailingSpaces(currentline);

			// skip comment lines //
			if(Misc::WstringStartsWith(currentline, L"//"))
				continue;

		}

		if(Insomething){

			switch(Something){
			case 1:
				{
					// list //
					if(Misc::WstringStartsWith(currentline, L"}")){
						// object ended //
						Insomething = false;
						Read = true;
						Something = 0;
						Level--;

						// parse variables //
						if(SpesLines.size() > 0){
							vector<shared_ptr<NamedVar>> HeaderVars;

							NamedVar::ProcessDataDump(SpesLines, HeaderVars, &RegisteredValues);

							vector<shared_ptr<NamedVar>>* currobjs = obj->Contents[Handleindex]->Variables->GetVec();
							for(unsigned int i = 0; i < HeaderVars.size(); i++){
								currobjs->push_back(HeaderVars[i]);
							}
							HeaderVars.clear();
						}

						SpesLines = L"";

						continue;
					}					
					// if begins with <t> is plain text //
					if(Misc::WstringStartsWith(currentline, L"<t>")){
						// store plain text //
						wstring line = Misc::WstringRemoveFirstWords(currentline, 1);
						if(line[line.size()-1] == L';'){
							line.erase(line.begin()+line.size()-1);
						}
						obj->Contents[Handleindex]->Lines.push_back(new wstring(line));
						continue;
					}

					// parse variable //
					SpesLines += L"\n"+currentline;

				}
				break;
			case 2:
				{
					// script variable blob //
					if(Misc::WstringStartsWith(currentline, L"}")){
						// object ended //
						Insomething = false;
						Read = true;
						Something = 0;
						Level--;

						SpesLines = L"";

						continue;
					}	

					if(Misc::WstringStartsWith(currentline, L"var")){
						// proper variable //

						// split words //
						vector<wstring> Words;
						Misc::CutWstring(Misc::WstringRemoveFirstWords(currentline, 1), L" ", Words);

						int stype = 0;
						wstring varname = L"";
						DataBlock* value = NULL;

						bool invalue = false;;

						for(unsigned int i = 0; i < Words.size(); i++){
							if(Words[i].size() < 1){
								Words.erase(Words.begin()+i);
								i--;
							}

							if(i == 0){
								// type //
								if(Misc::WstringCompareInsensitive(Words[i], L"int")){
									stype = DATABLOCK_TYPE_INT;
									continue;
								}
								if(Misc::WstringCompareInsensitive(Words[i], L"float")){
									stype = DATABLOCK_TYPE_FLOAT;
									continue;
								}
								if(Misc::WstringCompareInsensitive(Words[i], L"bool")){
									stype = DATABLOCK_TYPE_BOOL;
									continue;
								}
								if(Misc::WstringCompareInsensitive(Words[i], L"wstring")){
									stype = DATABLOCK_TYPE_WSTRING;
									continue;
								}
								if(Misc::WstringCompareInsensitive(Words[i], L"void")){
									stype = DATABLOCK_TYPE_VOIDPTR;
									continue;
								}
							}
							if(!invalue){
								// check for name //
								if(Words[i][0] == L'"'){
									// name //

									Words[i] = Misc::Replace(Words[i], L"\"", L"");

									if(Misc::CountOccuranceWstring(Words[i], L";") > 0){
										// end //

										Words[i] = Misc::Replace(Words[i], L";", L"");
										varname = Words[i];

										break;
									}

									varname = Words[i];

								}
								if(Words[i][0] == L'='){
									invalue = true;
								}
								continue;
							}

							// check for string //
							if(Words[i][0] == L'"'){

								if(stype != DATABLOCK_TYPE_WSTRING){

									Logger::Get()->Info(L"ScriptInterface: ReadObjectBlock: s variables block cannot assign string to other type, converted!", false);
									stype = DATABLOCK_TYPE_WSTRING;
								}

								Words[i].erase(Words[i].begin());

								// stitch rest of words together //
								wstring thing;
								for(unsigned int a = i; a < Words.size(); a++){
									if(a != i)
										thing += L" ";
									thing += Words[a];
								}

								// remove end row and " //
								for(int a = thing.size()-1; a > -1; a--){
									if(thing[a] == L';'){
										thing.erase(thing.begin()+a);
										continue;
									}
									if(thing[a] == L'"'){
										thing.erase(thing.begin()+a);
										break;
									}
								}

								value = new WstringBlock(thing);

								break;
							}
							// remove ; from messing with assign //
							Words[i] = Misc::Replace(Words[i], L";", L" ");

							// parse based on type //
							if(stype == DATABLOCK_TYPE_INT){

								value = new IntBlock(Convert::WstringToInt(Words[i]));
								break;
							}
							if(stype == DATABLOCK_TYPE_FLOAT){

								value = new FloatBlock(Convert::WstringToFloat(Words[i]));
								break;
							}
							if(stype == DATABLOCK_TYPE_BOOL){

								value = new BoolBlock(Convert::WstringFromBoolToInt(Words[i]) != 0);
								break;
							}
							//if(Misc::WstringCompareInsensitive(Words[i], L"void")){
							//	stype = DATABLOCKTYPE_VOIDPTR;
							//	break;
							//}
							Logger::Get()->Error(L"ScriptInterface: Invalid value for type: check var type!"+currentline);

						}

						// end //

						// check for validness //
						if((varname == L"") | (value == NULL) | (stype < 3)){

							Logger::Get()->Error(L"ScriptInterface: ReadObjectBlock: s variables contains invalid var "+currentline);
							continue;
						}
						//obj->Varss[Handleindex]->Vars.push_back(new ScriptNamedArguement(varname, value, stype, true, true));

						continue;
					}

					Logger::Get()->Info(L"ScriptInterface: ReadObjectBlock: s variables block contains invalid line "+currentline, false);

				}
				break;
			case 3:
				{
					bool Working = true;

					wstring instructions = L"";
					int IntendLevel = 0;
					wstring Typename = L"";
					wstring SName = L"";

					// first line should be definition //
					if(Misc::WstringStartsWith(currentline, L"inl")){
						// get script type //
						vector<wstring> Words;
						Misc::CutWstring(currentline, L" ", Words);

						int prevtype = 0;

						IntendLevel++;

						for(unsigned int a = 1; a < Words.size(); a++){
							if(prevtype != 0){
								switch(prevtype){
								case 1:
									{
										// type specification
										if((Words[a][0] == L'"') && (Words[a][Words[a].size()-1] == L'"')){
											Typename = Misc::Replace(Words[a], L"\"", L"");
											prevtype = 0;
											break;
										}

										Logger::Get()->Error(L"ScriptInterface: ReadObjectBlock: inl has invalid type "+Words[a]);

										prevtype = 0;
									}
									break;

								}


								continue;
							}

							if(Words[a] == L"type:"){
								prevtype = 1;
								continue;
							}
						}
					}


					bool Incode = false;
					while((reader.good()) & Working){
						reader.getline(Buff, 400);
						Line++;
						currentline = Buff;

						if(currentline.size() < 1)
							continue;

						// skip empty //
						for(unsigned int i = 0; i < currentline.size(); i++){
							if(currentline[i] == L' '){
								currentline.erase(currentline.begin());
								i--;
								continue;
							} else {
								break;
							}
						}
						if(Incode){
							if(currentline == L"@%};"){
								Incode = false;
								IntendLevel = 1;
								continue;
							}
							instructions += currentline+L"\n";
							continue;
						}
						if(IntendLevel == 0){
							if(Misc::WstringStartsWith(currentline, L"inl")){
								// get script type //
								vector<wstring> Words;
								Misc::CutWstring(currentline, L" ", Words);

								int prevtype = 0;

								IntendLevel++;

								for(unsigned int a = 1; a < Words.size(); a++){
									if(prevtype != 0){
										switch(prevtype){
										case 1:
											{
												// type specification
												if((Words[a][0] == L'"') && (Words[a][Words[a].size()-1] == L'"')){
													Typename = Misc::Replace(Words[a], L"\"", L"");
													prevtype = 0;
													break;
												}

												Logger::Get()->Error(L"ScriptInterface: ReadObjectBlock: inl has invalid type "+Words[a]);

												prevtype = 0;
											}
											break;

										}


										continue;
									}

									if(Words[a] == L"type:"){
										prevtype = 1;
										continue;
									}
								}
							}
							if(Misc::WstringStartsWith(currentline, L"}")){
								Working = false;
								//Logger::Get()->Error(L"ScriptInterface: ReadObjectBlock: Script body blob contained no definitions ");
								break;
							}
							continue;
						}
						if(IntendLevel == 1){
							if(Misc::WstringStartsWith(currentline, L"name")){
								vector<wstring> Words;
								Misc::CutWstring(currentline, L" = ", Words);

								if(Words.size() != 2){

									Logger::Get()->Error(L"ScriptInterface: ReadObjectBlock: script definition invalid "+currentline, Words.size(), true);
									continue;
								}

								// remove unnecessary stuff from value //
								while(Words[1][Words[1].size()-1] == L';'){
									Words[1].erase(Words[1].begin()+Words[1].size()-1);
								}
								while(Words[1][0] == L' '){
									Words[1].erase(Words[1].begin());
								}

								SName = Words[1];
								continue;
							}
							if(Misc::WstringStartsWith(currentline, L"body")){
								IntendLevel = 2;
								Incode = true;
								continue;
							}
							if(Misc::WstringStartsWith(currentline, L"}")){
								IntendLevel = 0;
								// create script //
								ScriptScript* tscript = new ScriptScript();
								tscript->Instructions = instructions;
								tscript->Name = SName;
								tscript->Source = sourcefile+L":OBJ:"+obj->Name;

								obj->Script = shared_ptr<ScriptScript>(tscript);
								//obj->Script = new ScriptScript();
								//obj->Script->Instructions = instructions;
								//obj->Script->Name = SName;
								//obj->Script->Source = sourcefile+L":OBJ:"+obj->Name;


								//Logger::Get()->Error(L"ScriptInterface: ReadObjectBlock: Script body blob contained no definitions ");
								continue;
							}

						}

					}


					Insomething = false;
					Something = 0;
					Level--;
				}
			break;
			case 4:
				{
					// text block //
					if(Misc::WstringStartsWith(currentline, L"}")){
						// object ended //
						Insomething = false;
						Read = true;
						Something = 0;
						Level--;

						SpesLines = L"";

						continue;
					}					
					// it is always text, just add a new line //

					// store plain text //

					obj->TextBlocks[Handleindex]->Lines.push_back(new wstring(currentline));
					continue;
				}
				break;
			}

			continue;
		}



		if(Misc::WstringStartsWith(currentline, L"}")){
			// object ended //
			Insomething = false;
			Read = true;
			Something = 0;
			Level--;
			continue;
		}

		wstring start = L"";
		Misc::WstringGetFirstWord(currentline, start);

		if(start == L"l"){
			// data list //
			Insomething = true;
			Read = true;
			Something = 1;

			Level++;

			// handle first line of object //

			wstring namey = L"";
			Misc::WstringGetSecondWord(currentline, namey);
			obj->Contents.push_back(new ObjectFileList(namey));
			Handleindex = obj->Contents.size()-1;

			continue;
		}
		if((start == L"t") || (start == L"l<t>")){
			// text block //
			Insomething = true;
			Read = true;
			Something = 4;

			Level++;

			// handle first line of object //

			wstring namey = L"";
			Misc::WstringGetSecondWord(currentline, namey);
			obj->TextBlocks.push_back(new ObjectFileTextBlock(namey));
			Handleindex = obj->TextBlocks.size()-1;

			continue;
		}
		if(start == L"s"){
			// get which type of script //
			wstring type = L"";
			Misc::WstringGetSecondWord(currentline, type);

			Level++;

			if(type == L"variables"){
				Insomething = true;
				Read = true;
				Something = 2;

				//obj->Varss.push_back(shared_ptr<ScriptVariableHolder>(new ScriptVariableHolder()));
				//Handleindex = obj->Varss.size()-1;

				continue;
			}
			if(type == L"scripts"){
				Insomething = true;
				Read = true;
				Something = 3;

				//obj->Scripts.push_back(new ScriptScript());
				//Handleindex = obj->Scripts.size()-1;

				continue;
			}

			Logger::Get()->Error(L"ScriptInterface: ReadObjectBlock: invalid script block no type "+type, true);

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
			writer << L"    o " << temp->TName << L" " << Misc::VectorValuesToSingleSmartPTR<wstring>(temp->Prefixes, L" ", true) << L" \"" 
				<< temp->Name << L"\" {" << endl;
		} else {
			writer << L"    o " << temp->TName << L" \"" << temp->Name << L"\" {" << endl;
		}

		// contents //
		for(unsigned int a = 0;  a < temp->Contents.size(); a++){
			ObjectFileList* tmp = temp->Contents[a];

			// start //
			writer << L"        l " << tmp->Name << L" {" << endl;
			// add text lines first //
			for(unsigned int ind = 0; ind < tmp->Lines.size(); ind++){
				writer << L"            <t> " << *tmp->Lines[ind] << endl;
			}
			// variable lines //
			vector<shared_ptr<NamedVar>>* tempvals = tmp->Variables->GetVec();
			for(unsigned int ind = 0; ind < tempvals->size(); ind++){
				writer << L"            " << tempvals->at(ind)->ToText(0) << endl;
			}
			// end //
			writer << L"        }" << endl;
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
			writer << L"        t " << tmp->Name << L" {" << endl;
			// add text lines //
			for(unsigned int ind = 0; ind < tmp->Lines.size(); ind++){
				writer << L"            " << *tmp->Lines[ind] << endl;
			}
			// end //
			writer << L"        }" << endl;
		}
		// script block //
		if(temp->Script != NULL){
			ScriptScript* scrpt = temp->Script.get();

			// start //
			writer << L"        s scripts {" << endl;
			writer << L"            inl type: \"script\" {" << endl;
			writer << L"                name = " << scrpt->Name << L";" << endl;
			writer << L"                body {" << endl;
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
				writer << L"                    " << SplitInstrLines[i] << endl;
			}
			// body end //
			writer << L"                @%};" << endl;
			writer << L"            }" << endl;
			// end //
			writer << L"        }" << endl;
		}


		// ending bracket //
		writer << L"    }" << endl;
	}

	// end //
	writer << L"}"/* << endl*/;
	return true;
}

vector<IntWstring*> Leviathan::ObjectFileProcessor::RegisteredValues;
