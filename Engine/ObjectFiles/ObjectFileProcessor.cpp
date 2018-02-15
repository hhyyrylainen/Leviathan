#include "Include.h"
// ------------------------------------ //
#include "ObjectFileProcessor.h"

#include "FileSystem.h"
#include "Common/DataStoring/DataBlock.h"
#ifndef NO_DEFAULT_DATAINDEX
#include "../Common/DataStoring/DataStore.h"
#endif
#include "Common/StringOperations.h"
#include "Iterators/StringIterator.h"
#include "utf8/core.h"
#include "utf8/checked.h"

#include "ObjectFile.h"

#ifdef ALLOW_INTERNAL_EXCEPTIONS
#include "Exceptions.h"
#endif // ALLOW_INTERNAL_EXCEPTIONS

#ifdef LEVIATHAN_USING_ANGELSCRIPT
#include "../Script/ScriptModule.h"
#include "Script/ScriptExecutor.h"
#include "Script/ScriptScript.h"
#endif // LEVIATHAN_USING_ANGELSCRIPT
using namespace Leviathan;
// ------------------------------------ //
#ifndef NO_DEFAULT_DATAINDEX
// quick macro to make this shorter //
#define ADDDATANAMEINTDEFINITION(x) {#x, std::shared_ptr<VariableBlock>(new VariableBlock(\
                new IntBlock(x)))}


std::map<std::string, std::shared_ptr<VariableBlock>>
    Leviathan::ObjectFileProcessor::RegisteredValues = {
    ADDDATANAMEINTDEFINITION(DATAINDEX_TICKTIME),
    ADDDATANAMEINTDEFINITION(DATAINDEX_TICKCOUNT),
    ADDDATANAMEINTDEFINITION(DATAINDEX_FRAMETIME),
    ADDDATANAMEINTDEFINITION(DATAINDEX_FPS),
    ADDDATANAMEINTDEFINITION(DATAINDEX_WIDTH),
    ADDDATANAMEINTDEFINITION(DATAINDEX_HEIGHT),
    ADDDATANAMEINTDEFINITION(DATAINDEX_FRAMETIME_MIN),
    ADDDATANAMEINTDEFINITION(DATAINDEX_FRAMETIME_MAX),
    ADDDATANAMEINTDEFINITION(DATAINDEX_FRAMETIME_AVERAGE),
    ADDDATANAMEINTDEFINITION(DATAINDEX_FPS_AVERAGE),
    ADDDATANAMEINTDEFINITION(DATAINDEX_FPS_MIN),
    ADDDATANAMEINTDEFINITION(DATAINDEX_FPS_MAX),
    
};
#else
std::map<std::string, std::shared_ptr<VariableBlock>>
    Leviathan::ObjectFileProcessor::RegisteredValues;
