#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_OBJECTLOADER
#include "ObjectLoader.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "ModelObject.h"
#include "TimingMonitor.h"

Leviathan::ObjectLoader::ObjectLoader(Engine* engine){
	m_Engine = engine;
}
Leviathan::ObjectLoader::~ObjectLoader(){
	m_Engine = NULL;

	// release memory //

}
// ------------------------------------ //
DLLEXPORT vector<GameObject::Model*> Leviathan::ObjectLoader::LoadModelFile(const wstring &file, bool finishnow /*= true*/){
	QUICKTIME_THISSCOPE;

	vector<GameObject::Model*> result = vector<GameObject::Model*>();
	// try to locate data file //
	wstring modelextension = L"levmd";
	wstring path = FileSystem::SearchForFile(FILEGROUP_MODEL, file, modelextension, false);

	if(path == L""){
		// the file wasn't found //
		Logger::Get()->Error(L"ObjectLoader: LoadModelFile: failed to find file: "+file);
		return result;
	}

	// use ObjectFileParser class to get structures from the file //
	vector<shared_ptr<NamedVar>> HeaderVars;
	vector<shared_ptr<ObjectFileObject>> structure =  ObjectFileProcessor::ProcessObjectFile(path, HeaderVars);

	if(structure.size() == 0){

		Logger::Get()->Error(L"ObjectLoader: LoadModelFile: file is empty/invalid structure: "+path);
		return result;
	}
	int headvar_int = 0;
	wstring headvar_str = L"";

	// parent name //
	wstring parentname = L"";

	bool IsUnCompiled = false;
	//bool CompileSucceeded = false;

	// interpret header vars //
	for(unsigned int i = 0; i < HeaderVars.size(); i++){
		if(HeaderVars[i]->GetName() == L"FileType"){
			// just in case: check is it correct //
			HeaderVars[i]->GetValue(headvar_int, headvar_str);
			if(headvar_str != L"Model"){
				// invalid file //
				Logger::Get()->Error(L"ObjectLoader: LoadModelFile: file header/type invalid expected \"Model\" received "+headvar_str, true);
				return result;
			}
		}
		if(HeaderVars[i]->GetName() == L"Model-Name"){
			// store the model file's name //
			HeaderVars[i]->GetValue(headvar_int, headvar_str);
			parentname = headvar_str;
		}
		if(HeaderVars[i]->GetName() == L"Model-Format"){
			// check type //
			HeaderVars[i]->GetValue(headvar_int, headvar_str);
			
		}
		if(HeaderVars[i]->GetName() == L"Model-UnCompiled"){
			// check type //
			HeaderVars[i]->GetValue(headvar_int, headvar_str);
			IsUnCompiled = headvar_int == 1;
			// erase non wanted headervars //
			HeaderVars.erase(HeaderVars.begin()+i);
			i--;
		}
	}


	// vector that is used in many places to send flags to various model functions //
	vector<shared_ptr<Flag>> Flags;


	// go through the structures and add them to the result vector //
	for(unsigned int i = 0; i < structure.size(); i++){
		if(structure[i]->TName == L"Model"){
			GameObject::Model* obj = new GameObject::Model();

			// set name //
			wstring objname = structure[i]->Name;

			// go through property lists and set data //
			for(unsigned int a = 0; a < structure[i]->Contents.size(); a++){
				// switch on list's name and process it's data //
				if(structure[i]->Contents[a]->Name == L"SurfaceProperties"){
					// process  here //

					continue;
				}
				if(structure[i]->Contents[a]->Name == L"Flags"){
					// process  here //

					continue;
				}
			}
			for(unsigned int a = 0; a < structure[i]->TextBlocks.size(); a++){
				// these don't have any variables must be in text blocks //
				if(structure[i]->TextBlocks[a]->Name == L"ModelFiles"){
					// located model files to load //

					// only single model file is supported ATM //
					//vector<wstring> FilePaths;

					// locate model //
					wstring filetosearchfor = L"";
					wstring extension = L"";
					if(structure[i]->TextBlocks[a]->Lines.size() > 0){
						filetosearchfor = FileSystem::RemoveExtension(*structure[i]->TextBlocks[a]->Lines[0], true);
						extension = FileSystem::GetExtension(*structure[i]->TextBlocks[a]->Lines[0]);

					} else {
						Logger::Get()->Error(L"ObjectLoader: LoadModelFile: model file contained no model data files to load", true);
						continue;
					}

					wstring path = FileSystem::SearchForFile(FILEGROUP_MODEL, filetosearchfor, extension, false);
					if(path == L""){
						Logger::Get()->Error(L"ObjectLoader: LoadModelFile: no model file found \""+filetosearchfor+L"\" with extension "+extension);
						continue;
					}
					// add normal model flag //
					Flags.push_back(shared_ptr<Flag>(new Flag(FLAG_GOBJECT_MODEL_TYPE_NORMAL)));
					// set them to the object //
					obj->SetModelToLoad(path, MultiFlag(Flags));
					// flags are released by Multiflag destructor OLD//
					// clear smart pointers //
					Flags.clear();

					continue;
				}
				if(structure[i]->TextBlocks[a]->Name == L"TextureFiles"){
					// process textures here //
					vector<int> TTypes;
					vector<shared_ptr<wstring>> TexturePaths;

					bool TypeLine = true;
					//int currentlinetype = 0;
					for(unsigned int index = 0; index < structure[i]->TextBlocks[a]->Lines.size(); index++){
						if(TypeLine){
							// check which type //
							int temptype = GameObject::Model::GetFlagFromTextureTypeName(*structure[i]->TextBlocks[a]->Lines[index]);



							// push to vector //
							TTypes.push_back(temptype);
							TypeLine = false;
							continue;
						}
						// check some stuff //
						// locate file and push to textures vector //
						wstring filetosearch = FileSystem::RemoveExtension(*structure[i]->TextBlocks[a]->Lines[index], true);
						wstring extension = FileSystem::GetExtension(*structure[i]->TextBlocks[a]->Lines[index]);
						// check for uncompiled model //
						if(IsUnCompiled){
							for(unsigned int ttypeindex = 0; ttypeindex < TTypes.size(); ttypeindex++){
								// check is it correct //
								if(TTypes[ttypeindex] == FLAG_GOBJECT_MODEL_TEXTURETYPE_UNKOWN){
									// texture file needs to be loaded //
									TextureDefinition* temp = LoadTextureDefinitionFile(filetosearch);
									if(temp == NULL){
										// error //
										Logger::Get()->Error(L"ObjectLoader: LoadModelFile: uncompiled model has a texture that doesn't have definition file, levtd");
										// erase //
										TTypes.erase(TTypes.begin()+ttypeindex);
										continue;
									}
									// get real type //
									int realtype = temp->Type;
									TTypes[ttypeindex] = realtype;
									wstring typesname = GameObject::Model::TextureFlagToTypeName(realtype);
									extension = temp->ImageExtension;
									// fix some lines for saving back to file //

									// insert extension //

									(*structure[i]->TextBlocks[a]->Lines[index]) += L"."+extension;

									// change previous line //
									*structure[i]->TextBlocks[a]->Lines[index-1] = Misc::Replace(*structure[i]->TextBlocks[a]->Lines[index-1], L"CheckRequired", typesname);

									SAFE_DELETE(temp);
								}
							}
						}



						// do the actual search here //
						wstring path = FileSystem::SearchForFile(FILEGROUP_TEXTURE, filetosearch, extension);
						if(path == L""){
							Logger::Get()->Error(L"ObjectLoader: no texture file found \""+filetosearch+L"\" with extension "+extension);
							path = L"404";
						}

						TexturePaths.push_back(shared_ptr<wstring>(new wstring(path)));
					}
					// generate flags from ttypes and push to model object //

					// push them to flags vector //
					for(unsigned int index = 0; index < TTypes.size(); index++){
						Flags.push_back(shared_ptr<Flag>(new Flag(TTypes[index])));
					}

					obj->SetTexturesToLoad(TexturePaths, MultiFlag(Flags));
					// flags should be cleared by Multiflag's destructor //
					// clear smart pointers //
					Flags.clear();
					continue;
				}

			}
			
			// set model flag //
			if(IsUnCompiled){
				wstring realtype = obj->GetModelTypeName();
				// set //
				for(unsigned int a = 0; a < structure[i]->Contents.size(); a++){
					if(structure[i]->Contents[a]->Name == L"Flags"){
						// set  ModelType line //
						structure[i]->Contents[a]->Variables->SetValue(L"ModelType", realtype);
						continue;
					}
				}

			}


			// finally add to result vector //
			result.push_back(obj);
			continue;
		}

		Logger::Get()->Error(L"ObjectLoader: unrecognized object in "+path+L" called "+structure[i]->TName, true);
	}

	if(IsUnCompiled){
		// remove coordinate system headvar //
		for(unsigned int i = 0; i < HeaderVars.size(); i++){
			if(HeaderVars[i]->GetName() == L"CoordinateSystem"){
				HeaderVars.erase(HeaderVars.begin()+i);
				break;
			}
		}



		// resave the file structure //
		if(ObjectFileProcessor::WriteObjectFile(structure, path, HeaderVars) != 1){

			Logger::Get()->Error(L"ObjectLoader: LoadModelFile: failed to save the file on top of the old file");
		}
	}

	//// cleanup header variables //
	//SAFE_DELETE_VECTOR(HeaderVars);

	return result;
}

