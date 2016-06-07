#include "Include.h"
// ------------------------------------ //
#include "ObjectFile.h"

#include "Common/StringOperations.h"
#include "utf8/checked.h"
#if !defined(ALTERNATIVE_EXCEPTIONS_FATAL) || defined(ALLOW_INTERNAL_EXCEPTIONS)
#include "Exceptions.h"
#endif
#ifdef USING_ANGELSCRIPT
#include "../Script/ScriptExecutor.h"
#include "../Script/ScriptModule.h"
#endif // USING_ANGELSCRIPT
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::ObjectFile::ObjectFile(NamedVars &stealfrom) : HeaderVars(&stealfrom){

}

DLLEXPORT Leviathan::ObjectFile::ObjectFile(){

}

DLLEXPORT Leviathan::ObjectFile::~ObjectFile(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ObjectFile::AddNamedVariable(shared_ptr<NamedVariableList> var){
	// Make sure that name is not in use //
	if(HeaderVars.Find(var->GetName()) < HeaderVars.GetVariableCount()){
        
		return false;
	}

	// Add it //
	HeaderVars.AddVar(var);
	return true;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ObjectFile::AddObject(shared_ptr<ObjectFileObject> obj){
	// Make sure that the name is not in use //
	if(IsObjectNameInUse(obj->GetName())){

		return false;
	}

	// Add the object //
	DefinedObjects.push_back(obj);
	return true;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ObjectFile::IsObjectNameInUse(const std::string &name) const{
	// Try to find an object with the same name //
	for(size_t i = 0; i < DefinedObjects.size(); i++){

		if(DefinedObjects[i]->GetName() == name)
			return true;
	}

	// Check for matching template names //


	// Didn't match anything //
	return false;
}
// ------------------------------------ //
DLLEXPORT NamedVars* Leviathan::ObjectFile::GetVariables(){
	return &HeaderVars;
}
// ------------------------------------ //
DLLEXPORT size_t Leviathan::ObjectFile::GetTotalObjectCount() const{
	// Add the template objects to actual objects //
	return DefinedObjects.size();
}

DLLEXPORT ObjectFileObject* Leviathan::ObjectFile::GetObjectFromIndex(size_t index) const{
	// Return from DefinedObjects if it is in range otherwise from the template instances //
	if(index < DefinedObjects.size()){

		return DefinedObjects[index].get();
	}

#ifndef ALTERNATIVE_EXCEPTIONS_FATAL
    // Invalid index //
    throw InvalidArgument("index is out of range");
#else
    return nullptr;
#endif //ALTERNATIVE_EXCEPTIONS_FATAL
}
// ------------------------------------ //
DLLEXPORT ObjectFileObject* Leviathan::ObjectFile::GetObjectWithType(const std::string &typestr) const{
	// Find the first that matches the type //
	for(size_t i = 0; i < DefinedObjects.size(); i++){

		if(DefinedObjects[i]->GetTypeName() == typestr)
			return DefinedObjects[i].get();
	}

	// Nothing found //
	return NULL;
}

DLLEXPORT std::vector<ObjectFileObject*> Leviathan::ObjectFile::GetAllObjectsWithType(const std::string &type) const {

    std::vector<ObjectFileObject*> result;

    for (size_t i = 0; i < DefinedObjects.size(); i++) {

        if (DefinedObjects[i]->GetTypeName() == type)
            result.push_back((DefinedObjects[i].get()));
    }

    return result;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ObjectFile::GenerateTemplatedObjects(LErrorReporter* reporterror)
{
	// Create template instances from the templates //
	for(size_t i = 0; i < TemplateInstantiations.size(); i++){

		// Find a template definition for this //
		auto tmpldef = FindTemplateDefinition(TemplateInstantiations[i]->GetNameOfParentTemplate());

		if(!tmpldef){

            reporterror->Error("ObjectFile: could not find template definition with name: "
				+TemplateInstantiations[i]->GetNameOfParentTemplate());
			return false;
		}

		// Try to generate it //
		auto resultobj = tmpldef->CreateInstanceFromThis(*TemplateInstantiations[i], reporterror);

		if(!resultobj){

            reporterror->Error("ObjectFile: could not instantiate template "
                "(parameter count probably didn't match), name: "
				+TemplateInstantiations[i]->GetNameOfParentTemplate());
			return false;
		}

		shared_ptr<ObjectFileObject> tmpobj(resultobj.release());

		// Add it to us //
		if(!AddObject(tmpobj)){
			
            reporterror->Error("ObjectFile: template instance's result was an object whose "
                "name is already in use, template name: "
				+TemplateInstantiations[i]->GetNameOfParentTemplate()+", result object: "+
                tmpobj->GetName());
			return false;
		}
	}


	// No errors occurred //
	return true;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ObjectFile::AddTemplateInstance(
    shared_ptr<ObjectFileTemplateInstance> tiobj)
{
	// Template instances may have names that are not present yet, the instance can be before the definition //
	TemplateInstantiations.push_back(tiobj);
}

DLLEXPORT bool Leviathan::ObjectFile::AddTemplate(
    shared_ptr<ObjectFileTemplateDefinition> templatedef)
{
	// Make sure the name is unique //
	for(size_t i = 0; i < TemplateDefinitions.size(); i++){

		if(TemplateDefinitions[i]->GetName() == templatedef->GetName()){

			// Conflicting name //
			return false;
		}
	}

	// Add it //
	TemplateDefinitions.push_back(templatedef);
	return true;
}

DLLEXPORT std::shared_ptr<ObjectFileTemplateDefinition> Leviathan::ObjectFile::FindTemplateDefinition(
    const string &name) const
{
	// Try to find one with the exact name //
	for(size_t i = 0; i < TemplateDefinitions.size(); i++){

		if(TemplateDefinitions[i]->GetName() == name){

			// Found a match //
			return TemplateDefinitions[i];
		}
	}

	// Nothing found //
	return NULL;
}

// ------------------ ObjectFileListProper ------------------ //
DLLEXPORT Leviathan::ObjectFileListProper::ObjectFileListProper(const std::string &name) :
    Name(name) {

}
// ------------------------------------ //
DLLEXPORT const std::string& Leviathan::ObjectFileListProper::GetName() const {
    return Name;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ObjectFileListProper::AddVariable(shared_ptr<NamedVariableList> var) {
    // Make sure that name is not in use //
    if (Variables.Find(var->GetName()) < Variables.GetVariableCount()) {
        return false;
    }

    // Add it //
    Variables.AddVar(var);
    return true;
}
// ------------------------------------ //
DLLEXPORT NamedVars& Leviathan::ObjectFileListProper::GetVariables() {
    return Variables;
}
// ------------------ ObjectFileList ------------------ //
DLLEXPORT Leviathan::ObjectFileList::~ObjectFileList() {

}

DLLEXPORT Leviathan::ObjectFileObjectProper::ObjectFileObjectProper(const std::string &name,
    const std::string &typesname, std::vector<std::unique_ptr<std::string>>&& prefix) :
    Name(name), TName(typesname), Prefixes(std::move(prefix)) 
{

}

DLLEXPORT Leviathan::ObjectFileObjectProper::~ObjectFileObjectProper() {
    // Let the script go //
    Script.reset();

    // Release memory //
    SAFE_DELETE_VECTOR(Contents);
    SAFE_DELETE_VECTOR(TextBlocks);
}
// ------------------------------------ //
DLLEXPORT const std::string& Leviathan::ObjectFileObjectProper::GetName() const {
    return Name;
}

DLLEXPORT const std::string& Leviathan::ObjectFileObjectProper::GetTypeName() const {
    return TName;
}

DLLEXPORT std::shared_ptr<ScriptScript> Leviathan::ObjectFileObjectProper::GetScript() const {
    return Script;
}

DLLEXPORT size_t Leviathan::ObjectFileObjectProper::GetPrefixesCount() const {
    return Prefixes.size();
}

DLLEXPORT const std::string& Leviathan::ObjectFileObjectProper::GetPrefix(size_t index) const {
    // Check the index //
    if (index >= Prefixes.size()) {

    #ifndef ALTERNATIVE_EXCEPTIONS_FATAL
        throw InvalidArgument("index is out of range");
    #else
        LEVIATHAN_ASSERT(0, "index is out of range");
    #endif
    }

    return *Prefixes[index];
}

DLLEXPORT const std::string* Leviathan::ObjectFileObjectProper::GetPrefixPtr(size_t index) const {

    if (index >= Prefixes.size()) {

        return nullptr;
    }

    return Prefixes[index].get();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ObjectFileObjectProper::AddVariableList(unique_ptr<ObjectFileList> &list) {
    // Make sure name is unique //
    for (size_t i = 0; i < Contents.size(); i++) {

        if (Contents[i]->GetName() == list->GetName()) {
            return false;
        }
    }

    // Add it //
    Contents.push_back(list.release());
    return true;
}

DLLEXPORT bool Leviathan::ObjectFileObjectProper::AddTextBlock(unique_ptr<ObjectFileTextBlock> &tblock) {
    // Make sure name is unique //
    for (size_t i = 0; i < TextBlocks.size(); i++) {

        if (TextBlocks[i]->GetName() == tblock->GetName()) {
            return false;
        }
    }

    // Add it //
    TextBlocks.push_back(tblock.release());
    return true;
}

DLLEXPORT void Leviathan::ObjectFileObjectProper::AddScriptScript(shared_ptr<ScriptScript> script) {
    // Warn if we already have a script //
    if (Script) {
    #ifndef LEVIATHAN_UE_PLUGIN
        Logger::Get()->Warning("ObjectFileObject: already has a script, overwrote old one, name: "
            + Name);
    #endif
    }

    Script = script;
}
// ------------------------------------ //
DLLEXPORT ObjectFileList* Leviathan::ObjectFileObjectProper::GetListWithName(const std::string &name) const {
    // Loop and compare names //
    for (size_t i = 0; i < Contents.size(); i++) {

        if (Contents[i]->GetName() == name)
            return Contents[i];
    }

    // Not found //
    return NULL;
}

DLLEXPORT ObjectFileTextBlock* Leviathan::ObjectFileObjectProper::GetTextBlockWithName(const std::string &name) const {
    // Loop and compare names //
    for (size_t i = 0; i < TextBlocks.size(); i++) {

        if (TextBlocks[i]->GetName() == name)
            return TextBlocks[i];
    }

    // Not found //
    return NULL;
}

DLLEXPORT size_t Leviathan::ObjectFileObjectProper::GetListCount() const {
    return Contents.size();
}

DLLEXPORT ObjectFileList* Leviathan::ObjectFileObjectProper::GetList(size_t index) const {
    // Check the index //
    if (index >= Contents.size()) {

    #ifndef ALTERNATIVE_EXCEPTIONS_FATAL
        throw InvalidArgument("index is out of range");
    #else
        return nullptr;
    #endif
    }

    return Contents[index];
}

DLLEXPORT size_t Leviathan::ObjectFileObjectProper::GetTextBlockCount() const {
    return TextBlocks.size();
}

DLLEXPORT ObjectFileTextBlock* Leviathan::ObjectFileObjectProper::GetTextBlock(size_t index) const {
    // Check the index //
    if (index >= TextBlocks.size()) {

    #ifndef ALTERNATIVE_EXCEPTIONS_FATAL
        throw InvalidArgument("index is out of range");
    #else
        return nullptr;
    #endif
    }

    return TextBlocks[index];
}

DLLEXPORT bool Leviathan::ObjectFileObjectProper::IsThisTemplated() const {
    return false;
}

// ------------------ ObjectFileObject ------------------ //
DLLEXPORT Leviathan::ObjectFileObject::~ObjectFileObject() {

}

DLLEXPORT Leviathan::ObjectFileTextBlockProper::ObjectFileTextBlockProper(const string &name) :
    Name(name) {

}

DLLEXPORT Leviathan::ObjectFileTextBlockProper::~ObjectFileTextBlockProper() {
    SAFE_DELETE_VECTOR(Lines);
}
// ------------------------------------ //
DLLEXPORT const string& Leviathan::ObjectFileTextBlockProper::GetName() const {
    return Name;
}
// ------------------------------------ //
DLLEXPORT size_t Leviathan::ObjectFileTextBlockProper::GetLineCount() const {
    return Lines.size();
}

DLLEXPORT const string& Leviathan::ObjectFileTextBlockProper::GetLine(size_t index) const {
    // Check the index //
    if (index >= Lines.size()) {
    #ifndef ALTERNATIVE_EXCEPTIONS_FATAL
        throw InvalidArgument("index is out of range");
    #else
        LEVIATHAN_ASSERT(false, "index is over Lines.size()");
    #endif
    }

    return *Lines[index];
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ObjectFileTextBlockProper::AddTextLine(const string &line) {
    Lines.push_back(new string(line));
}

// ------------------ ObjectFileTextBlock ------------------ //
DLLEXPORT Leviathan::ObjectFileTextBlock::~ObjectFileTextBlock() {

}

DLLEXPORT Leviathan::ObjectFileTemplateDefinition::ObjectFileTemplateDefinition(const string &name,
    std::vector<unique_ptr<string>> &parameters, std::shared_ptr<ObjectFileObject> obj) :
    Name(name), Parameters(move(parameters)), RepresentingObject(obj) {

}
// ------------------------------------ //
DLLEXPORT const string& Leviathan::ObjectFileTemplateDefinition::GetName() const {
    return Name;
}
// ------------------------------------ //
DLLEXPORT  std::shared_ptr<ObjectFileTemplateDefinition>
Leviathan::ObjectFileTemplateDefinition::CreateFromObject(const string &name,
    std::shared_ptr<ObjectFileObject> obj, std::vector<unique_ptr<string>> &templateargs) {
    // This could be changed to a function that tears down the object and creates mess
    // of templating objects
    auto resultobj = make_shared<ObjectFileTemplateDefinition>(name, templateargs, obj);

    return resultobj;
}

DLLEXPORT std::unique_ptr<Leviathan::ObjectFileTemplateObject> ObjectFileTemplateDefinition::CreateInstanceFromThis(
    const ObjectFileTemplateInstance &instanceargs, LErrorReporter* reporterror /*= nullptr*/) {
    // First make sure that template counts match, return NULL otherwise //
    if (Parameters.size() != instanceargs.Arguments.size()) {

        return NULL;
    }


    // Make sure these are not templated //
    std::string newname = RepresentingObject->GetName();
    std::string newtype = RepresentingObject->GetTypeName();

    std::vector<std::unique_ptr<std::string>> newprefixes;

    ReplaceStringWithTemplateArguments(newname, instanceargs.Arguments);


    ReplaceStringWithTemplateArguments(newtype, instanceargs.Arguments);

    for (size_t i = 0; i < RepresentingObject->GetPrefixesCount(); i++) {

        unique_ptr<std::string> newprefix(new std::string(RepresentingObject->GetPrefix(i)));

        ReplaceStringWithTemplateArguments(*newprefix, instanceargs.Arguments);

        newprefixes.push_back(std::move(newprefix));
    }


    // We somehow need to detect all places that might have the template parameters and
    // change them
    unique_ptr<ObjectFileTemplateObject> internalobj(new ObjectFileTemplateObject(newname,
        newtype, std::move(newprefixes)));

    // Do the contents the same way //
    for (size_t i = 0; i < RepresentingObject->GetListCount(); i++) {

        // Copy the data from the list replacing all the templates with values //
        ObjectFileList* curlist = RepresentingObject->GetList(i);

        std::string actlistname = curlist->GetName();

        ReplaceStringWithTemplateArguments(actlistname, instanceargs.Arguments);

        unique_ptr<ObjectFileList> listobj(new ObjectFileListProper(actlistname));

        const std::vector<shared_ptr<NamedVariableList>>& vallist =
            *curlist->GetVariables().GetVec();

        // Add the values replacing template arguments //
        for (size_t a = 0; a < vallist.size(); a++) {

            // First check the name //
            std::string newvarlistname = vallist[a]->GetName();

            ReplaceStringWithTemplateArguments(newvarlistname, instanceargs.Arguments);

            vector<VariableBlock*> newvaluesforthing;
            newvaluesforthing.reserve(vallist[a]->GetVariableCount());

            // Try to replace values that are strings //
            for (size_t b = 0; b < vallist[a]->GetVariableCount(); b++) {

                VariableBlock* processblock = vallist[a]->GetValueDirect(b);

                auto typval = processblock->GetBlock()->Type;

                // Non-string types don't need replacing //
                if (typval != DATABLOCK_TYPE_STRING && typval != DATABLOCK_TYPE_WSTRING) {

                    newvaluesforthing.push_back(new
                        VariableBlock(processblock->GetBlock()->AllocateNewFromThis()));

                } else {

                    // Try replacing stuff //
                    std::string valstring;

                    if (!processblock->ConvertAndAssingToVariable<std::string>(valstring)) {

                        DEBUG_BREAK;
                    }

                    // Try to replace //
                    ReplaceStringWithTemplateArguments(valstring, instanceargs.Arguments);

                    // Parse the result into a variable block //
                    try {

                        unique_ptr<VariableBlock> tmpblock(new VariableBlock(valstring, NULL));

                        newvaluesforthing.push_back(tmpblock.release());

                    }
                    catch (const InvalidArgument &e) {

                        if (reporterror) {

                            reporterror->Warning("ObjectFileTemplates: a templated list has an "
                                "invalid value to parse, using empty string instead:");
                            e.Print(reporterror);
                        }

                        newvaluesforthing.push_back(new VariableBlock(new StringBlock(
                            new std::string())));
                        continue;
                    }
                }
            }

            // Add the variable list //
            listobj->AddVariable(shared_ptr<NamedVariableList>(new
                NamedVariableList(newvarlistname, newvaluesforthing)));
        }

        // Add the list to the new object //
        internalobj->AddVariableList(listobj);

    }

    // Process the text blocks //
    for (size_t i = 0; i < RepresentingObject->GetTextBlockCount(); i++) {

        // Copy the data from the list replacing all the templates with values //
        ObjectFileTextBlock* curblock = RepresentingObject->GetTextBlock(i);

        std::string actblockname = curblock->GetName();

        ReplaceStringWithTemplateArguments(actblockname, instanceargs.Arguments);

        unique_ptr<ObjectFileTextBlock> textblock(new ObjectFileTextBlockProper(actblockname));

        // Replace all the lines //


        for (size_t a = 0; a < curblock->GetLineCount(); a++) {

            std::string curline = curblock->GetLine(a);

            ReplaceStringWithTemplateArguments(curline, instanceargs.Arguments);

            textblock->AddTextLine(curline);
        }

        // Add to the object //
        internalobj->AddTextBlock(textblock);
    }

    // Process the script source //


    // Only the first segment should be used //
#ifdef LEVIATHAN_USING_ANGELSCRIPT
    auto scrptwrap = RepresentingObject->GetScript();
    if (scrptwrap) {

        auto scrptmodule = scrptwrap->GetModuleSafe();

        if (scrptmodule && scrptmodule->GetScriptSegmentCount() > 0) {

            // We can try to specialize the code //
            auto onlysegment = scrptmodule->GetScriptSegment(0);

            const string& resultingsource = *onlysegment->SourceCode;

            const string& newsource = ReplaceStringTemplateArguments(resultingsource,
                instanceargs.Arguments);

            std::string newname = scrptmodule->GetName();

            ReplaceStringWithTemplateArguments(newname, instanceargs.Arguments);

            shared_ptr<ScriptScript> newscript(new
                ScriptScript(ScriptExecutor::Get()->CreateNewModule(newname,
                    scrptmodule->GetSource())));

            // Add the new script to the object //
            shared_ptr<ScriptSourceFileData> newsourcesegment(new
                ScriptSourceFileData(onlysegment->SourceFile, onlysegment->StartLine, newsource));

            auto newmod = newscript->GetModuleSafe();

            newmod->AddScriptSegment(newsourcesegment);
            newmod->SetBuildState(SCRIPTBUILDSTATE_READYTOBUILD);

            internalobj->AddScriptScript(newscript);
        }
    }
#endif // LEVIATHAN_USING_ANGELSCRIPT

    // Return the object //
    return internalobj;
}
// ------------------------------------ //
void Leviathan::ObjectFileTemplateDefinition::ReplaceStringWithTemplateArguments(
    std::string &target, const std::vector<std::unique_ptr<std::string>> &args) {
    
    LEVIATHAN_ASSERT(Parameters.size() == args.size(), "ReplaceStringWithTemplateArguments arguments size mismatch");

    // Look for strings matching our parameters //
    for (size_t i = 0; i < Parameters.size(); i++) {

        target = StringOperations::Replace<std::string>(target, *Parameters[i], *args[i]);
    }
}

std::string Leviathan::ObjectFileTemplateDefinition::ReplaceStringTemplateArguments(
    const string &target, const std::vector<unique_ptr<string>> &args) {

    LEVIATHAN_ASSERT(Parameters.size() == args.size(), "ReplaceStringTemplateArguments arguments size mismatch");

    string result;

    if (Parameters.size() == 0)
        return target;

    result = StringOperations::Replace<string>(target, *Parameters[0], *args[0]);

    // Look for strings matching our parameters //
    for (size_t i = 1; i < Parameters.size(); i++) {

        result = StringOperations::Replace<string>(result, *Parameters[i], *args[i]);
    }

    return result;
}
// ------------------ ObjectFileTemplateInstance ------------------ //
DLLEXPORT Leviathan::ObjectFileTemplateInstance::ObjectFileTemplateInstance(
    const string &mastertmplname, std::vector<unique_ptr<string>> &templateargs) :
    TemplatesName(mastertmplname), Arguments(move(templateargs)) {

}
// ------------------ ObjectFileTemplateObject ------------------ //
DLLEXPORT Leviathan::ObjectFileTemplateObject::ObjectFileTemplateObject(const std::string &name,
    const std::string &typesname, vector<std::unique_ptr<std::string>>&& prefix) :
    ObjectFileObjectProper(name, typesname, std::move(prefix)) {

}
