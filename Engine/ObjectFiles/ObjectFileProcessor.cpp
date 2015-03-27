#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_OBJECTFILEPROCESSOR
#include "ObjectFileProcessor.h"
#endif
#include "FileSystem.h"
#include <boost/assign/list_of.hpp>
#include "Common/DataStoring/DataStore.h"
#include "Common/DataStoring/DataBlock.h"
#include "Common/StringOperations.h"
#include "Iterators/StringIterator.h"
#include "Common/Misc.h"
#include "utf8/core.h"
#include "utf8/checked.h"
#include "Script/ScriptInterface.h"
#include "Script/ScriptScript.h"
#include "ObjectFileTemplates.h"
using namespace Leviathan;
// ------------------------------------ //
ObjectFileProcessor::ObjectFileProcessor(){}
Leviathan::ObjectFileProcessor::~ObjectFileProcessor(){}

// quick macro to make this shorter //
#ifdef _MSC_VER
#define ADDDATANAMEINTDEFINITION(x) (WIDEN(#x), shared_ptr<VariableBlock>(new VariableBlock(new IntBlock(x))))
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
	;

// ------------------------------------ //
void Leviathan::ObjectFileProcessor::Initialize(){
#ifdef _DEBUG
	// Just out of curiosity check this //
	auto iter = RegisteredValues.find(L"DATAINDEX_TICKTIME");

	if(iter == RegisteredValues.end()){

		Logger::Get()->Error("ObjectFileProcessor: RegisteredValues are messed up, "
            "DATAINDEX_TICKTIME is not defined, check the macros!");
        
		return;
	}
#endif // _DEBUG
}
void Leviathan::ObjectFileProcessor::Release(){
	// Release our allocated memory //
	RegisteredValues.clear();
}
// ------------------------------------ //
DLLEXPORT  void Leviathan::ObjectFileProcessor::RegisterValue(const wstring &name, VariableBlock* valuetokeep){
	RegisteredValues[name] = shared_ptr<VariableBlock>(valuetokeep);
}
// ------------------ Processing function ------------------ //
DLLEXPORT unique_ptr<ObjectFile> Leviathan::ObjectFileProcessor::ProcessObjectFile(const std::wstring &file){
	
	// First read the file entirely //
	std::string filecontents;

	std::string fileansi = Convert::WstringToString(file);

	try{
		FileSystem::ReadFileEntirely(fileansi, filecontents);
	}
	catch(const ExceptionInvalidArgument &e){

		Logger::Get()->Error(L"ObjectFileProcessor: ProcessObjectFile: file could not be read, exception:");
		e.PrintToLog();
		return NULL;
	}
	
	// Skip empty files //
	if(filecontents.size() == 0){

		Logger::Get()->Warning(L"ObjectFileProcessor: file is empty, "+file);
		return NULL;
	}

	// Skip the BOM if there is one //
	if(utf8::starts_with_bom(filecontents.begin(), filecontents.end())){

		// Pop the first 3 bytes //
		filecontents = filecontents.substr(3, filecontents.size()-3);
	}


	// Create the target object //
	unique_ptr<ObjectFile> ofile(new ObjectFile());

	bool succeeded = true;

	// Create an UTF8 supporting iterator //
	StringIterator itr(new UTF8DataIterator(filecontents), true);

	while(!itr.IsOutOfBounds()){
		// First get the first thing defining what the following object/thing will be //
		auto thingtype = itr.GetNextCharacterSequence<string>(UNNORMALCHARACTER_TYPE_LOWCODES |
            UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS, SPECIAL_ITERATOR_FILEHANDLING);

		if(!thingtype)
			continue;

		// Store the starting line for error reporting purposes //
		size_t thisstart = itr.GetCurrentLine();
        
		if(*thingtype == "template"){
			// Either a template definition or a template instantiation //
			
			if(!TryToHandleTemplate(file, itr, *ofile, *thingtype)){

				Logger::Get()->Error(L"ObjectFileProcessor: processing a template definitions/instantiation has failed,"
                    L"file: "+file+L"("+Convert::ToWstring(thisstart)+L")");
                
				succeeded = false;
				break;
			}
			
			continue;

		} else if(*thingtype == "o"){

			// Process an object //
			auto tmpobj = TryToLoadObject(file, itr, *ofile, *thingtype);

			if(!tmpobj){

				Logger::Get()->Error(L"ObjectFileProcessor: processing an object has failed, file: "+
					file+L"("+Convert::ToWstring(thisstart)+L")");
				succeeded = false;
				break;
			}

			if(!ofile->AddObject(tmpobj)){

				Logger::Get()->Error(L"ObjectFileProcessor: object has a conflicting name, name: \""+tmpobj->GetName()+
                    L"\", file: "+file+L"("+Convert::ToWstring(thisstart)+L"), current line: "+
                    Convert::ToWstring(itr.GetCurrentLine()));
                
				succeeded = false;
				break;
			}


			continue;

		} else {
			// It should be a named variable //

			auto ptr = TryToLoadNamedVariables(file, itr, *thingtype);
			if(!ptr){

				Logger::Get()->Error(L"ObjectFileProcessor: processing a NamedVariableList has failed, file: "+
					file+L"("+Convert::ToWstring(thisstart)+L")");
				succeeded = false;
				break;
			}

			// Add to the object //
			if(!ofile->AddNamedVariable(ptr)){

				Logger::Get()->Error(L"ObjectFileProcessor: variable name already in use, file: "+file+L"("+
					Convert::ToWstring(thisstart)+L"):");
				return NULL;
			}

			continue;
		}

		// It is something that cannot be handled //
		Logger::Get()->Error(L"ObjectFile has an invalid block ("+Convert::StringToWstring(*thingtype)+L"), file: "+file
            +L"("+Convert::ToWstring(thisstart)+L")");

		// The file is clearly malformed //
		succeeded = false;
		break;
	}


	if(!succeeded || !itr.IsOutOfBounds()){

		// It failed //
		Logger::Get()->Error(L"ObjectFileProcessor: could not parse file: "+file+L" parsing has ended on line: "+
			Convert::ToWstring(itr.GetCurrentLine()));
		return NULL;
	}

	// Generate the template instantiations and it's done //
	if(!ofile->GenerateTemplatedObjects()){

		Logger::Get()->Error(L"ObjectFileProcessor: file has invalid templates (either bad names, "
            L"or instances without definitions), file: "+file);
        
		return NULL;
	}

	return ofile;
}
// ------------------------------------ //
shared_ptr<NamedVariableList> Leviathan::ObjectFileProcessor::TryToLoadNamedVariables(const wstring &file,
    StringIterator &itr, const string &preceeding)
{
	// Try to load a named variable of format: "Variable = myvalue;" //

    // Store the beginning line //
    size_t startline = itr.GetCurrentLine();
    
	// Next thing after the preceeding is rest of the name until the '=' character //

	// This is often empty //
	auto restofname = itr.GetUntilEqualityAssignment<string>(EQUALITYCHARACTER_TYPE_ALL, SPECIAL_ITERATOR_FILEHANDLING);

	if(!restofname && preceeding.size() == 0){
		// No name //
		Logger::Get()->Error(L"ObjectFile named variable is malformed, unknown block?, file: "+file+L":"+
            Convert::ToWstring(startline)+L"-"+
			Convert::ToWstring(itr.GetCurrentLine())+L")");
		return NULL;
	}

	// There needs to be a separator, the last character should be it //
	int lastchar = itr.GetPreviousCharacter();

	if(lastchar != '=' && lastchar != ':'){

		// Invalid format //
		Logger::Get()->Error(L"ObjectFile block isn't a named variable, unknown block?, file: "+file+L"("+
            Convert::ToWstring(itr.GetCurrentLine())+L")");
		return NULL;
	}

	// We need to skip whitespace //
	itr.SkipWhiteSpace(SPECIAL_ITERATOR_FILEHANDLING);

	// Get the value //
	auto valuestr = itr.GetUntilNextCharacterOrNothing<string>(';', SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	if(!valuestr){
		// No ';' //
		Logger::Get()->Error(L"ObjectFile named variable is missing an ending ';' (should be like: \"MyVar = 42;\")"
			L", file: "+file+L"("+Convert::ToWstring(startline)+L")");
		return NULL;
	}

	// Skip the ';' //
	itr.MoveToNext();

	// Try to construct a named variable //

	// Put the name together //
	string varname = preceeding+ (restofname ? *restofname: string());

	// Try to use the name and the variable string to try to create one named variable //

	try{

		wstring convname;
		convname.reserve(varname.size());

		utf8::utf8to16(varname.begin(), varname.end(), back_inserter(convname));

		wstring convval;
		convval.reserve(valuestr->size());

		utf8::utf8to16(valuestr->begin(), valuestr->end(), back_inserter(convval));

		// It surprisingly worked! //

		// This was a valid definition //
		return make_shared<NamedVariableList>(convname, convval, &RegisteredValues);

	} catch(const utf8::invalid_code_point &ec){

		Logger::Get()->Error(L"ObjectFileProcessor: invalid UTF8 sequence in a named variable, file: "+file+L"("+
			Convert::ToWstring(startline)+L"):");
		Logger::Get()->Write(L"\t> "+Convert::StringToWstring(ec.what()));
		return NULL;
	} catch(const ExceptionInvalidArgument &e){

		Logger::Get()->Error(L"ObjectFileProcessor: invalid UTF8 sequence in a named variable, file: "+file+L"("+
			Convert::ToWstring(startline)+L"):");
		e.PrintToLog();
		return NULL;
	}
}
// ------------------------------------ //
bool Leviathan::ObjectFileProcessor::TryToHandleTemplate(const wstring &file, StringIterator &itr, ObjectFile &obj, const string &preceeding){
	// Skip potential space between 'template' and '<' //
	itr.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	// Get to the first character //
	if(itr.GetCharacter(0) != '<'){

		Logger::Get()->Error(L"ObjectFile template has a missing '<' after 'template', file: "+file+L"("+
			Convert::ToWstring(itr.GetCurrentLine())+L")");
		return false;
	}

	size_t startline = itr.GetCurrentLine();

	// Move from the '<' to the actual content //
	itr.MoveToNext();


	std::vector<unique_ptr<string>> templateargs;

	// Skip if there is nothing //
	if(itr.GetCharacter() != '>'){

		// Now process the template arguments //
		auto tmpldata = itr.GetUntilNextCharacterOrNothing<string>('>', SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

		if(!tmpldata){

			Logger::Get()->Error(L"ObjectFile template has an invalid argument list (missing the ending '>') , file: "+file
				+L"("+Convert::ToWstring(startline)+L")");
			return false;
		}

		StringIterator itr2(tmpldata.get());

		auto tmplarg = itr2.GetNextCharacterSequence<string>(UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS | UNNORMALCHARACTER_TYPE_LOWCODES,
			SPECIAL_ITERATOR_FILEHANDLING);

		// Go somewhere proper //
		itr2.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

		if(tmplarg && tmplarg->size()){
			templateargs.push_back(move(tmplarg));
		}

		while(itr2.GetCharacter() == ',' && !itr2.IsOutOfBounds()){

			// More arguments //
			tmplarg = itr2.GetNextCharacterSequence<string>(UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS | UNNORMALCHARACTER_TYPE_LOWCODES,
				SPECIAL_ITERATOR_FILEHANDLING);

			if(tmplarg && tmplarg->size()){
				templateargs.push_back(move(tmplarg));
			}

			// Potentially more //
			itr2.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);
		}
	}

	StringIterator quoteremover(NULL, false);

	// Remove any quotes from the arguments //
	for(size_t i = 0; i < templateargs.size(); i++){

		auto chartype = templateargs[i]->at(0);

		if(chartype == '"' || chartype == '\''){
			// Remove the quotes //
			quoteremover.ReInit(new UTF8DataIterator(*templateargs[i]), true);
			auto newstr = quoteremover.GetStringInQuotes<string>(QUOTETYPE_BOTH);
			if(!newstr || newstr->empty()){

				Logger::Get()->Warning(L"ObjectFileProcessor: Template: failed to remove quotes from template argument, "+
					Convert::StringToWstring(*templateargs[i]));
				continue;
			}

			templateargs[i] = move(newstr);
		}
	}


	// We should now be at the '>' character //

	// Move over it //
	itr.MoveToNext();

	itr.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	// Now should be the name //
	auto name = itr.GetNextCharacterSequence<string>(UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS | UNNORMALCHARACTER_TYPE_LOWCODES,
		SPECIAL_ITERATOR_FILEHANDLING);

	if(!name || name->size() < 3){

		Logger::Get()->Error(L"ObjectFile template has too short name (has to be a minimum of 3 characters), file: "+file+L"("
			+Convert::ToWstring(itr.GetCurrentLine())+L")");
		return false;
	}

	if(!templateargs.size()){
		// This is a template instantiation //

		// Next should be the argument list //
		itr.SkipWhiteSpace(SPECIAL_ITERATOR_FILEHANDLING);

		// We should now be at the '<' character //
		if(itr.GetCharacter() != '<'){

			Logger::Get()->Error(L"ObjectFile template instance has an invalid argument list (missing starting '<' after name) , file: "+file
				+L"("+Convert::ToWstring(startline)+L")");
			return false;
		}

		itr.MoveToNext();

		auto tmpldata = itr.GetUntilNextCharacterOrNothing<string>('>', SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

		if(!tmpldata){

			Logger::Get()->Error(L"ObjectFile template has an invalid argument list (missing the ending '>' or the "
				L"instantiation is missing it's parameters) , file: "+file+L"("+Convert::ToWstring(startline)+L")");
			return false;
		}

		StringIterator itr3(tmpldata.get());


		// Load all the arguments //
		auto instarg = itr3.GetNextCharacterSequence<string>(UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS | UNNORMALCHARACTER_TYPE_LOWCODES,
			SPECIAL_ITERATOR_FILEHANDLING);

		itr.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

		std::vector<unique_ptr<string>> instanceargs;

		if(instarg && instarg->size()){
			instanceargs.push_back(move(instarg));
		}

		while(itr3.GetCharacter() == ',' && !itr3.IsOutOfBounds()){

			// More arguments //
			instarg = itr3.GetNextCharacterSequence<string>(UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS | UNNORMALCHARACTER_TYPE_LOWCODES,
				SPECIAL_ITERATOR_FILEHANDLING);

			if(instarg && instarg->size()){
				instanceargs.push_back(move(instarg));
			}

			// Potentially more //
			itr3.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);
		}

		// Remove any quotes from the arguments //
		for(size_t i = 0; i < instanceargs.size(); i++){

			auto chartype = instanceargs[i]->at(0);

			if(chartype == '"' || chartype == '\''){
				// Remove the quotes //
				quoteremover.ReInit(new UTF8DataIterator(*instanceargs[i]), true);
				auto newstr = quoteremover.GetStringInQuotes<string>(QUOTETYPE_BOTH);
				if(!newstr || newstr->empty()){

					if(instanceargs[i]->size() > 2){
						Logger::Get()->Warning(L"ObjectFileProcessor: Template: failed to remove quotes from template argument, "+
							Convert::StringToWstring(*instanceargs[i]));
					}
					continue;
				}

				instanceargs[i] = move(newstr);
			}
		}


		// We should now be at the '>' character //
		itr.MoveToNext();

		// Create a template instantiation and add it to the file //
		auto ourval = make_shared<ObjectFileTemplateInstance>(*name, instanceargs);

		// Add it //
		obj.AddTemplateInstance(ourval);


		return true;
	}

	// Handle the template definition //
	if(itr.GetCharacter() != ':'){

		Logger::Get()->Error(L"ObjectFile template definition has no ':' after name (template and following object must be separated), "
			L"file: "+file+L"("	+Convert::ToWstring(startline)+L")");
		return false;
	}

	// Skip the o //
	auto justchecking = itr.GetUntilNextCharacterOrNothing<string>('o', SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	if(!justchecking){

		Logger::Get()->Error(L"ObjectFile template definition is missing 'o' after ':' (or maybe there is a missing space?), "
			L"file: "+file+L"("	+Convert::ToWstring(startline)+L")");
		return false;
	}

	// Move over it and the object should start there //
	itr.MoveToNext();

	// Now there should be an object definition //
	auto templatesobject = TryToLoadObject(file, itr, obj, "");

	if(!templatesobject){

		Logger::Get()->Error(L"ObjectFile template definition has an invalid object, "
			L"file: "+file+L"("	+Convert::ToWstring(startline)+L")");
		return false;
	}

	// Create a template definition from the object //
	auto createdtemplate = ObjectFileTemplateDefinition::CreateFromObject(*name, templatesobject, templateargs);

	if(!createdtemplate){

		Logger::Get()->Error(L"ObjectFile template failed to create from an object, file: "+file+L"("+Convert::ToWstring(startline)+L")");
		return false;
	}


	// Now add it //
	if(!obj.AddTemplate(createdtemplate)){


		Logger::Get()->Error(L"ObjectFile template has a conflicting name, name: \""+Convert::StringToWstring(*name)+L"\", "
			L"file: "+file+L"("	+Convert::ToWstring(startline)+L")");
		return false;
	}


	return true;
}
// ------------------------------------ //
shared_ptr<ObjectFileObject> Leviathan::ObjectFileProcessor::TryToLoadObject(const wstring &file, StringIterator &itr, ObjectFile &obj, 
	const string &preceeding)
{
	auto typesname = itr.GetNextCharacterSequence<string>(UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS | UNNORMALCHARACTER_TYPE_LOWCODES,
		SPECIAL_ITERATOR_FILEHANDLING);

	if(!typesname || !typesname->size()){

		Logger::Get()->Error(L"ObjectFile object definition has no typename (or anything valid, for that matter, after 'o'), file: "+file+L"("
			+Convert::ToWstring(itr.GetCurrentLine())+L")");
		return NULL;
	}

	itr.SkipWhiteSpace(SPECIAL_ITERATOR_FILEHANDLING);

	size_t startline = itr.GetCurrentLine();

	std::vector<unique_ptr<string>> prefixesvec;

	// Now there should be variable number of prefixes followed by a name //
	while(itr.GetCharacter() != '"'){

		auto oprefix = itr.GetNextCharacterSequence<string>(UNNORMALCHARACTER_TYPE_LOWCODES, SPECIAL_ITERATOR_FILEHANDLING);

		if(oprefix && oprefix->size()){
			prefixesvec.push_back(move(oprefix));
		}

		itr.SkipWhiteSpace(SPECIAL_ITERATOR_FILEHANDLING);

		if(itr.IsOutOfBounds()){
			// Failed //
			Logger::Get()->Error(L"ObjectFile object doesn't have a name, because prefixes are messed up"
				L"(expected quoted string like this: \"o Type Prefix \'MyName\'\")"
				L", started file: "+file+L"("+Convert::ToWstring(startline)+L")");
			return NULL;
		}
	}

	// Now try to get the name //
	auto oname = itr.GetStringInQuotes<string>(QUOTETYPE_BOTH, SPECIAL_ITERATOR_FILEHANDLING);

	if(!oname || !oname->size()){

		Logger::Get()->Error(L"ObjectFile object doesn't have a name (expected quoted string like this: \"o Type Prefix \'MyName\'\")"
			L", started file: "+file+L"("+Convert::ToWstring(startline)+L")");
		return NULL;
	}

	// There should now be a { //
	itr.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	if(itr.GetCharacter() != '{'){
		// There is a missing brace //

		Logger::Get()->Error(L"ObjectFile object is missing a '{' after it's name, file: "+file+L"("+Convert::ToWstring(itr.GetCurrentLine())+L")");
		return NULL;
	}

	// Convert the loaded utf8 strings to wide strings //
	std::vector<wstring*> convprefix;
	wstring wstrname;
	wstring wstrtname;

	// Reserve space //
	wstrname.reserve(oname->size());
	wstrtname.reserve(typesname->size());
	convprefix.reserve(prefixesvec.size());

	try{


		utf8::utf8to16(oname->begin(), oname->end(), back_inserter(wstrname));
		
		utf8::utf8to16(typesname->begin(), typesname->end(), back_inserter(wstrtname));

		// Convert the prefixes //
		for(size_t i = 0; i < prefixesvec.size(); i++){

			unique_ptr<wstring> resstr(new wstring());
			resstr->reserve(prefixesvec[i]->size());

			utf8::utf8to16(prefixesvec[i]->begin(), prefixesvec[i]->end(), back_inserter(*resstr));

			convprefix.push_back(resstr.release());
		}

	} catch(const utf8::invalid_utf8 &e1){

		SAFE_DELETE_VECTOR(convprefix);

		Logger::Get()->Error(L"ObjectFile contains an invalid utf8 sequence, file: "+file+L"("+Convert::ToWstring(startline)+L"):");
		Logger::Get()->Write(L"\t> "+Convert::StringToWstring(e1.what()));
		return NULL;

	} catch(const utf8::not_enough_room &e2){
		
		SAFE_DELETE_VECTOR(convprefix);

		Logger::Get()->Error(L"ObjectFile contains an invalid utf8 sequence, file: "+file+L"("+Convert::ToWstring(startline)+L"):");
		Logger::Get()->Write(L"\t> "+Convert::StringToWstring(e2.what()));
		return NULL;
	}


	// Create a new ObjectFileObject to hold our contents //
	shared_ptr<ObjectFileObject> ourobj = make_shared<ObjectFileObjectProper>(wstrname, wstrtname, convprefix);

	// These are now managed by the object (so we don't need to delete these) //
	convprefix.clear();


	// Now there should be the object contents //
	itr.MoveToNext();

	while(itr.GetCharacter() != '}'){
		// First skip whitespace //
		itr.SkipWhiteSpace(SPECIAL_ITERATOR_FILEHANDLING);

		// Then check the character //
		int curcharacter = itr.GetCharacter();
		size_t ourline = itr.GetCurrentLine();

		// We want to be on the next character to continue processing //
		itr.MoveToNext();

		switch(curcharacter){
		case 'l':
			{
				if(!TryToLoadVariableList(file, itr, *ourobj, startline)){

					Logger::Get()->Error(L"ObjectFile object contains an invalid variable list, file: "+file+L"("+Convert::ToWstring(ourline)+L")");
					return NULL;
				}
			}
			break;
		case 't':
			{
				if(!TryToLoadTextBlock(file, itr, *ourobj, startline)){

					Logger::Get()->Error(L"ObjectFile object contains an invalid text block, file: "+file+L"("+Convert::ToWstring(ourline)+L")");
					return NULL;
				}
			}
			break;
		case 's':
			{
				if(!TryToLoadScriptBlock(file, itr, *ourobj, startline)){

					Logger::Get()->Error(L"ObjectFile object contains an invalid script block, file: "+file+L"("+Convert::ToWstring(ourline)+L")");
					return NULL;
				}
			}
			break;
		case '}':
			{
				// The object ended properly //

				// Add the object to the file's object //
				return ourobj;
			}
			break;
		}

	}

	// It didn't end properly //
	Logger::Get()->Error(L"ObjectFile object is missing a closing '}' after it's contents, file: "+file+L"("+Convert::ToWstring(startline)+L")");
	return NULL;
}
// ------------------------------------ //
bool Leviathan::ObjectFileProcessor::TryToLoadVariableList(const wstring &file, StringIterator &itr, ObjectFileObject &obj, size_t startline){
	// First thing is the name //
	auto ourname = itr.GetNextCharacterSequence<string>(UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS | UNNORMALCHARACTER_TYPE_LOWCODES,
		SPECIAL_ITERATOR_FILEHANDLING);

	// Check is it valid //
	if(!ourname || ourname->size() == 0){

		Logger::Get()->Error(L"ObjectFile variable list has an invalid name, file: "+file+L"("+Convert::ToWstring(itr.GetCurrentLine())+L")");
		return false;
	}

	itr.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	// There should be a '{' character here //
	if(itr.GetCharacter() != '{'){
		// There is a missing brace //

		Logger::Get()->Error(L"ObjectFile variable list is missing '{' after it's name, file: "+file+L"("+
			Convert::ToWstring(itr.GetCurrentLine())+L")");
		return false;
	}

	// Still on the first line //
	size_t ourstartline = itr.GetCurrentLine();

	itr.MoveToNext();

	itr.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);


	// Convert the loaded utf8 strings to wide strings //
	wstring wstrname;

	try{
		wstrname.reserve(ourname->size());

		utf8::utf8to16(ourname->begin(), ourname->end(), back_inserter(wstrname));

	} catch(const utf8::invalid_utf8 &e1){

		Logger::Get()->Error(L"ObjectFile contains an invalid utf8 sequence, file: "+file+L"("+Convert::ToWstring(startline)+L"):");
		Logger::Get()->Write(L"\t> "+Convert::StringToWstring(e1.what()));
		return false;

	} catch(const utf8::not_enough_room &e2){

		Logger::Get()->Error(L"ObjectFile contains an invalid utf8 sequence, file: "+file+L"("+Convert::ToWstring(startline)+L"):");
		Logger::Get()->Write(L"\t> "+Convert::StringToWstring(e2.what()));
		return false;
	}

	// Create us //
	unique_ptr<ObjectFileList> ourobj(new ObjectFileListProper(wstrname));


	// Now we should get named variables until a } //
	while(!itr.IsOutOfBounds()){
		// First skip whitespace //
		itr.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

		if(itr.GetCharacter() == '}'){
			// Valid //
			// Skip it so the object doesn't end here //
			itr.MoveToNext();

			// Add us to the object //
			if(!obj.AddVariableList(ourobj)){

				Logger::Get()->Error(L"ObjectFile variable list has conflicting name inside it's object, file: "+file+L"("+
					Convert::ToWstring(ourstartline)+L")");
				return false;
			}
			return true;
		}

		size_t varsline = itr.GetCurrentLine();
		// Try to load a named variable //
		auto loadvar = TryToLoadNamedVariables(file, itr, "");

		if(!loadvar){

			Logger::Get()->Error(L"ObjectFile variable list has an invalid variable, file: "+file+L"("+
				Convert::ToWstring(varsline)+L")");
			return false;
		}

		// Add a variable to us //
		if(!ourobj->AddVariable(loadvar)){

			Logger::Get()->Error(L"ObjectFile variable list has conflicting name inside it's object, name: \""+loadvar->GetName()+
				L"\", file: "+file+L"("+Convert::ToWstring(itr.GetCurrentLine())+L")");
			return false;
		}
	}


	Logger::Get()->Error(L"ObjectFile variable list is missing the closing '}', file: "+file+L"("+Convert::ToWstring(ourstartline)+L")");
	// It failed //
	return false;
}

bool Leviathan::ObjectFileProcessor::TryToLoadTextBlock(const wstring &file, StringIterator &itr, ObjectFileObject &obj, size_t startline){
	// First thing is the name //
	auto ourname = itr.GetNextCharacterSequence<string>(UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS | UNNORMALCHARACTER_TYPE_LOWCODES,
		SPECIAL_ITERATOR_FILEHANDLING);

	// Check is it valid //
	if(!ourname || ourname->size() == 0){

		Logger::Get()->Error(L"ObjectFile variable list has an invalid name, file: "+file+L"("+Convert::ToWstring(itr.GetCurrentLine())+L")");
		return false;
	}

	itr.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	// There should be a '{' character here //
	if(itr.GetCharacter() != '{'){
		// There is a missing brace //

		Logger::Get()->Error(L"ObjectFile variable list is missing '{' after it's name, file: "+file+L"("+
			Convert::ToWstring(itr.GetCurrentLine())+L")");
		return false;
	}

	// Still on the first line //
	size_t ourstartline = itr.GetCurrentLine();

	itr.MoveToNext();

	itr.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	// Convert the loaded utf8 strings to wide strings //
	wstring wstrname;

	try{
		wstrname.reserve(ourname->size());

		utf8::utf8to16(ourname->begin(), ourname->end(), back_inserter(wstrname));

	} catch(const utf8::invalid_utf8 &e1){

		Logger::Get()->Error(L"ObjectFile contains an invalid utf8 sequence, file: "+file+L"("+Convert::ToWstring(startline)+L"):");
		Logger::Get()->Write(L"\t> "+Convert::StringToWstring(e1.what()));
		return false;

	} catch(const utf8::not_enough_room &e2){

		Logger::Get()->Error(L"ObjectFile contains an invalid utf8 sequence, file: "+file+L"("+Convert::ToWstring(startline)+L"):");
		Logger::Get()->Write(L"\t> "+Convert::StringToWstring(e2.what()));
		return false;
	}

	// Create us //
	unique_ptr<ObjectFileTextBlock> ourobj(new ObjectFileTextBlockProper(wstrname));


	// Now we should get named variables until a } //
	while(!itr.IsOutOfBounds()){
		// First skip whitespace //
		itr.SkipWhiteSpace(SPECIAL_ITERATOR_FILEHANDLING);

		if(itr.GetCharacter() == '}'){
			// Valid //

			// Skip it so the object doesn't end here //
			itr.MoveToNext();

			// Add us to the object //
			if(!obj.AddTextBlock(ourobj)){

				Logger::Get()->Error(L"ObjectFile text block has a conflicting name, file: "+file+L"("+Convert::ToWstring(ourstartline)+L")");
				return false;
			}

			return true;
		}

		// Read a single line //
		auto linething = itr.GetUntilLineEnd<string>();

		// Add it to us //
		ourobj->AddTextLine(*linething);
	}


	Logger::Get()->Error(L"ObjectFile text block is missing the closing '}', file: "+file+L"("+Convert::ToWstring(ourstartline)+L")");
	// It failed //
	return false;
}

bool Leviathan::ObjectFileProcessor::TryToLoadScriptBlock(const wstring &file, StringIterator &itr, ObjectFileObject &obj, size_t startline){
	// The line that contains the 's' //
	size_t lineforname = itr.GetCurrentLine();

	// First thing is the name //
	auto ourname = itr.GetNextCharacterSequence<string>(UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS | UNNORMALCHARACTER_TYPE_LOWCODES,
		SPECIAL_ITERATOR_FILEHANDLING);

	wstring modname;

	// Check is it valid //
	if(!ourname || ourname->size() == 0){
		// Auto generate our name //
		modname = obj.GetName()+L"'s_script";
	}


	itr.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	// There should be a '{' character here, or the line must have changed //
	bool linechanged = (lineforname != itr.GetCurrentLine());
	if(itr.GetCharacter() != '{' && !linechanged){
		// There is a missing brace //

		Logger::Get()->Error(L"ObjectFile script block is missing '{' after it's name, file: "+file+L"("+
			Convert::ToWstring(itr.GetCurrentLine())+L")");
		return false;
	}

	// Move to the next line //
	if(!linechanged)
		itr.GetUntilLineEnd<string>();

	// This is the line the script block starts //
	// TODO: verify that this is correct
	//size_t ourstartline = itr.GetCurrentLine()-1;
	size_t ourstartline = itr.GetCurrentLine();

	// Get until the ending sequence //
	auto scriptdata = itr.GetUntilCharacterSequence<string>("@%};", SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	if(!scriptdata){

		Logger::Get()->Error(L"ObjectFile script block is missing the source code, file: "+file+L"("+
			Convert::ToWstring(ourstartline)+L")");
		return false;
	}

	// Check was it terminated properly, the last character processed should be ';' //
	if(itr.GetCharacter() != ';'){

		Logger::Get()->Error(L"ObjectFile script block is missing the ending sequence (\"@%};\"), file: "+file+L"("+
			Convert::ToWstring(ourstartline)+L")");
		return false;
	}
	
	// Only do conversion if the name hasn't been generated into a wstring already //
	if(ourname && ourname->size() > 0){
		try{
			modname.reserve(ourname->size());

			utf8::utf8to16(ourname->begin(), ourname->end(), back_inserter(modname));

		} catch(const utf8::invalid_utf8 &e1){

			Logger::Get()->Error(L"ObjectFile contains an invalid utf8 sequence, file: "+file+L"("+Convert::ToWstring(startline)+L"):");
			Logger::Get()->Write(L"\t> "+Convert::StringToWstring(e1.what()));
			return false;

		} catch(const utf8::not_enough_room &e2){

			Logger::Get()->Error(L"ObjectFile contains an invalid utf8 sequence, file: "+file+L"("+Convert::ToWstring(startline)+L"):");
			Logger::Get()->Write(L"\t> "+Convert::StringToWstring(e2.what()));
			return false;
		}
	}

	// Create us //
	shared_ptr<ScriptScript> ourobj(new ScriptScript(ScriptInterface::Get()->GetExecutor()->CreateNewModule(modname, Convert::WstringToString(file)
		+"("+Convert::ToString(ourstartline)+")")));
	
	// Add the source to the script //
	auto ourmod = ourobj->GetModule();
	

	ourmod->AddScriptSegment(Convert::WstringToString(file), ourstartline, *scriptdata);
	ourmod->SetBuildState(SCRIPTBUILDSTATE_READYTOBUILD);


	// Add to the object //
	obj.AddScriptScript(ourobj);


	return true;
}
// ------------------------------------ //
DLLEXPORT  bool Leviathan::ObjectFileProcessor::WriteObjectFile(ObjectFile &data, const std::wstring &file){


	return false;
}

