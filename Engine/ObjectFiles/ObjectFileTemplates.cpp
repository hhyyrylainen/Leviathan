// ------------------------------------ //
#include "ObjectFileTemplates.h"

#include "ObjectFileObject.h"
#include "Common/StringOperations.h"
#include "../Script/ScriptExecutor.h"
#include "../Script/ScriptModule.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::ObjectFileTemplateDefinition::ObjectFileTemplateDefinition(const string &name,
    std::vector<unique_ptr<string>> &parameters, std::shared_ptr<ObjectFileObject> obj) :
    Name(name), Parameters(move(parameters)), RepresentingObject(obj)
{

}
// ------------------------------------ //
DLLEXPORT const string& Leviathan::ObjectFileTemplateDefinition::GetName() const{
	return Name;
}
// ------------------------------------ //
DLLEXPORT  std::shared_ptr<ObjectFileTemplateDefinition>
Leviathan::ObjectFileTemplateDefinition::CreateFromObject(const string &name,
    std::shared_ptr<ObjectFileObject> obj, std::vector<unique_ptr<string>> &templateargs)
{
	// This could be changed to a function that tears down the object and creates mess
    // of templating objects
	auto resultobj = make_shared<ObjectFileTemplateDefinition>(name, templateargs, obj);

	return resultobj;
}

