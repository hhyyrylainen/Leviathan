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
#include "Utility/Iterators/WstringIterator.h"
#include "Common/Misc.h"
#include "utf8/core.h"
#include "boost/regex/pending/unicode_iterator.hpp"
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

	typedef std::string::const_iterator iterator_type;

	// Iterators that move through the string //
	iterator_type first(filecontents.begin());
	iterator_type last(filecontents.end());


	bool succeeded = true;


	// Start looping through the characters //




	if(!succeeded || first != last){

		// It failed //
		Logger::Get()->Error(L"ObjectFileProcessor: could not parse file: "+file);
		return returned;
	}


	return returned;
}
// ------------------------------------ //
DLLEXPORT int Leviathan::ObjectFileProcessor::WriteObjectFile(std::vector<shared_ptr<ObjectFileObject>> &objects, const std::wstring &file, 
	std::vector<shared_ptr<NamedVariableList>> &headervars)
{
	// The output string which will then be written to the file //




	return false;
}