DLLEXPORT TextureDefinition* Leviathan::ObjectLoader::LoadTextureDefinitionFile(const wstring &file){
	TextureDefinition* result = NULL;
	// try to locate data file //
	wstring modelextension = L"levtd";
	wstring path = FileSystem::SearchForFile(FILEGROUP_TEXTURE, file, modelextension, false);

	if(path == L""){
		// the file wasn't found //
		Logger::Get()->Error(L"ObjectLoader: LoadTextureDefinitionFile: failed to find file: "+file);
		return result;
	}

	// use ObjectFileParser class to get structures from the file //
	vector<shared_ptr<NamedVar>> HeaderVars;
	vector<shared_ptr<ObjectFileObject>> structure =  ObjectFileProcessor::ProcessObjectFile(path, HeaderVars);

	int headvar_int = 0;
	wstring headvar_str = L"";

	// parent name //
	// create //
	result = new TextureDefinition();

	wstring DefaultAnimationName;

	// interpret header vars //
	for(unsigned int i = 0; i < HeaderVars.size(); i++){
		if(HeaderVars[i]->GetName() == L"Author"){
			// set author //
			HeaderVars[i]->GetValue(headvar_int, headvar_str);
			result->Creator = headvar_str;
		}
		if(HeaderVars[i]->GetName() == L"Texture-Name"){
			// store the texture's name //
			HeaderVars[i]->GetValue(headvar_int, headvar_str);
			result->Name = headvar_str;
		}
		if(HeaderVars[i]->GetName() == L"Texture-Type"){
			// check type //
			HeaderVars[i]->GetValue(headvar_int, headvar_str);
			result->Type = GameObject::Model::GetFlagFromTextureTypeName(headvar_str);
		}
		if(HeaderVars[i]->GetName() == L"Width"){
			// store width //
			HeaderVars[i]->GetValue(headvar_int, headvar_str);
			result->Width = headvar_int;
		}
		if(HeaderVars[i]->GetName() == L"Height"){
			// store height //
			HeaderVars[i]->GetValue(headvar_int, headvar_str);
			result->Height = headvar_int;
		}
		if(HeaderVars[i]->GetName() == L"ActualExtension"){
			// store image's extension //
			HeaderVars[i]->GetValue(headvar_int, headvar_str);
			result->ImageExtension = headvar_str;
		}
		if(HeaderVars[i]->GetName() == L"Animation"){
			// check does have animation //
			if(HeaderVars[i]->IsIntValue()){
				// no animation //
				result->HasAnimation = false;
				continue;
			}
			// there is a animation //
			HeaderVars[i]->GetValue(headvar_int, headvar_str);
			result->HasAnimation = true;
			DefaultAnimationName = headvar_str;
		}
	}

	if(result->HasAnimation){
		// go through the structures and handle animations in them //
		for(unsigned int i = 0; i < structure.size(); i++){
			if(structure[i]->TName == L"AnimatedTexture"){
				// process this in some way//
				Logger::Get()->Info(L"ObjectLoader: LoadTextureDefinitionFile: loading Animations is not supported yet!", true);
				continue;
			}
		}
	}
	// SMART pointers
	//SAFE_DELETE_VECTOR(HeaderVars);

	// done //
	return result;
}

// ------------------------------------ //
//vector<GameObject::Model*> Leviathan::ObjectLoader::LoadModelAssIScene(const aiScene* scene){
//	vector<GameObject::Model*> result = vector<GameObject::Model*>();
//
//	// check does scene contain any meshes //
//	if(scene->mNumMeshes < 1){
//		// the file wasn't found //
//		Logger::Get()->Error(L"ObjectLoader:: LoadModelAssIScene: no meshes in scene!");
//		return result;
//	}
//
//	// push back 1 model and use it's hacky function to process the scene //
//	result.push_back(new GameObject::Model());
//
//	result[0]->LoadFromaiScene(scene);
//
//
//	return result;
//}
// ------------------------------------ //

// ------------------------------------ //