DLLEXPORT std::unique_ptr<ObjectFileTemplateObject> Leviathan::ObjectFileTemplateDefinition::CreateInstanceFromThis(
    const ObjectFileTemplateInstance &instanceargs)
{
	// First make sure that template counts match, return NULL otherwise //
	if(Parameters.size() != instanceargs.Arguments.size()){

		return NULL;
	}


	// Make sure these are not templated //
	std::string newname = RepresentingObject->GetName();
	std::string newtype = RepresentingObject->GetTypeName();

	std::vector<std::string*> newprefixes;

	ReplaceStringWithTemplateArguments(newname, instanceargs.Arguments);
	ReplaceStringWithTemplateArguments(newtype, instanceargs.Arguments);

	for(size_t i = 0; i < RepresentingObject->GetPrefixesCount(); i++){

		unique_ptr<std::string> newprefix(new std::string(RepresentingObject->GetPrefix(i)));

		ReplaceStringWithTemplateArguments(*newprefix, instanceargs.Arguments);

		newprefixes.push_back(newprefix.release());
	}


	// We somehow need to detect all places that might have the template parameters and
    // change them
	unique_ptr<ObjectFileTemplateObject> internalobj(new ObjectFileTemplateObject(newname,
            newtype, newprefixes));

	// Do the contents the same way //
	for(size_t i = 0; i < RepresentingObject->GetListCount(); i++){

		// Copy the data from the list replacing all the templates with values //
		ObjectFileList* curlist = RepresentingObject->GetList(i);

		std::string actlistname = curlist->GetName();

		ReplaceStringWithTemplateArguments(actlistname, instanceargs.Arguments);

		unique_ptr<ObjectFileList> listobj(new ObjectFileListProper(actlistname));

		const std::vector<shared_ptr<NamedVariableList>>& vallist =
            *curlist->GetVariables().GetVec();

		// Add the values replacing template arguments //
		for(size_t a = 0; a < vallist.size(); a++){

			// First check the name //
			std::string newvarlistname = vallist[a]->GetName();

			ReplaceStringWithTemplateArguments(newvarlistname, instanceargs.Arguments);

			vector<VariableBlock*> newvaluesforthing;
			newvaluesforthing.reserve(vallist[a]->GetVariableCount());

			// Try to replace values that are strings //
			for(size_t b = 0; b < vallist[a]->GetVariableCount(); b++){

				VariableBlock* processblock = vallist[a]->GetValueDirect(b);

				auto typval = processblock->GetBlock()->Type;

				// Non-string types don't need replacing //
				if(typval != DATABLOCK_TYPE_STRING && typval != DATABLOCK_TYPE_WSTRING){

					newvaluesforthing.push_back(new
                        VariableBlock(processblock->GetBlock()->AllocateNewFromThis()));

				} else {

					// Try replacing stuff //
					std::string valstring;

					if(!processblock->ConvertAndAssingToVariable<std::string>(valstring)){

						DEBUG_BREAK;
					}

					// Try to replace //
					ReplaceStringWithTemplateArguments(valstring, instanceargs.Arguments);

					// Parse the result into a variable block //
					try{

						unique_ptr<VariableBlock> tmpblock(new VariableBlock(valstring, NULL));

						newvaluesforthing.push_back(tmpblock.release());

					} catch(const InvalidArgument &e){

						Logger::Get()->Warning("ObjectFileTemplates: a templated list has an "
                            "invalid value to parse, using empty string instead:");
						e.PrintToLog();

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
	for(size_t i = 0; i < RepresentingObject->GetTextBlockCount(); i++){

		// Copy the data from the list replacing all the templates with values //
		ObjectFileTextBlock* curblock = RepresentingObject->GetTextBlock(i);

		std::string actblockname = curblock->GetName();

		ReplaceStringWithTemplateArguments(actblockname, instanceargs.Arguments);

		unique_ptr<ObjectFileTextBlock> textblock(new ObjectFileTextBlockProper(actblockname));

		// Replace all the lines //


		for(size_t a = 0; a < curblock->GetLineCount(); a++){

			std::string curline = curblock->GetLine(a);

			ReplaceStringWithTemplateArguments(curline, instanceargs.Arguments);

			textblock->AddTextLine(curline);
		}

		// Add to the object //
		internalobj->AddTextBlock(textblock);
	}

	// Process the script source //


	// Only the first segment should be used //
	auto scrptwrap = RepresentingObject->GetScript();
	if(scrptwrap){

		auto scrptmodule = scrptwrap->GetModuleSafe();

		if(scrptmodule && scrptmodule->GetScriptSegmentCount() > 0){

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



	// Return the object //
	return internalobj;
}
// ------------------------------------ //
void Leviathan::ObjectFileTemplateDefinition::ReplaceStringWithTemplateArguments(
    std::string &target, const std::vector<unique_ptr<std::string>> &args)
{
    if(Parameters.size() != args.size()){
        Logger::Get()->Error("Wrong number of args passed to ReplaceStringWithTemplateArguments, "
        "doesn't match with master template");
    }

	// Look for strings matching our parameters //
	for(size_t i = 0; i < Parameters.size(); i++){

		target = StringOperations::Replace<std::string>(target, *Parameters[i], *args[i]);
	}
}

std::string Leviathan::ObjectFileTemplateDefinition::ReplaceStringTemplateArguments(
    const string &target, const std::vector<unique_ptr<string>> &args)
{
	if(Parameters.size() != args.size()){
        Logger::Get()->Error("Wrong number of args passed to ReplaceStringTemplateArguments, "
        "doesn't match with master template");
    }

	string result;

	if(Parameters.size() == 0)
		return target;

	result = StringOperations::Replace<string>(target, *Parameters[0], *args[0]);

	// Look for strings matching our parameters //
	for(size_t i = 1; i < Parameters.size(); i++){

		result = StringOperations::Replace<string>(result, *Parameters[i], *args[i]);
	}

	return result;
}
// ------------------ ObjectFileTemplateInstance ------------------ //
DLLEXPORT Leviathan::ObjectFileTemplateInstance::ObjectFileTemplateInstance(
    const string &mastertmplname, std::vector<unique_ptr<string>> &templateargs) :
    TemplatesName(mastertmplname), Arguments(move(templateargs))
{

}
// ------------------ ObjectFileTemplateObject ------------------ //
DLLEXPORT Leviathan::ObjectFileTemplateObject::ObjectFileTemplateObject(const std::string &name,
    const std::string &typesname, vector<std::string*> prefix) : 
	ObjectFileObjectProper(name, typesname, prefix)
{

}
