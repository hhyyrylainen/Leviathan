#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_OBJECTFILETEMPLATES
#include "ObjectFileTemplates.h"
#endif
#include "ObjectFileObject.h"
#include "Common\StringOperations.h"
#include "Script\ScriptInterface.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::ObjectFileTemplateDefinition::ObjectFileTemplateDefinition(const string &name, std::vector<unique_ptr<string>> &parameters, 
	shared_ptr<ObjectFileObject> obj) : Name(name), Parameters(move(parameters)), RepresentingObject(obj)
{
	// Convert the Parameters //
	WParameters.reserve(Parameters.size());

	for(size_t i = 0; i < Parameters.size(); i++){

		// Convert and move //
		WParameters.push_back(move(unique_ptr<wstring>(new wstring(Convert::Utf8ToUtf16(*Parameters[i])))));
	}


}
// ------------------------------------ //
DLLEXPORT const string& Leviathan::ObjectFileTemplateDefinition::GetName() const{
	return Name;
}
// ------------------------------------ //
DLLEXPORT  shared_ptr<ObjectFileTemplateDefinition> Leviathan::ObjectFileTemplateDefinition::CreateFromObject(const string &name, 
	shared_ptr<ObjectFileObject> obj, std::vector<unique_ptr<string>> &templateargs)
{
	// This could be changed to a function that tears down the object and creates mess of templating objects //
	shared_ptr<ObjectFileTemplateDefinition> resultobj(new ObjectFileTemplateDefinition(name, templateargs, obj));

	return resultobj;
}

