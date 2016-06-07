#include "Include.h"
// ------------------------------------ //
#include "ObjectFileProcessor.h"

#include "FileSystem.h"
#include "Common/DataStoring/DataBlock.h"
#include "Common/StringOperations.h"
#include "Iterators/StringIterator.h"
#include "utf8/core.h"
#include "utf8/checked.h"

#include "ObjectFile.h"

#ifdef ALLOW_INTERNAL_EXCEPTIONS
#include "Exceptions.h"
#endif // ALLOW_INTERNAL_EXCEPTIONS

#ifdef USING_ANGELSCRIPT
#include "../Script/ScriptModule.h"
#include "Script/ScriptExecutor.h"
#include "Script/ScriptScript.h"
#endif // USING_ANGELSCRIPT
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
ObjectFileProcessor::ObjectFileProcessor(){}
Leviathan::ObjectFileProcessor::~ObjectFileProcessor(){}

#ifndef NO_DEFAULT_DATAINDEX
// quick macro to make this shorter //
#define ADDDATANAMEINTDEFINITION(x) {#x, std::shared_ptr<VariableBlock>(new VariableBlock(\
                new IntBlock(x)))}


map<std::string, std::shared_ptr<VariableBlock>> Leviathan::ObjectFileProcessor::RegisteredValues = {
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
map<std::string, std::shared_ptr<VariableBlock>> Leviathan::ObjectFileProcessor::RegisteredValues;
#endif
// ------------------------------------ //
void Leviathan::ObjectFileProcessor::Initialize(){
#if defined(_DEBUG) && !defined(NO_DEFAULT_DATAINDEX)
	// Just out of curiosity check this //
	auto iter = RegisteredValues.find(L"DATAINDEX_TICKTIME");

	if(iter == RegisteredValues.end()){

		reporterror->Error("ObjectFileProcessor: RegisteredValues are messed up, "
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
DLLEXPORT  void Leviathan::ObjectFileProcessor::RegisterValue(const std::string &name, VariableBlock* valuetokeep){
	RegisteredValues[name] = std::shared_ptr<VariableBlock>(valuetokeep);
}
// ------------------ Processing function ------------------ //
DLLEXPORT std::unique_ptr<ObjectFile> Leviathan::ObjectFileProcessor::ProcessObjectFile(
    const std::string &file, LErrorReporter* reporterror)
{
	// First read the file entirely //
	std::string filecontents;

    if (!FileSystem::ReadFileEntirely(file, filecontents)) {

        reporterror->Error("ObjectFileProcessor: ProcessObjectFile: file could not be read");
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

DLLEXPORT std::unique_ptr<Leviathan::ObjectFile> ObjectFileProcessor::ProcessObjectFileFromString(
    std::string filecontents, const std::string &filenameforerrors, LErrorReporter* reporterror) 
{
    // Create the target object //
    unique_ptr<ObjectFile> ofile(new ObjectFile());

    bool succeeded = true;

    // Create an UTF8 supporting iterator //
    StringIterator itr(new UTF8DataIterator(filecontents), true);

    while (!itr.IsOutOfBounds()) {
        // First get the first thing defining what the following object/thing will be //
        auto thingtype = itr.GetNextCharacterSequence<string>(UNNORMALCHARACTER_TYPE_LOWCODES |
            UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS, SPECIAL_ITERATOR_FILEHANDLING);

        if (!thingtype)
            continue;

        // Store the starting line for error reporting purposes //
        size_t thisstart = itr.GetCurrentLine();

        if (*thingtype == "template") {
            // Either a template definition or a template instantiation //

            if (!TryToHandleTemplate(filenameforerrors, itr, *ofile, *thingtype, reporterror)) {

                reporterror->Error("ObjectFileProcessor: processing a template definitions "
                    "or instantiation has failed, file: " + filenameforerrors + "(" +
                    Convert::ToString(thisstart) + ")");

                succeeded = false;
                break;
            }

            continue;

        } else if (*thingtype == "o") {

            // Process an object //
            auto tmpobj = TryToLoadObject(filenameforerrors, itr, *ofile, *thingtype, reporterror);

            if (!tmpobj) {

                reporterror->Error("ObjectFileProcessor: processing an object has failed, file: " +
                    filenameforerrors + "(" + Convert::ToString(thisstart) + ")");
                succeeded = false;
                break;
            }

            if (!ofile->AddObject(tmpobj)) {

                reporterror->Error("ObjectFileProcessor: object has a conflicting name, name: \"" + tmpobj->GetName() +
                    "\", file: " + filenameforerrors + "(" + Convert::ToString(thisstart) + "), current line: " +
                    Convert::ToString(itr.GetCurrentLine()));

                succeeded = false;
                break;
            }


            continue;

        } else {
            // It should be a named variable //

            auto ptr = TryToLoadNamedVariables(filenameforerrors, itr, *thingtype, reporterror);
            if (!ptr) {

                reporterror->Error("ObjectFileProcessor: processing a NamedVariableList has failed, file: " +
                    filenameforerrors + "(" + Convert::ToString(thisstart) + ")");
                succeeded = false;
                break;
            }

            // Add to the object //
            if (!ofile->AddNamedVariable(ptr)) {

                reporterror->Error("ObjectFileProcessor: variable name already in use, file: "
                    "" + filenameforerrors + "(" + Convert::ToString(thisstart) + "):");
                return NULL;
            }

            continue;
        }

        // It is something that cannot be handled //
        reporterror->Error("ObjectFile has an invalid block (" + *thingtype + "), file: " + filenameforerrors
            + "(" + Convert::ToString(thisstart) + ")");

        // The file is clearly malformed //
        succeeded = false;
        break;
    }


    if (!succeeded || !itr.IsOutOfBounds()) {

        // It failed //
        reporterror->Error("ObjectFileProcessor: could not parse file: " + filenameforerrors + " parsing has ended on line: " +
            Convert::ToString(itr.GetCurrentLine()));
        return NULL;
    }

    // Generate the template instantiations and it's done //
    if (!ofile->GenerateTemplatedObjects(reporterror)) {

        reporterror->Error("ObjectFileProcessor: file has invalid templates (either bad names, "
            "or instances without definitions), file: " + filenameforerrors);

        return NULL;
    }

    return ofile;
}

// ------------------------------------ //
shared_ptr<NamedVariableList> Leviathan::ObjectFileProcessor::TryToLoadNamedVariables(const std::string &file,
    StringIterator &itr, const string &preceeding, LErrorReporter* reporterror)
{
	// Try to load a named variable of format: "Variable = myvalue;" //

    // Store the beginning line //
    size_t startline = itr.GetCurrentLine();
    
	// Next thing after the preceeding is rest of the name until the '=' character //

	// This is often empty //
	auto restofname = itr.GetUntilEqualityAssignment<string>(EQUALITYCHARACTER_TYPE_ALL, SPECIAL_ITERATOR_FILEHANDLING);

	if(!restofname && preceeding.size() == 0){
		// No name //
		reporterror->Error("ObjectFile named variable is malformed, unknown block?, file: "+file+":"+
            Convert::ToString(startline)+"-"+
			Convert::ToString(itr.GetCurrentLine())+")");
		return NULL;
	}

	// There needs to be a separator, the last character should be it //
	int lastchar = itr.GetPreviousCharacter();

	if(lastchar != '=' && lastchar != ':'){

		// Invalid format //
		reporterror->Error("ObjectFile block isn't a named variable, unknown block?, file: "+file+"("+
            Convert::ToString(itr.GetCurrentLine())+")");
		return NULL;
	}

	// We need to skip whitespace //
	itr.SkipWhiteSpace(SPECIAL_ITERATOR_FILEHANDLING);

	// Get the value //
	auto valuestr = itr.GetUntilNextCharacterOrAll<string>(';', 
        SPECIAL_ITERATOR_ONNEWLINE_STOP | SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	if(!valuestr){
		// No ';' or nothing after the equals sign //
		reporterror->Error("ObjectFile named variable is empty or missing an (optional) ending ';' "
            "(should be like: \"MyVar = 42;\"), file: "+file+"("+Convert::ToString(startline)+")");
		return NULL;
	}

	// Skip the ';' //
	itr.MoveToNext();

	// Try to construct a named variable //

	// Put the name together //
	string varname = preceeding+ (restofname ? *restofname: string());

	// Try to use the name and the variable string to try to create one named variable //

	try{

        // NamedVariableList now uses UTF8 so conversion is no longer required //

		// This was a valid definition //
		return make_shared<NamedVariableList>(varname, *valuestr, reporterror, &RegisteredValues);

	} catch(const InvalidArgument &e){

		reporterror->Error("ObjectFileProcessor: named variable parse failed, file: "+
            file+"("+Convert::ToString(startline)+"):");
		e.Print(reporterror);
		return NULL;
	}
}
// ------------------------------------ //
bool Leviathan::ObjectFileProcessor::TryToHandleTemplate(const std::string &file, StringIterator &itr, ObjectFile &obj,
    const string &preceeding, LErrorReporter* reporterror)
{
	// Skip potential space between 'template' and '<' //
	itr.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	// Get to the first character //
	if(itr.GetCharacter(0) != '<'){

		reporterror->Error("ObjectFile template has a missing '<' after 'template', file: "+file+"("+
			Convert::ToString(itr.GetCurrentLine())+")");
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

			reporterror->Error("ObjectFile template has an invalid argument list (missing the ending '>') , file: "+
                file+"("+Convert::ToString(startline)+")");
			return false;
		}

		StringIterator itr2(tmpldata.get());

		auto tmplarg = itr2.GetNextCharacterSequence<string>(UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS |
            UNNORMALCHARACTER_TYPE_LOWCODES,
			SPECIAL_ITERATOR_FILEHANDLING);

		// Go somewhere proper //
		itr2.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

		if(tmplarg && tmplarg->size()){
			templateargs.push_back(move(tmplarg));
		}

		while(itr2.GetCharacter() == ',' && !itr2.IsOutOfBounds()){

			// More arguments //
			tmplarg = itr2.GetNextCharacterSequence<string>(UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS |
                UNNORMALCHARACTER_TYPE_LOWCODES,
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

				reporterror->Warning("ObjectFileProcessor: Template: failed to remove quotes "
                    "from template argument, "+*templateargs[i]);
                
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
	auto name = itr.GetNextCharacterSequence<string>(UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS |
        UNNORMALCHARACTER_TYPE_LOWCODES, SPECIAL_ITERATOR_FILEHANDLING);

	if(!name || name->size() < 3){

		reporterror->Error("ObjectFile template has too short name (has to be a minimum of 3 "
            "characters), file: "+file+"("+Convert::ToString(itr.GetCurrentLine())+")");
		return false;
	}

	if(!templateargs.size()){
		// This is a template instantiation //

		// Next should be the argument list //
		itr.SkipWhiteSpace(SPECIAL_ITERATOR_FILEHANDLING);

		// We should now be at the '<' character //
		if(itr.GetCharacter() != '<'){

			reporterror->Error("ObjectFile template instance has an invalid argument list "
                "(missing starting '<' after name) , file: "+file+
                "("+Convert::ToString(startline)+")");
			return false;
		}

		itr.MoveToNext();

		auto tmpldata = itr.GetUntilNextCharacterOrNothing<string>('>',
            SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

		if(!tmpldata){

			reporterror->Error("ObjectFile template has an invalid argument list "
                "(missing the ending '>' or the instantiation is missing it's parameters) , "
                "file: "+file+"("+Convert::ToString(startline)+")");
			return false;
		}

		StringIterator itr3(tmpldata.get());


		// Load all the arguments //
		auto instarg = itr3.GetNextCharacterSequence<string>(
            UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS |
            UNNORMALCHARACTER_TYPE_LOWCODES, SPECIAL_ITERATOR_FILEHANDLING);

		itr.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

		std::vector<unique_ptr<string>> instanceargs;

		if(instarg && instarg->size()){
			instanceargs.push_back(move(instarg));
		}

		while(itr3.GetCharacter() == ',' && !itr3.IsOutOfBounds()){

			// More arguments //
			instarg = itr3.GetNextCharacterSequence<string>(
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
				quoteremover.ReInit(new UTF8DataIterator(*instanceargs[i]), true);
				auto newstr = quoteremover.GetStringInQuotes<string>(QUOTETYPE_BOTH);
                
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
		auto ourval = make_shared<ObjectFileTemplateInstance>(*name, instanceargs);

		// Add it //
		obj.AddTemplateInstance(ourval);


		return true;
	}

	// Handle the template definition //
	if(itr.GetCharacter() != ':'){

		reporterror->Error("ObjectFile template definition has no ':' after name "
            "(template and following object must be separated), file: "+
            file+"("+Convert::ToString(startline)+")");
		return false;
	}

	// Skip the o //
	auto justchecking = itr.GetUntilNextCharacterOrNothing<string>('o',
        SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	if(!justchecking){

		reporterror->Error("ObjectFile template definition is missing 'o' after ':' "
            "(or maybe there is a missing space?), file: "+
            file+"("+Convert::ToString(startline)+")");
		return false;
	}

	// Move over it and the object should start there //
	itr.MoveToNext();

	// Now there should be an object definition //
	auto templatesobject = TryToLoadObject(file, itr, obj, "", reporterror);

	if(!templatesobject){

		reporterror->Error("ObjectFile template definition has an invalid object, "
			"file: "+file+"("+Convert::ToString(startline)+")");
		return false;
	}

	// Create a template definition from the object //
	auto createdtemplate = ObjectFileTemplateDefinition::CreateFromObject(*name, templatesobject,
        templateargs);

	if(!createdtemplate){

		reporterror->Error("ObjectFile template failed to create from an object, file: "+
            file+"("+Convert::ToString(startline)+")");
		return false;
	}


	// Now add it //
	if(!obj.AddTemplate(createdtemplate)){


		reporterror->Error("ObjectFile template has a conflicting name, name: \""+
            *name+"\", ""file: "+file+"("+Convert::ToString(startline)+")");
		return false;
	}


	return true;
}
// ------------------------------------ //
shared_ptr<ObjectFileObject> Leviathan::ObjectFileProcessor::TryToLoadObject(
    const std::string &file, StringIterator &itr, ObjectFile &obj,  const string &preceeding, 
    LErrorReporter* reporterror)
{
	auto typesname = itr.GetNextCharacterSequence<string>(
        UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS |
        UNNORMALCHARACTER_TYPE_LOWCODES, SPECIAL_ITERATOR_FILEHANDLING);

	if(!typesname || !typesname->size()){

		reporterror->Error("ObjectFile object definition has no typename "
            "(or anything valid, for that matter, after 'o'), file: "+file+"("
			+Convert::ToString(itr.GetCurrentLine())+")");
		return NULL;
	}

	itr.SkipWhiteSpace(SPECIAL_ITERATOR_FILEHANDLING);

	size_t startline = itr.GetCurrentLine();

	std::vector<unique_ptr<string>> prefixesvec;

	// Now there should be variable number of prefixes followed by a name //
	while(itr.GetCharacter() != '"'){

		auto oprefix = itr.GetNextCharacterSequence<string>(UNNORMALCHARACTER_TYPE_LOWCODES,
            SPECIAL_ITERATOR_FILEHANDLING);

		if(oprefix && oprefix->size()){
			prefixesvec.push_back(move(oprefix));
		}

		itr.SkipWhiteSpace(SPECIAL_ITERATOR_FILEHANDLING);

		if(itr.IsOutOfBounds()){
			// Failed //
			reporterror->Error("ObjectFile object doesn't have a name, because prefixes are messed up"
				"(expected quoted string like this: \"o Type Prefix \'MyName\'\")"
				", started file: "+file+"("+Convert::ToString(startline)+")");
			return NULL;
		}
	}

	// Now try to get the name //
	auto oname = itr.GetStringInQuotes<string>(QUOTETYPE_BOTH, SPECIAL_ITERATOR_FILEHANDLING);

	if(!oname || !oname->size()){

		reporterror->Error("ObjectFile object doesn't have a name (expected quoted string "
            "like this: \"o Type Prefix \'MyName\'\"), started file: "+
            file+"("+Convert::ToString(startline)+")");
		return NULL;
	}

	// There should now be a { //
	itr.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	if(itr.GetCharacter() != '{'){
		// There is a missing brace //

		reporterror->Error("ObjectFile object is missing a '{' after it's name, file: "+
            file+"("+Convert::ToString(itr.GetCurrentLine())+")");
		return NULL;
	}

	// No need to convert the loaded utf8 strings //

	// Create a new ObjectFileObject to hold our contents //
	shared_ptr<ObjectFileObject> ourobj = make_shared<ObjectFileObjectProper>(*oname,
        *typesname, std::move(prefixesvec));

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
				if(!TryToLoadVariableList(file, itr, *ourobj, startline, reporterror)){

					reporterror->Error("ObjectFile object contains an invalid variable list, file: "+
                        file+"("+Convert::ToString(ourline)+")");
					return NULL;
				}
			}
			break;
		case 't':
			{
				if(!TryToLoadTextBlock(file, itr, *ourobj, startline, reporterror)){

					reporterror->Error("ObjectFile object contains an invalid text block, file: "+
                        file+"("+Convert::ToString(ourline)+")");
					return NULL;
				}
			}
			break;
		case 's':
			{
				if(!TryToLoadScriptBlock(file, itr, *ourobj, startline, reporterror)){

					reporterror->Error("ObjectFile object contains an invalid script block, file: "+
                        file+"("+Convert::ToString(ourline)+")");
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
	reporterror->Error("ObjectFile object is missing a closing '}' after it's contents, "
        "file: "+file+"("+Convert::ToString(startline)+")");
	return NULL;
}
// ------------------------------------ //
bool Leviathan::ObjectFileProcessor::TryToLoadVariableList(const std::string &file,
    StringIterator &itr, ObjectFileObject &obj, size_t startline, LErrorReporter* reporterror)
{
	// First thing is the name //
	auto ourname = itr.GetNextCharacterSequence<string>(
        UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS | UNNORMALCHARACTER_TYPE_LOWCODES,
		SPECIAL_ITERATOR_FILEHANDLING);

	// Check is it valid //
	if(!ourname || ourname->size() == 0){

		reporterror->Error("ObjectFile variable list has an invalid name, file: "+
            file+"("+Convert::ToString(itr.GetCurrentLine())+")");
		return false;
	}

	itr.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	// There should be a '{' character here //
	if(itr.GetCharacter() != '{'){
		// There is a missing brace //

		reporterror->Error("ObjectFile variable list is missing '{' after it's name, file: "+
            file+"("+Convert::ToString(itr.GetCurrentLine())+")");
		return false;
	}

	// Still on the first line //
	size_t ourstartline = itr.GetCurrentLine();

	itr.MoveToNext();

	itr.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	// Create us //
	unique_ptr<ObjectFileList> ourobj(new ObjectFileListProper(*ourname));

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

				reporterror->Error("ObjectFile variable list has conflicting name inside "
                    "it's object, file: "+file+"("+Convert::ToString(ourstartline)+")");
				return false;
			}
            
			return true;
		}

		size_t varsline = itr.GetCurrentLine();
		// Try to load a named variable //
		auto loadvar = TryToLoadNamedVariables(file, itr, "", reporterror);

		if(!loadvar){

			reporterror->Error("ObjectFile variable list has an invalid variable, file: "
                +file+"("+Convert::ToString(varsline)+")");
			return false;
		}

		// Add a variable to us //
		if(!ourobj->AddVariable(loadvar)){

			reporterror->Error("ObjectFile variable list has conflicting name inside it's "
                "object, name: \""+loadvar->GetName()+"\", file: "+file+"("+
                Convert::ToString(itr.GetCurrentLine())+")");
			return false;
		}
	}


	reporterror->Error("ObjectFile variable list is missing the closing '}', file: "+
        file+"("+Convert::ToString(ourstartline)+")");
	// It failed //
	return false;
}

bool Leviathan::ObjectFileProcessor::TryToLoadTextBlock(const std::string &file,
    StringIterator &itr, ObjectFileObject &obj, size_t startline, LErrorReporter* reporterror)
{
	// First thing is the name //
	auto ourname = itr.GetNextCharacterSequence<string>(
        UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS | UNNORMALCHARACTER_TYPE_LOWCODES,
		SPECIAL_ITERATOR_FILEHANDLING);

	// Check is it valid //
	if(!ourname || ourname->size() == 0){

		reporterror->Error("ObjectFile variable text block has an invalid name, file: "+
            file+"("+Convert::ToString(itr.GetCurrentLine())+")");
		return false;
	}

	itr.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	// There should be a '{' character here //
	if(itr.GetCharacter() != '{'){
		// There is a missing brace //

		reporterror->Error("ObjectFile variable list is missing '{' after it's name, file: "+
            file+"("+Convert::ToString(itr.GetCurrentLine())+")");
		return false;
	}

	// Still on the first line //
	size_t ourstartline = itr.GetCurrentLine();

	itr.MoveToNext();

	itr.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	// Create us //
	unique_ptr<ObjectFileTextBlock> ourobj(new ObjectFileTextBlockProper(*ourname));

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

				reporterror->Error("ObjectFile text block has a conflicting name, file: "+
                    file+"("+Convert::ToString(ourstartline)+")");
				return false;
			}

			return true;
		}

		// Read a single line //
		auto linething = itr.GetUntilLineEnd<string>();

		// Add it to us //
		ourobj->AddTextLine(*linething);
	}


	reporterror->Error("ObjectFile text block is missing the closing '}', file: "+
        file+"("+Convert::ToString(ourstartline)+")");
	// It failed //
	return false;
}

bool Leviathan::ObjectFileProcessor::TryToLoadScriptBlock(const std::string &file,
    StringIterator &itr, ObjectFileObject &obj, size_t startline, LErrorReporter* reporterror)
{
	// The line that contains the 's' //
	size_t lineforname = itr.GetCurrentLine();

	// First thing is the name //
	auto ourname = itr.GetNextCharacterSequence<string>(
        UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS | UNNORMALCHARACTER_TYPE_LOWCODES,
		SPECIAL_ITERATOR_FILEHANDLING);

	std::string modname;

	// Check is it valid //
	if(!ourname || ourname->size() == 0){
		// Auto generate our name //
		modname = obj.GetName()+"'s_script";
    } else {

        modname = *ourname;
    }

	itr.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	// There should be a '{' character here, or the line must have changed //
	bool linechanged = (lineforname != itr.GetCurrentLine());
	if(itr.GetCharacter() != '{' && !linechanged){
		// There is a missing brace //

		reporterror->Error("ObjectFile script block is missing '{' after it's name, file: "+
            file+"("+Convert::ToString(itr.GetCurrentLine())+")");
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
	auto scriptdata = itr.GetUntilCharacterSequence<string>("@%};",
        SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	if(!scriptdata){

		reporterror->Error("ObjectFile script block is missing the source code, file: "+
            file+"("+Convert::ToString(ourstartline)+")");
		return false;
	}

	// Check was it terminated properly, the last character processed should be ';' //
	if(itr.GetCharacter() != ';'){

		reporterror->Error("ObjectFile script block is missing the ending sequence "
            "(\"@%};\"), file: "+file+"("+Convert::ToString(ourstartline)+")");
		return false;
	}

	// Create us //
#ifdef LEVIATHAN_USING_ANGELSCRIPT
    shared_ptr<ScriptScript> ourobj(new ScriptScript(ScriptExecutor::Get()->CreateNewModule(
        modname, file + "(" + Convert::ToString(ourstartline) + ")")));

    // Add the source to the script //
    auto ourmod = ourobj->GetModule();


    ourmod->AddScriptSegment(file, ourstartline, *scriptdata);
    ourmod->SetBuildState(SCRIPTBUILDSTATE_READYTOBUILD);

    // Add to the object //
    obj.AddScriptScript(ourobj);

#else
    reporterror->Error("ObjectFile has a ScriptBlock but there is no script support compiled in");
#endif // LEVIATHAN_USING_ANGELSCRIPT

	return true;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ObjectFileProcessor::WriteObjectFile(ObjectFile &data,
    const std::string &file, LErrorReporter* reporterror)
{

	return false;
}

