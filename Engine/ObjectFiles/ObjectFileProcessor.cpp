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
#include "boost/regex/pending/unicode_iterator.hpp"
#include "utf8/checked.h"
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
	;

// ------------------------------------ //
void Leviathan::ObjectFileProcessor::Initialize(){
#ifdef _DEBUG
	// Just out of curiosity check this //
	auto iter = RegisteredValues.find(L"DATAINDEX_TICKTIME");

	if(iter == RegisteredValues.end()){

		Logger::Get()->Error(L"ObjectFileProcessor: RegisteredValues are messed up, DATAINDEX_TICKTIME is not defined, check the macros!");
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
DLLEXPORT std::vector<shared_ptr<ObjectFileObject>> Leviathan::ObjectFileProcessor::ProcessObjectFile(const std::wstring &file,
	std::vector<shared_ptr<NamedVariableList>> &HeaderVars)
{
	std::vector<shared_ptr<ObjectFileObject>> returned;

	// read the file entirely //
	std::string filecontents;

	std::string fileansi = Convert::WstringToString(file);

	try{
		FileSystem::ReadFileEntirely(fileansi, filecontents);
	}
	catch(const ExceptionInvalidArgument &e){

		Logger::Get()->Error(L"ObjectFileProcessor: ProcessObjectFile: file could not be read, exception:");
		e.PrintToLog();
		return returned;
	}
	
	// Skip empty files //
	if(filecontents.size() == 0){

		Logger::Get()->Warning(L"ObjectFileProcessor: file is empty, "+file);
		return returned;
	}

	// Skip the BOM if there is one //
	if(utf8::starts_with_bom(filecontents.begin(), filecontents.end())){

		// Pop the first 3 bytes //
		filecontents = filecontents.substr(3, filecontents.size()-3);
	}






	bool succeeded = true;

	// Create an UTF8 supporting iterator //
	StringIterator itr(new UTF8DataIterator(filecontents), true);

	while(!itr.IsOutOfBounds()){
		// First get the first thing defining what the following object/thing will be //
		auto thingtype = itr.GetNextCharacterSequence<string>(UNNORMALCHARACTER_TYPE_LOWCODES | UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS, 
			SPECIAL_ITERATOR_FILEHANDLING);


		if(!thingtype)
			continue;

		if(*thingtype == "template"){
			// Either a template definition or a template instantiation //

			// Get to the first character //
			if(itr.GetCharacter(0) != '<'){

				itr.GetUntilNextCharacterOrNothing<string>('<', SPECIAL_ITERATOR_FILEHANDLING);
			}

			// Move from the '<' to the actual content //
			itr.MoveToNext();

			// Now process the template arguments //
			auto tmplarg = itr.GetNextCharacterSequence<string>(UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS | UNNORMALCHARACTER_TYPE_LOWCODES,
				SPECIAL_ITERATOR_FILEHANDLING);

			// Check where we are right now //
			if(itr.GetCharacter() == ' '){
				// Go somewhere proper //
				itr.SkipWhiteSpace(SPECIAL_ITERATOR_FILEHANDLING);
			}

			std::vector<string*> templateargs;

			if(tmplarg && tmplarg->size()){
				templateargs.push_back(tmplarg.release());
			}

			size_t startline = itr.GetCurrentLine();

			while(itr.GetCharacter() == ','){

				// More arguments //
				tmplarg = itr.GetNextCharacterSequence<string>(UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS | UNNORMALCHARACTER_TYPE_LOWCODES,
					SPECIAL_ITERATOR_FILEHANDLING);

				if(tmplarg && tmplarg->size()){
					templateargs.push_back(tmplarg.release());
				}

				// Potentially more //
				itr.SkipWhiteSpace(SPECIAL_ITERATOR_FILEHANDLING);

				if(itr.IsOutOfBounds()){
					// Failed //
					Logger::Get()->Error(L"ObjectFile template has invalid argument list (specifically the one looking like \"template<arg1, arg2>\")"
						L", started file: "+file+L"("+Convert::ToWstring(startline)+L")");
					succeeded = false;
					break;
				}
			}

			// We should now be at the '>' character //
			if(itr.GetCharacter() != '>'){

				DEBUG_BREAK;
			}

			// Move over it //
			itr.MoveToNext();

			itr.SkipWhiteSpace(SPECIAL_ITERATOR_FILEHANDLING);

			// Now should be the name //
			auto name = itr.GetNextCharacterSequence<string>(UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS | UNNORMALCHARACTER_TYPE_LOWCODES,
				SPECIAL_ITERATOR_FILEHANDLING);

			if(!name || name->size() < 3){

				Logger::Get()->Error(L"ObjectFile template has too short name (has to be a minimum of 3 characters), file: "+file+L"("
					+Convert::ToWstring(itr.GetCurrentLine())+L")");
				succeeded = false;
				break;
			}

			if(!templateargs.size()){
				// This is a template instantiation //

				DEBUG_BREAK;
				continue;
			}

			// Handle the template definition //
			if(itr.GetCharacter() != ':'){

				Logger::Get()->Error(L"ObjectFile template definition has no ':' after name (template and following object must be separated), "
					L"file: "+file+L"("	+Convert::ToWstring(itr.GetCurrentLine())+L")");
				succeeded = false;
				break;
			}

			// Now there should be an object definition //
			DEBUG_BREAK;
			
			continue;
		} else if(*thingtype == "o"){
			// Process an object //

			auto typesname = itr.GetNextCharacterSequence<string>(UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS | UNNORMALCHARACTER_TYPE_LOWCODES,
				SPECIAL_ITERATOR_FILEHANDLING);

			if(!typesname || !typesname->size()){

				Logger::Get()->Error(L"ObjectFile object definition has no typename (or anything valid, for that matter, after 'o'), file: "+file+L"("
					+Convert::ToWstring(itr.GetCurrentLine())+L")");
				succeeded = false;
				break;
			}

			itr.SkipWhiteSpace(SPECIAL_ITERATOR_FILEHANDLING);
			
			size_t startline = itr.GetCurrentLine();

			std::vector<string*> prefixesvec;

			// Now there should be variable number of prefixes followed by a name //
			while(itr.GetCharacter() != '"'){

				auto oprefix = itr.GetNextCharacterSequence<string>(UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS | UNNORMALCHARACTER_TYPE_LOWCODES,
					SPECIAL_ITERATOR_FILEHANDLING);

				if(oprefix && oprefix->size()){
					prefixesvec.push_back(oprefix.release());
				}

				if(itr.IsOutOfBounds()){
					// Failed //
					Logger::Get()->Error(L"ObjectFile object doesn't have a name, because prefixes are messed up"
						L"(expected quoted string like this: \"o Type Prefix \'MyName\'\")"
						L", started file: "+file+L"("+Convert::ToWstring(startline)+L")");
					succeeded = false;
					break;
				}
			}

			// Now try to get the name //
			auto oname = itr.GetStringInQuotes<string>(QUOTETYPE_BOTH, SPECIAL_ITERATOR_FILEHANDLING);

			if(!oname || !oname->size()){

				Logger::Get()->Error(L"ObjectFile object doesn't have a name (expected quoted string like this: \"o Type Prefix \'MyName\'\")"
					L", started file: "+file+L"("+Convert::ToWstring(startline)+L")");
				succeeded = false;
				break;
			}

			// There should now be a { //
			itr.GetUntilNextCharacterOrNothing<string>('{', SPECIAL_ITERATOR_FILEHANDLING);

			if(itr.GetCharacter() != '{'){
				// There is a missing brace //

				Logger::Get()->Error(L"ObjectFile object is missing a '{' after it's name, file: "+file+L"("+Convert::ToWstring(itr.GetCurrentLine())+L")");
				succeeded = false;
				break;
			}

			// Now there should be the object contents //



			continue;
		} else {
			// It should be a named variable //
			//itr.SetDebugMode(true);
			
			// This is often empty //
			auto restofname = itr.GetUntilEqualityAssignment<string>(EQUALITYCHARACTER_TYPE_ALL, SPECIAL_ITERATOR_FILEHANDLING);

			// We need to skip whitespace //
			itr.SkipWhiteSpace(SPECIAL_ITERATOR_FILEHANDLING);

			// Get the value //
			auto valuestr = itr.GetUntilNextCharacterOrNothing<string>(';', SPECIAL_ITERATOR_FILEHANDLING);

			if(valuestr){
				// Try to construct a named variable //

				// Put the name together //
				string varname = *thingtype+ (restofname ? *restofname: string());

				// Use the name and the variable string to try to create one //
				shared_ptr<NamedVariableList> tmpval;

				try{

					wstring convname;
					convname.reserve(varname.size());

					utf8::utf8to16(varname.begin(), varname.end(), back_inserter(convname));

					wstring convval;
					convval.reserve(valuestr->size());

					utf8::utf8to16(valuestr->begin(), valuestr->end(), back_inserter(convval));

					tmpval = shared_ptr<NamedVariableList>(new NamedVariableList(convname, convval, &RegisteredValues));

					// It surprisingly worked! //

					// Add to the object //


					// This was a valid definition //
					continue;

				} catch(const utf8::invalid_code_point &ec){
					
					DEBUG_BREAK;
				} catch(const ExceptionInvalidArgument &e){

					DEBUG_BREAK;
				}
			}
		}

		// It is something that cannot be handled //
		DEBUG_BREAK;

		Logger::Get()->Error(L"ObjectFile has an invalid block ("+Convert::StringToWstring(*thingtype)+L"), file: "+file+L"("
			+Convert::ToWstring(itr.GetCurrentLine())+L")");
		// The file is clearly malformed //
		succeeded = false;
		break;
	}


	if(!succeeded || !itr.IsOutOfBounds()){

		// It failed //
		Logger::Get()->Error(L"ObjectFileProcessor: could not parse file: "+file+L" parsing has ended on line: "+
			Convert::ToWstring(itr.GetCurrentLine()));
		return returned;
	}

	// Generate the template instantiations and it's done //


	return returned;
}
// ------------------------------------ //
DLLEXPORT int Leviathan::ObjectFileProcessor::WriteObjectFile(std::vector<shared_ptr<ObjectFileObject>> &objects, const std::wstring &file, 
	std::vector<shared_ptr<NamedVariableList>> &headervars)
{
	// The output string which will then be written to the file //




	return false;
}