#endif
// ------------------------------------ //
void Leviathan::ObjectFileProcessor::Initialize(){
#if defined(_DEBUG) && !defined(NO_DEFAULT_DATAINDEX)
	// Just out of curiosity check this //
	auto iter = RegisteredValues.find("DATAINDEX_TICKTIME");

	if(iter == RegisteredValues.end()){

		LOG_FATAL("ObjectFileProcessor: RegisteredValues are messed up, "
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
DLLEXPORT  void Leviathan::ObjectFileProcessor::RegisterValue(const std::string &name,
    VariableBlock* valuetokeep)
{
	RegisteredValues[name] = std::shared_ptr<VariableBlock>(valuetokeep);
}

template<class TStr, class TLineType>
    void LogError(const TStr &msgStart, const std::string &file, TLineType line,
    LErrorReporter* reporterror)
{
    std::stringstream sstream;
    sstream << "ObjectFileProcessor: " << msgStart << ", file: " << file << ":" << line;
    
    reporterror->Error(sstream.str());
}
// ------------------ Processing function ------------------ //
DLLEXPORT std::unique_ptr<ObjectFile> Leviathan::ObjectFileProcessor::ProcessObjectFile(
    const std::string &file, LErrorReporter* reporterror)
{
	// First read the file entirely //
	std::string filecontents;

    if (!FileSystem::ReadFileEntirely(file, filecontents)) {

        reporterror->Error("ObjectFileProcessor: ProcessObjectFile: file could not be read: " +
            file);
        return nullptr;
    }
	
	// Skip empty files //
	if(filecontents.size() == 0){

		return nullptr;
	}

	// Skip the BOM if there is one //
	if(utf8::starts_with_bom(filecontents.begin(), filecontents.end())){

		// Pop the first 3 bytes //
		filecontents = filecontents.substr(3, filecontents.size()-3);
	}

    return ProcessObjectFileFromString(filecontents, file, reporterror);
}

DLLEXPORT std::unique_ptr<Leviathan::ObjectFile>
    ObjectFileProcessor::ProcessObjectFileFromString(
    const std::string &filecontents, const std::string &filenameforerrors,
    LErrorReporter* reporterror) 
{
    // Create the target object //
    auto ofile = std::make_unique<ObjectFile>();

    bool succeeded = true;

    // Create an UTF8 supporting iterator //
    StringIterator itr(std::make_unique<UTF8PointerDataIterator>(filecontents));

    while(!itr.IsOutOfBounds()){
        
        // First get the first thing defining what the following object/thing will be //
        auto thingtype = itr.GetNextCharacterSequence<std::string>(
            UNNORMALCHARACTER_TYPE_LOWCODES | UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS,
            SPECIAL_ITERATOR_FILEHANDLING);

        // Empty line
        if(!thingtype)
            continue;

        // Store the starting line for error reporting purposes //
        size_t thisstart = itr.GetCurrentLine();

        if(*thingtype == "template") {
            // Either a template definition or a template instantiation //

            if(!TryToHandleTemplate(filenameforerrors, itr, *ofile, *thingtype, reporterror)){

                LogError("processing a template definitions or instantiation has failed",
                    filenameforerrors, thisstart, reporterror);

                succeeded = false;
                break;
            }

            continue;

        } else if(*thingtype == "o"){

            // Process an object //
            auto tmpobj = TryToLoadObject(filenameforerrors, itr, *ofile, *thingtype,
                reporterror);

            if(!tmpobj){

                LogError("processing an object has failed",
                    filenameforerrors, thisstart, reporterror);
                
                succeeded = false;
                break;
            }

            if(!ofile->AddObject(tmpobj)){

                LogError("object has a conflicting name, name: \"" + tmpobj->GetName() +
                    "\"",
                    filenameforerrors, thisstart, reporterror);

                succeeded = false;
                break;
            }


            continue;

        } else{
            // It should be a named variable //

            auto ptr = TryToLoadNamedVariables(filenameforerrors, itr, *thingtype, reporterror);
            if(!ptr){

                LogError("processing a NamedVariableList has failed",
                    filenameforerrors, thisstart, reporterror);
                
                succeeded = false;
                break;
            }

            // Add to the object //
            if(!ofile->AddNamedVariable(ptr)){

                LogError("variable name (\"" + ptr->GetName() + "\") already in use",
                    filenameforerrors, thisstart, reporterror);
                
                succeeded = false;
                break;
            }

            continue;
        }

        // It is something that cannot be handled //
        LogError("ObjectFile has an invalid/unknown block (" + *thingtype + ")",
            filenameforerrors, thisstart, reporterror);
                        
        // The file is clearly malformed //
        succeeded = false;
        break;
    }


    if(!succeeded || !itr.IsOutOfBounds()){

        // It failed //
        LogError("parsing file failed. Parsing ended",
            filenameforerrors, itr.GetCurrentLine(), reporterror);

        // Notify about unended strings and comments //

        if(itr.IsInsideString()){
            LogError("parsing ended inside unclosed quotes",
                filenameforerrors, itr.GetCurrentLine(), reporterror);
        }

        if(itr.IsInsideComment()){
            LogError("parsing ended inside unclosed comment",
                filenameforerrors, itr.GetCurrentLine(), reporterror);
        }
        
        return nullptr;
    }

    // Generate the template instantiations and it's done //
    if(!ofile->GenerateTemplatedObjects(reporterror)){

        LogError("file has invalid templates (either bad names, "
            "or instances without definitions)",
            filenameforerrors, itr.GetCurrentLine(), reporterror);
        
        return nullptr;
    }

    return ofile;
}

// ------------------------------------ //
std::shared_ptr<NamedVariableList>
    Leviathan::ObjectFileProcessor::TryToLoadNamedVariables(const std::string &file,
        StringIterator &itr, const std::string &preceeding, LErrorReporter* reporterror)
{
	// Try to load a named variable of format: "Variable = myvalue;" //

    // Store the beginning line //
    size_t startline = itr.GetCurrentLine();
    
	// Next thing after the preceeding is rest of the name until the '=' character //

	// This is often empty //
	auto restofname = itr.GetUntilEqualityAssignment<std::string>(
        EQUALITYCHARACTER_TYPE_ALL, SPECIAL_ITERATOR_FILEHANDLING);

	if(!restofname && preceeding.size() == 0){
		// No name //
        LogError("ObjectFile named variable is malformed, unknown block?",
            file, Convert::ToString(startline)+"-"+
			Convert::ToString(itr.GetCurrentLine()), reporterror);
        
		return nullptr;
	}

	// There needs to be a separator, the last character should be it //
	int lastchar = itr.GetPreviousCharacter();

	if(lastchar != '=' && lastchar != ':'){

		// Invalid format //
        LogError("ObjectFile block isn't a named variable, unknown block?",
            file, itr.GetCurrentLine(), reporterror);
        
		return nullptr;
	}

	// We need to skip whitespace //
	itr.SkipWhiteSpace(SPECIAL_ITERATOR_FILEHANDLING);

	// Get the value //
	auto valuestr = itr.GetUntilNextCharacterOrAll<std::string>(';', 
        SPECIAL_ITERATOR_ONNEWLINE_STOP | SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	if(!valuestr){
		// No ';' or nothing after the equals sign //

        LogError("ObjectFile named variable is empty or missing an (optional) ending ';' "
            "(should be like: \"MyVar = 42;\")",
            file, startline, reporterror);
        
		return nullptr;
	}

	// Skip the ';' //
	itr.MoveToNext();

	// Try to construct a named variable //

	// Put the name together //
    std::string varname = preceeding+ (restofname ? *restofname: std::string());

	// Try to use the name and the variable string to try to create one named variable //

	try{

        // NamedVariableList now uses UTF8 so conversion is no longer required //

		// This was a valid definition //
		return std::make_shared<NamedVariableList>(varname, *valuestr,
            reporterror, &RegisteredValues);

	} catch(const InvalidArgument &e){

        LogError("named variable parse failed (see following exception)",
            file, startline, reporterror);
		e.Print(reporterror);
		return nullptr;
	}
}
// ------------------------------------ //
bool Leviathan::ObjectFileProcessor::TryToHandleTemplate(const std::string &file,
    StringIterator &itr, ObjectFile &obj, const std::string &preceeding,
    LErrorReporter* reporterror)
{
	// Skip potential space between 'template' and '<' //
	itr.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	// Get to the first character //
	if(itr.GetCharacter(0) != '<'){
        
        LogError("ObjectFile template has a missing '<' after 'template'",
            file, itr.GetCurrentLine(), reporterror);
        
		return false;
	}

	size_t startline = itr.GetCurrentLine();

	// Move from the '<' to the actual content //
	itr.MoveToNext();

	std::vector<std::unique_ptr<std::string>> templateargs;

	// Skip ifthere is nothing //
	if(itr.GetCharacter() != '>'){

		// Now process the template arguments //
		auto tmpldata = itr.GetUntilNextCharacterOrNothing<std::string>('>',
            SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

		if(!tmpldata){
            
            LogError("ObjectFile template has an invalid argument list (missing "
                "the ending '>')",
                file, startline, reporterror);

			return false;
		}

		StringIterator itr2(tmpldata.get());

		auto tmplarg = itr2.GetNextCharacterSequence<std::string>(
            UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS |
            UNNORMALCHARACTER_TYPE_LOWCODES,
			SPECIAL_ITERATOR_FILEHANDLING);

		// Go somewhere proper //
		itr2.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

		if(tmplarg && tmplarg->size()){
			templateargs.push_back(move(tmplarg));
		}

		while(itr2.GetCharacter() == ',' && !itr2.IsOutOfBounds()){

			// More arguments //
			tmplarg = itr2.GetNextCharacterSequence<std::string>(
                UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS |
                UNNORMALCHARACTER_TYPE_LOWCODES,
				SPECIAL_ITERATOR_FILEHANDLING);

			if(tmplarg && tmplarg->size()){
				templateargs.push_back(move(tmplarg));
			}

			// Potentially more //
			itr2.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);
		}
	}

	StringIterator quoteremover;

	// Remove any quotes from the arguments //
	for(size_t i = 0; i < templateargs.size(); i++){

		auto chartype = templateargs[i]->at(0);

		if(chartype == '"' || chartype == '\''){
            
			// Remove the quotes //
			quoteremover.ReInit(std::make_unique<UTF8DataIterator>(*templateargs[i]));
			auto newstr = quoteremover.GetStringInQuotes<std::string>(QUOTETYPE_BOTH);
            
			if(!newstr || newstr->empty()){
                
                LogError("Template: failed to remove quotes from template argument, " +
                    *templateargs[i],
                    file, startline, reporterror);
                
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
	auto name = itr.GetNextCharacterSequence<std::string>(
        UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS |
        UNNORMALCHARACTER_TYPE_LOWCODES, SPECIAL_ITERATOR_FILEHANDLING);

	if(!name || name->size() < 3){

        LogError("ObjectFile template has too short name (has to be a minimum of 3 "
            "characters), file",
            file, itr.GetCurrentLine(), reporterror);
        
		return false;
	}

	if(!templateargs.size()){
		// This is a template instantiation //

		// Next should be the argument list //
		itr.SkipWhiteSpace(SPECIAL_ITERATOR_FILEHANDLING);

		// We should now be at the '<' character //
		if(itr.GetCharacter() != '<'){
            
            LogError("ObjectFile template instance has an invalid argument list "
                "(missing starting '<' after name)",
                file, startline, reporterror);

			return false;
		}

		itr.MoveToNext();

		auto tmpldata = itr.GetUntilNextCharacterOrNothing<std::string>('>',
            SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

		if(!tmpldata){
            
            LogError("ObjectFile template has an invalid argument list "
                "(missing the ending '>' or the instantiation is missing its parameters)",
                file, startline, reporterror);
            
			return false;
		}

		StringIterator itr3(tmpldata.get());

		// Load all the arguments //
		auto instarg = itr3.GetNextCharacterSequence<std::string>(
            UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS |
            UNNORMALCHARACTER_TYPE_LOWCODES, SPECIAL_ITERATOR_FILEHANDLING);

		itr.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

		std::vector<std::unique_ptr<std::string>> instanceargs;

		if(instarg && instarg->size()){
			instanceargs.push_back(move(instarg));
		}

		while(itr3.GetCharacter() == ',' && !itr3.IsOutOfBounds()){

			// More arguments //
			instarg = itr3.GetNextCharacterSequence<std::string>(
                UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS |
                UNNORMALCHARACTER_TYPE_LOWCODES, SPECIAL_ITERATOR_FILEHANDLING);

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
				quoteremover.ReInit(std::make_unique<UTF8DataIterator>(*instanceargs[i]));
				auto newstr = quoteremover.GetStringInQuotes<std::string>(QUOTETYPE_BOTH);
                
				if(!newstr || newstr->empty()){

					if(instanceargs[i]->size() > 2){
						reporterror->Warning("ObjectFileProcessor: Template: failed to "
                            "remove quotes from template argument, "+*instanceargs[i]);
					}
                    
					continue;
				}

				instanceargs[i] = move(newstr);
			}
		}


		// We should now be at the '>' character //
		itr.MoveToNext();

		// Create a template instantiation and add it to the file //
		auto ourval = std::make_shared<ObjectFileTemplateInstance>(*name, instanceargs);

		// Add it //
		obj.AddTemplateInstance(ourval);


		return true;
	}

	// Handle the template definition //
	if(itr.GetCharacter() != ':'){
        
        LogError("ObjectFile template definition has no ':' after name "
            "(template and following object must be separated)",
            file, startline, reporterror);

		return false;
	}

	// Skip the o //
	auto justchecking = itr.GetUntilNextCharacterOrNothing<std::string>('o',
        SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	if(!justchecking){
        
        LogError("ObjectFile template definition is missing 'o' after ':' "
            "(or maybe there is a missing space?)",
            file, startline, reporterror);
        
		return false;
	}

	// Move over it and the object should start there //
	itr.MoveToNext();

    const auto templateObjStartLine = itr.GetCurrentLine();

	// Now there should be an object definition //
	auto templatesobject = TryToLoadObject(file, itr, obj, "", reporterror);

	if(!templatesobject){
        
        LogError("ObjectFile template definition has an invalid object",
            file, templateObjStartLine, reporterror);
		return false;
	}

	// Create a template definition from the object //
	auto createdtemplate = ObjectFileTemplateDefinition::CreateFromObject(*name,
        templatesobject, templateargs);

	if(!createdtemplate){
        
        LogError("ObjectFile template failed to create from an object",
            file, startline, reporterror);
		return false;
	}

	// Now add it //
	if(!obj.AddTemplate(createdtemplate)){
        
        LogError("ObjectFile template has a conflicting name, name: \"" + *name + "\"",
            file, startline, reporterror);

		return false;
	}

	return true;
}
// ------------------------------------ //
std::shared_ptr<ObjectFileObject> Leviathan::ObjectFileProcessor::TryToLoadObject(
    const std::string &file, StringIterator &itr, ObjectFile &obj,
    const std::string &preceeding, LErrorReporter* reporterror)
{
	itr.SkipWhiteSpace(SPECIAL_ITERATOR_FILEHANDLING);

	size_t startline = itr.GetCurrentLine();

	std::vector<std::unique_ptr<std::string>> prefixesvec;
    std::unique_ptr<std::string> typesname;

	// Now there should be variable number of prefixes followed by a name //
	while(itr.GetCharacter() != '"'){

		auto oprefix = itr.GetNextCharacterSequence<std::string>(
            UNNORMALCHARACTER_TYPE_LOWCODES,
            SPECIAL_ITERATOR_FILEHANDLING);

		if(oprefix && oprefix->size()){

            if(!typesname){

                // First prefix is the type //
                typesname = std::move(oprefix);

            } else{

                prefixesvec.push_back(move(oprefix));
            }
		}

		itr.SkipWhiteSpace(SPECIAL_ITERATOR_FILEHANDLING);

		if(itr.IsOutOfBounds()){
			// Failed //
            LogError("ObjectFile object doesn't have a name, because prefixes are messed up"
				"(expected quoted std::string like this: \"o Type Prefix \'MyName\'\")",
                file, startline, reporterror);
            
			return nullptr;
		}
	}

	// Now try to get the name //
	auto oname = itr.GetStringInQuotes<std::string>(QUOTETYPE_BOTH,
        SPECIAL_ITERATOR_FILEHANDLING);

	if(!oname || !oname->size()){
        
        LogError("ObjectFile object doesn't have a name (expected quoted std::string "
            "like this: \"o Type Prefix \'MyName\'\")",
            file, startline, reporterror);
        
		return nullptr;
	}

	// There should now be a{ //
	itr.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	if(itr.GetCharacter() != '{'){
		// There is a missing brace //
        LogError("ObjectFile object is missing a '{' after its name",
            file, itr.GetCurrentLine(), reporterror);
		return nullptr;
	}

	// No need to convert the loaded utf8 std::strings //

	// Create a new ObjectFileObject to hold our contents //
    std::shared_ptr<ObjectFileObject> ourobj = std::make_shared<ObjectFileObjectProper>(
        *oname, typesname ? *typesname : "", std::move(prefixesvec));

	// Now there should be the object contents //
	itr.MoveToNext();

	while(!itr.IsOutOfBounds()){
		// First skip whitespace //
		itr.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

		// Then check the character //
		int curcharacter = itr.GetCharacter();
		size_t ourline = itr.GetCurrentLine();

		// We want to be on the next character to continue processing //
		itr.MoveToNext();

		switch(curcharacter){
		case 'l':
			{
				if(!TryToLoadVariableList(file, itr, *ourobj, startline, reporterror)){

                    LogError("ObjectFile object contains an invalid variable list",
                        file, ourline, reporterror);

					return nullptr;
				}
			}
			break;
		case 't':
			{
				if(!TryToLoadTextBlock(file, itr, *ourobj, startline, reporterror)){

                    LogError("ObjectFile object contains an invalid text block",
                        file, ourline, reporterror);

					return nullptr;
				}
			}
			break;
		case 's':
			{
				if(!TryToLoadScriptBlock(file, itr, *ourobj, startline, reporterror)){

                    LogError("ObjectFile object contains an invalid script block",
                        file, ourline, reporterror);

					return nullptr;
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
    LogError("ObjectFile object \"" + *oname + "\" is missing a closing '}' "
        "after its contents",
        file, startline, reporterror);
    
	return nullptr;
}
// ------------------------------------ //
bool Leviathan::ObjectFileProcessor::TryToLoadVariableList(const std::string &file,
    StringIterator &itr, ObjectFileObject &obj, size_t startline, LErrorReporter* reporterror)
{
	// First thing is the name //
    itr.SkipWhiteSpace();
	auto ourname = itr.GetNextCharacterSequence<std::string>(
        UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS,
		SPECIAL_ITERATOR_FILEHANDLING);

    if(ourname){
        StringOperations::RemovePreceedingTrailingSpaces(*ourname);
    }

	// Check is it valid //
	if(!ourname || ourname->size() == 0){
        
        LogError("ObjectFile variable list has an invalid name",
            file, itr.GetCurrentLine(), reporterror);

		return false;
	}

	itr.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	// There should be a '{' character here //
	if(itr.GetCharacter() != '{'){
		// There is a missing brace //
        LogError("ObjectFile variable list is missing '{' after its name",
            file, itr.GetCurrentLine(), reporterror);

		return false;
	}

	// Still on the first line //
	size_t ourstartline = itr.GetCurrentLine();

	itr.MoveToNext();

	itr.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	// Create us //
    std::unique_ptr<ObjectFileList> ourobj = std::make_unique<ObjectFileListProper>(*ourname);

	// Now we should get named variables until a } //
	while(!itr.IsOutOfBounds()){
		// First skip whitespace //
		itr.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

		if(itr.GetCharacter() == '}'){
			// Valid //
			// Skip it so the object doesn't end here //
			itr.MoveToNext();

			// Add us to the object //
			if(!obj.AddVariableList(std::move(ourobj))){
                
                LogError("ObjectFile variable list has conflicting name inside "
                    "its object",
                    file, ourstartline, reporterror);
                
				return false;
			}
            
			return true;
		}

		size_t varsline = itr.GetCurrentLine();
        
		// Try to load a named variable //
		auto loadvar = TryToLoadNamedVariables(file, itr, "", reporterror);

		if(!loadvar){
            
            LogError("ObjectFile variable list has an invalid variable",
                file, varsline, reporterror);
            
			return false;
		}

		// Add a variable to us //
		if(!ourobj->AddVariable(loadvar)){
            
            LogError("ObjectFile variable list has conflicting name inside its "
                "object, name: \""+loadvar->GetName()+"\"",
                file, varsline, reporterror);
            
			return false;
		}
	}

    LogError("ObjectFile variable list is missing the closing '}'",
        file, ourstartline, reporterror);
    
	// It failed //
	return false;
}

bool Leviathan::ObjectFileProcessor::TryToLoadTextBlock(const std::string &file,
    StringIterator &itr, ObjectFileObject &obj, size_t startline, LErrorReporter* reporterror)
{
	// First thing is the name //
    itr.SkipWhiteSpace();
	auto ourname = itr.GetNextCharacterSequence<std::string>(
        UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS,
		SPECIAL_ITERATOR_FILEHANDLING);

    if(ourname){
        
        StringOperations::RemovePreceedingTrailingSpaces(*ourname);
    }

	// Check is it valid //
	if(!ourname || ourname->size() == 0){
        
        LogError("ObjectFile variable text block has an invalid name",
            file, itr.GetCurrentLine(), reporterror);
        
		return false;
	}

	itr.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	// There should be a '{' character here //
	if(itr.GetCharacter() != '{'){
		// There is a missing brace //
        LogError("ObjectFile variable list is missing '{' after its name",
            file, startline, reporterror);

		return false;
	}

	// Still on the first line //
	size_t ourstartline = itr.GetCurrentLine();

	itr.MoveToNext();

	itr.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	// Create us //
    std::unique_ptr<ObjectFileTextBlock> ourobj =std::make_unique<ObjectFileTextBlockProper>(
        *ourname);

	// Now we should get text lines until a } //
	while(!itr.IsOutOfBounds()){
		// First skip whitespace //
		itr.SkipWhiteSpace(SPECIAL_ITERATOR_FILEHANDLING);

		if(itr.GetCharacter() == '}'){
			// Valid //

			// Skip it so the object doesn't end here //
			itr.MoveToNext();

			// Add us to the object //
			if(!obj.AddTextBlock(std::move(ourobj))){

                LogError("ObjectFile text block has a conflicting name",
                    file, ourstartline, reporterror);
                
				return false;
			}

			return true;
		}

        // If there was an empty line we will be at whitespace right now
        if(StringOperations::IsCharacterWhitespace(itr.GetCharacter())){

            // Empty line
            ourobj->AddTextLine("");
            
        } else {

            // Read a single line //
            auto linething = itr.GetUntilLineEnd<std::string>();

            // Add it to us //
            // But make sure we strip also trailing spaces
            StringOperations::RemovePreceedingTrailingSpaces(*linething);
            ourobj->AddTextLine(*linething);
        }
	}

    LogError("ObjectFile text block is missing the closing '}'",
        file, ourstartline, reporterror);
    
	// It failed //
	return false;
}

bool Leviathan::ObjectFileProcessor::TryToLoadScriptBlock(const std::string &file,
    StringIterator &itr, ObjectFileObject &obj, size_t startline, LErrorReporter* reporterror)
{
	// The line that contains the 's' //
	size_t lineforname = itr.GetCurrentLine();

	// First thing is the name //
	auto ourname = itr.GetNextCharacterSequence<std::string>(
        UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS | UNNORMALCHARACTER_TYPE_LOWCODES,
		SPECIAL_ITERATOR_FILEHANDLING);

	std::string modname;

	// Check is it valid //
	if(!ourname || ourname->size() == 0){
		// Auto generate our name //
		modname = obj.GetName()+"'s_script";
    } else{

        modname = *ourname;
    }

	itr.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	// There should be a '{' character here, or the line must have changed //
	bool linechanged = (lineforname != itr.GetCurrentLine());
	if(itr.GetCharacter() != '{' && !linechanged){
        
		// There is a missing brace //
        LogError("ObjectFile script block is missing '{' after its name",
            file, itr.GetCurrentLine(), reporterror);
        
		return false;
	}

	// Move to the next line //
	if(!linechanged)
		itr.GetUntilLineEnd<std::string>();

	// This is the line the script block starts //
	// TODO: verify that this is correct
	//size_t ourstartline = itr.GetCurrentLine()-1;
	size_t ourstartline = itr.GetCurrentLine();

	// Get until the ending sequence //
	auto scriptdata = itr.GetUntilCharacterSequence<std::string>("@%};",
        SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	if(!scriptdata){
        
        LogError("ObjectFile script block is missing the source code",
            file, ourstartline, reporterror);
        
		return false;
	}

	// Check was it terminated properly, the last character processed should be ';' //
	if(itr.GetCharacter() != ';'){

        LogError("ObjectFile script block is missing the ending sequence "
            "(\"@%};\")",
            file, ourstartline, reporterror);
        
		return false;
	}

	// Create us //
#ifdef LEVIATHAN_USING_ANGELSCRIPT
    auto ourobj = std::make_shared<ScriptScript>((ScriptExecutor::Get()->CreateNewModule(
        modname, file + "(" + Convert::ToString(ourstartline) + ")")));

    // Add the source to the script //
    auto ourmod = ourobj->GetModule();

    ourmod->AddScriptSegment(file, static_cast<int>(ourstartline), *scriptdata);
    ourmod->SetBuildState(SCRIPTBUILDSTATE_READYTOBUILD);

    // Add to the object //
    obj.AddScriptScript(ourobj);

#else
    LogError("ObjectFile has a ScriptBlock but there is no script "
        "support compiled in",
        file, ourstartline, reporterror);
#endif// LEVIATHAN_USING_ANGELSCRIPT

	return true;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ObjectFileProcessor::WriteObjectFile(ObjectFile &data,
    const std::string &file, LErrorReporter* reporterror)
{
    std::string datatowrite;

    if(!SerializeObjectFile(data, datatowrite)){

        reporterror->Error("WriteObjectFile: failed to serialize data into a std::string");
        return false;
    }

    FileSystem::WriteToFile(datatowrite, file);

	return true;
}

DLLEXPORT bool Leviathan::ObjectFileProcessor::SerializeObjectFile(ObjectFile &data,
    std::string &receiver)
{

    // Header variables //
    for (size_t i = 0; i < data.GetVariables()->GetVariableCount(); ++i){

        const auto* variable = data.GetVariables()->GetValueDirectRaw(i);

        receiver += variable->ToText() + "\n";
    }

    receiver += "\n";

    // Template definitions //
    for (size_t i = 0; i < data.GetTemplateDefinitionCount(); ++i){

        std::shared_ptr<ObjectFileTemplateDefinition> object = data.GetTemplateDefinition(i);

        LEVIATHAN_ASSERT(object, "GetTemplateDefinition iteration invalid");

        receiver += object->Serialize();
    }

    // Objects //
    for (size_t i = 0; i < data.GetTotalObjectCount(); ++i){

        std::shared_ptr<ObjectFileObject> object = data.GetObject(i);

        LEVIATHAN_ASSERT(object, "GetObject iteration invalid");

        if(object->IsThisTemplated())
            continue;

        receiver += object->Serialize();
    }

    // Template instantiations //
    for (size_t i = 0; i < data.GetTemplateInstanceCount(); ++i){

        std::shared_ptr<ObjectFileTemplateInstance> object = data.GetTemplateInstance(i);

        LEVIATHAN_ASSERT(object, "GetTemplateInstance iteration invalid");

        receiver += object->Serialize();
    }

    receiver += "\n";
    return true;
}