DLLEXPORT unique_ptr<ObjectFileTemplateObject> Leviathan::ObjectFileTemplateDefinition::CreateInstanceFromThis(const ObjectFileTemplateInstance 
	&instanceargs)
{
	// First make sure that template counts match, return NULL otherwise //
	if(Parameters.size() != instanceargs.Arguments.size() || WParameters.size() != instanceargs.WArguments.size()){

		return NULL;
	}


	// Make sure these are not templated //
	wstring newname = RepresentingObject->GetName();
	wstring newtype = RepresentingObject->GetTypeName();

	std::vector<wstring*> newprefixes;

	ReplaceWstringWithTemplateArguments(newname, instanceargs.WArguments);
	ReplaceWstringWithTemplateArguments(newtype, instanceargs.WArguments);

	for(size_t i = 0; i < RepresentingObject->GetPrefixesCount(); i++){

		unique_ptr<wstring> newprefix(new wstring(RepresentingObject->GetPrefix(i)));

		ReplaceWstringWithTemplateArguments(*newprefix, instanceargs.WArguments);

		newprefixes.push_back(newprefix.release());
	}


	// We somehow need to detect all places that might have the template parameters and change them //
	unique_ptr<ObjectFileTemplateObject> internalobj(new ObjectFileTemplateObject(newname, newtype, newprefixes));

	// Do the contents the same way //
	for(size_t i = 0; i < RepresentingObject->GetListCount(); i++){

		// Copy the data from the list replacing all the templates with values //
		ObjectFileList* curlist = RepresentingObject->GetList(i);

		wstring actlistname = curlist->GetName();

		ReplaceWstringWithTemplateArguments(actlistname, instanceargs.WArguments);

		unique_ptr<ObjectFileList> listobj(new ObjectFileListProper(actlistname));

		const std::vector<shared_ptr<NamedVariableList>>& vallist = *curlist->GetVariables().GetVec();



		// Add the values replacing template arguments //
		for(size_t a = 0; a < vallist.size(); a++){

			// First check the name //
			wstring newvarlistname = vallist[a]->GetName();

			ReplaceWstringWithTemplateArguments(newvarlistname, instanceargs.WArguments);

			vector<VariableBlock*> newvaluesforthing;
			newvaluesforthing.reserve(vallist[a]->GetVariableCount());

			// Try to replace values that are strings //
			for(size_t b = 0; b < vallist[a]->GetVariableCount(); b++){


				VariableBlock* processblock = vallist[a]->GetValueDirect(b);

				auto typval = processblock->GetBlock()->Type;

				// Non-string types don't need replacing //
				if(typval != DATABLOCK_TYPE_STRING && typval != DATABLOCK_TYPE_WSTRING){

					newvaluesforthing.push_back(new VariableBlock(processblock->GetBlock()->AllocateNewFromThis()));

				} else {

					// Try replacing stuff //
					wstring valstring;

					if(!processblock->ConvertAndAssingToVariable<wstring>(valstring)){

						DEBUG_BREAK;
					}

					// Try to replace //
					ReplaceWstringWithTemplateArguments(valstring, instanceargs.WArguments);

					// Parse the result into a variable block //
					try{

						unique_ptr<VariableBlock> tmpblock(new VariableBlock(valstring, NULL));

						newvaluesforthing.push_back(tmpblock.release());

					} catch(const ExceptionInvalidArgument &e){

#ifdef _DEBUG
						Logger::Get()->Warning(L"ObjectFileTemplates: a templated list has an invalid value to parse, using empty string instead:");
						e.PrintToLog();
#endif // _DEBUG

						newvaluesforthing.push_back(new VariableBlock(new WstringBlock(new wstring())));
						continue;
					}
				}
			}

			// Add the variable list //
			listobj->AddVariable(shared_ptr<NamedVariableList>(new NamedVariableList(newvarlistname, newvaluesforthing)));
		}

		// Add the list to the new object //
		internalobj->AddVariableList(listobj);

	}

	// Process the text blocks //
	for(size_t i = 0; i < RepresentingObject->GetTextBlockCount(); i++){

		// Copy the data from the list replacing all the templates with values //
		ObjectFileTextBlock* curblock = RepresentingObject->GetTextBlock(i);

		wstring actblockname = curblock->GetName();

		ReplaceWstringWithTemplateArguments(actblockname, instanceargs.WArguments);

		unique_ptr<ObjectFileTextBlock> textblock(new ObjectFileTextBlockProper(actblockname));

		// Replace all the lines //


		for(size_t a = 0; a < curblock->GetLineCount(); a++){

			wstring curline = curblock->GetLine(a);


			ReplaceWstringWithTemplateArguments(curline, instanceargs.WArguments);

			textblock->AddTextLine(curline);
		}

		// Add to the object //
		internalobj->AddTextBlock(textblock);
	}

	// Process the script source //


	// Only the first segment should be used //
	auto scrptwrap = internalobj->GetScript();
	if(scrptwrap){

		auto scrptmodule = scrptwrap->GetModuleSafe();

		if(scrptmodule && scrptmodule->GetScriptSegmentCount() > 0){

			// We can try to specialize the code //
			auto onlysegment = scrptmodule->GetScriptSegment(0);

			const string& resultingsource = *onlysegment->SourceCode;

			const string& newsource = ReplaceStringTemplateArguments(resultingsource, instanceargs.Arguments);


			shared_ptr<ScriptScript> newscript(new ScriptScript(ScriptInterface::Get()->GetExecutor()->CreateNewModule(scrptmodule->GetName(), 
				scrptmodule->GetSource())));

			// Add the new script to the object //
			shared_ptr<ScriptSourceFileData> newsourcesegment(new ScriptSourceFileData(onlysegment->SourceFile, onlysegment->StartLine, 
				newsource));

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
void Leviathan::ObjectFileTemplateDefinition::ReplaceWstringWithTemplateArguments(wstring &target, const std::vector<unique_ptr<wstring>> &args){

	assert(WParameters.size() == args.size() && "Wrong number of args passed to ReplaceWstringWithTemplateArguments, doesn't match with master template");

	// Look for strings matching our parameters //
	for(size_t i = 0; i < WParameters.size(); i++){

		target = StringOperations::Replace<wstring>(target, *WParameters[i], *args[i]);
	}
}

std::string Leviathan::ObjectFileTemplateDefinition::ReplaceStringTemplateArguments(const string &target, const std::vector<unique_ptr<string>> &args){

	assert(Parameters.size() == args.size() && "Wrong number of args passed to ReplaceStringTemplateArguments, doesn't match with master template");

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
DLLEXPORT Leviathan::ObjectFileTemplateInstance::ObjectFileTemplateInstance(const string &mastertmplname, std::vector<unique_ptr<string>> 
	&templateargs) : TemplatesName(mastertmplname), Arguments(move(templateargs))
{
	// Convert the Arguments //
	WArguments.reserve(Arguments.size());

	for(size_t i = 0; i < Arguments.size(); i++){

		// Convert and move //
		WArguments.push_back(move(unique_ptr<wstring>(new wstring(Convert::Utf8ToUtf16(*Arguments[i])))));
	}
}
// ------------------ ObjectFileTemplateObject ------------------ //
DLLEXPORT Leviathan::ObjectFileTemplateObject::ObjectFileTemplateObject(const wstring &name, const wstring &typesname, vector<wstring*> prefix) : 
	ObjectFileObjectProper(name, typesname, prefix)
{

}
