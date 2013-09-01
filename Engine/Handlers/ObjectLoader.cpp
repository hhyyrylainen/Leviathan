#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_OBJECTLOADER
#include "ObjectLoader.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "Entities\ModelObject.h"
#include "Statistics\TimingMonitor.h"
#include "OgreManualObject.h"

Leviathan::ObjectLoader::ObjectLoader(Engine* engine){
	m_Engine = engine;
}
// ------------------------------------ //
DLLEXPORT vector<GameObject::Model*> Leviathan::ObjectLoader::LoadModelFile(const wstring &file, bool finishnow /*= true*/){
	QUICKTIME_THISSCOPE;

	vector<GameObject::Model*> result = vector<GameObject::Model*>();
	// try to locate data file //
	wstring modelextension = L"levmd";
	wstring path = FileSystem::Get()->SearchForFile(FILEGROUP_MODEL, file, modelextension, false);

	if(path == L""){
		// the file wasn't found //
		Logger::Get()->Error(L"ObjectLoader: LoadModelFile: failed to find file: "+file);
		return result;
	}

	// use ObjectFileParser class to get structures from the file //
	vector<shared_ptr<NamedVariableList>> HeaderVars;
	vector<shared_ptr<ObjectFileObject>> structure =  ObjectFileProcessor::ProcessObjectFile(path, HeaderVars);

	if(structure.size() == 0){

		Logger::Get()->Error(L"ObjectLoader: LoadModelFile: file is empty/invalid structure: "+path);
		return result;
	}
	wstring headvar_str = L"";

	// parent name //
	wstring parentname = L"";

	bool IsUnCompiled = false;
	//bool CompileSucceeded = false;

	// interpret header vars //
	for(unsigned int i = 0; i < HeaderVars.size(); i++){
		if(HeaderVars[i]->GetName() == L"FileType"){
			// just in case: check is it correct //
			VariableBlock* curblock = HeaderVars[i]->GetValueDirect();

			if(!curblock->ConvertAndAssingToVariable<wstring>(headvar_str)){

				// dang //
				DEBUG_BREAK;
			}

			if(headvar_str != L"Model"){
				// invalid file //
				Logger::Get()->Error(L"ObjectLoader: LoadModelFile: file header/type invalid expected \"Model\" received "+headvar_str, true);
				return result;
			}
		}
		if(HeaderVars[i]->GetName() == L"Model-Name"){
			// store the model file's name //
			if(!HeaderVars[i]->GetValue().ConvertAndAssingToVariable<wstring>(parentname)){

				// dang //
				DEBUG_BREAK;
				Logger::Get()->Error(L"LoadModelFile: invalid header value (cannot be cast to right type, check definition) named: "+
					HeaderVars[i]->GetName());
			}
		}
		if(HeaderVars[i]->GetName() == L"Model-Format"){
			// check type //
			if(!HeaderVars[i]->GetValue().ConvertAndAssingToVariable<wstring>(headvar_str)){

				// dang //
				DEBUG_BREAK;
				Logger::Get()->Error(L"LoadModelFile: invalid header value (cannot be cast to right type, check definition) named: "+
					HeaderVars[i]->GetName());
			}
		}
		if(HeaderVars[i]->GetName() == L"Model-UnCompiled"){
			// get compilation state flag //
			if(!HeaderVars[i]->GetValue().ConvertAndAssingToVariable<bool>(IsUnCompiled)){

				// dang //
				DEBUG_BREAK;
				Logger::Get()->Error(L"LoadModelFile: invalid header value (cannot be cast to right type, check definition) named: "+
					HeaderVars[i]->GetName());
			}
		}
	}


	// vector that is used in many places to send flags to various model functions //
	vector<shared_ptr<Flag>> Flags;


	// go through the structures and add them to the result vector //
	for(unsigned int i = 0; i < structure.size(); i++){
		if(structure[i]->TName == L"Model"){
			unique_ptr<GameObject::Model> obj(new GameObject::Model());

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
					// TODO: support for rendering and collision models
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

					wstring path = FileSystem::Get()->SearchForFile(FILEGROUP_MODEL, filetosearchfor, extension, false);
					if(path == L""){
						Logger::Get()->Error(L"ObjectLoader: LoadModelFile: no model file found \""+filetosearchfor+L"\" with extension "+extension);
						continue;
					}
					// set them to the object //
					obj->SetModelToLoad(path);

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
						wstring path = FileSystem::Get()->SearchForFile(FILEGROUP_TEXTURE, filetosearch, extension);
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


			}


			// finally add to result vector //
			result.push_back(obj.release());
			continue;
		}

		Logger::Get()->Error(L"ObjectLoader: unrecognized object in "+path+L" called "+structure[i]->TName, true);
	}

	if(IsUnCompiled){
		// remove coordinate system header variables //
		for(unsigned int i = 0; i < HeaderVars.size(); i++){
			if(HeaderVars[i]->GetName() == L"CoordinateSystem"){
				HeaderVars.erase(HeaderVars.begin()+i);
				break;
			}
		}



		// re-save the file structure //
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
	wstring path = FileSystem::Get()->SearchForFile(FILEGROUP_TEXTURE, file, modelextension, false);

	if(path == L""){
		// the file wasn't found //
		Logger::Get()->Error(L"ObjectLoader: LoadTextureDefinitionFile: failed to find file: "+file);
		return result;
	}

	// use ObjectFileParser class to get structures from the file //
	vector<shared_ptr<NamedVariableList>> HeaderVars;
	vector<shared_ptr<ObjectFileObject>> structure =  ObjectFileProcessor::ProcessObjectFile(path, HeaderVars);

	wstring headvar_str = L"";

	// parent name //
	// create //
	result = new TextureDefinition();

	wstring DefaultAnimationName;

	// interpret header vars //
	for(unsigned int i = 0; i < HeaderVars.size(); i++){
		if(HeaderVars[i]->GetName() == L"Author"){
			// set author //
			if(!HeaderVars[i]->GetValue().ConvertAndAssingToVariable<wstring>(result->Creator)){

				// dang //
				DEBUG_BREAK;
				Logger::Get()->Error(L"LoadTextureDefinitionFile: invalid header value (cannot be cast to right type, check definition) named: "+
					HeaderVars[i]->GetName());
			}
		}
		if(HeaderVars[i]->GetName() == L"Texture-Name"){
			// store the texture's name //
			if(!HeaderVars[i]->GetValue().ConvertAndAssingToVariable<wstring>(result->Name)){

				// dang //
				DEBUG_BREAK;
				Logger::Get()->Error(L"LoadTextureDefinitionFile: invalid header value (cannot be cast to right type, check definition) named: "+
					HeaderVars[i]->GetName());
			}
		}
		if(HeaderVars[i]->GetName() == L"Texture-Type"){
			// check type //
			if(!HeaderVars[i]->GetValue().ConvertAndAssingToVariable<wstring>(headvar_str)){

				// dang //
				DEBUG_BREAK;
				Logger::Get()->Error(L"LoadTextureDefinitionFile: invalid header value (cannot be cast to right type, check definition) named: "+
					HeaderVars[i]->GetName());

				result->Type = GameObject::Model::GetFlagFromTextureTypeName(L"error");
				continue;
			}
			result->Type = GameObject::Model::GetFlagFromTextureTypeName(headvar_str);
		}
		if(HeaderVars[i]->GetName() == L"Width"){
			// store width //
			if(!HeaderVars[i]->GetValue().ConvertAndAssingToVariable<int>(result->Width)){

				// dang //
				DEBUG_BREAK;
				Logger::Get()->Error(L"LoadTextureDefinitionFile: invalid header value (cannot be cast to right type, check definition) named: "+
					HeaderVars[i]->GetName());
			}
		}
		if(HeaderVars[i]->GetName() == L"Height"){
			// store height //
			if(!HeaderVars[i]->GetValue().ConvertAndAssingToVariable<int>(result->Height)){

				// dang //
				DEBUG_BREAK;
				Logger::Get()->Error(L"LoadTextureDefinitionFile: invalid header value (cannot be cast to right type, check definition) named: "+
					HeaderVars[i]->GetName());
			}
		}
		if(HeaderVars[i]->GetName() == L"ActualExtension"){
			// store image's extension //
			if(!HeaderVars[i]->GetValue().ConvertAndAssingToVariable<wstring>(result->ImageExtension)){

				// dang //
				DEBUG_BREAK;
				Logger::Get()->Error(L"LoadTextureDefinitionFile: invalid header value (cannot be cast to right type, check definition) named: "+
					HeaderVars[i]->GetName());
			}
		}
		if(HeaderVars[i]->GetName() == L"Animation"){
			// check does have animation //
			if(!HeaderVars[i]->GetValue().IsConversionAllowedNonPtr<wstring>()){
				// no animation //
				result->HasAnimation = false;
				continue;
			}

			// there is a animation //
			result->HasAnimation = true;
			DefaultAnimationName = (wstring)HeaderVars[i]->GetValue();
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

DLLEXPORT void Leviathan::ObjectLoader::CreateTestCubeToScene(Ogre::SceneManager* scene, string meshname){


	Ogre::ManualObject* TestModel(NULL);

	// create object to scene manager //
	TestModel = scene->createManualObject(meshname+"_manual");

	// we do not want to update this later //
	TestModel->setDynamic(false);

	float lSize = 0.4f;
	TestModel->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_TRIANGLE_LIST);
	{
		float cp = 1.0f * lSize ;
		float cm = -1.0f * lSize;

		TestModel->position(cm, cp, cm);// a vertex
		TestModel->colour(Ogre::ColourValue(0.0f,1.0f,0.0f,1.0f));
		TestModel->position(cp, cp, cm);// a vertex
		TestModel->colour(Ogre::ColourValue(1.0f,1.0f,0.0f,1.0f));
		TestModel->position(cp, cm, cm);// a vertex
		TestModel->colour(Ogre::ColourValue(1.0f,0.0f,0.0f,1.0f));
		TestModel->position(cm, cm, cm);// a vertex
		TestModel->colour(Ogre::ColourValue(0.0f,0.0f,0.0f,1.0f));

		TestModel->position(cm, cp, cp);// a vertex
		TestModel->colour(Ogre::ColourValue(0.0f,1.0f,1.0f,1.0f));
		TestModel->position(cp, cp, cp);// a vertex
		TestModel->colour(Ogre::ColourValue(1.0f,1.0f,1.0f,1.0f));
		TestModel->position(cp, cm, cp);// a vertex
		TestModel->colour(Ogre::ColourValue(1.0f,0.0f,1.0f,1.0f));
		TestModel->position(cm, cm, cp);// a vertex
		TestModel->colour(Ogre::ColourValue(0.0f,0.0f,1.0f,1.0f));

		TestModel->triangle(0,1,2);
		TestModel->triangle(2,3,0);
		TestModel->triangle(4,6,5);
		TestModel->triangle(6,4,7);

		TestModel->triangle(0,4,5);
		TestModel->triangle(5,1,0);
		TestModel->triangle(2,6,7);
		TestModel->triangle(7,3,2);

		TestModel->triangle(0,7,4);
		TestModel->triangle(7,0,3);
		TestModel->triangle(1,5,6);
		TestModel->triangle(6,2,1);	
	}
	TestModel->end();

	// axes //
	TestModel->begin("BaseWhiteNoLighting",Ogre::RenderOperation::OT_LINE_LIST);
	{
		float lAxeSize = 2.0f * lSize;
		TestModel->position(0.0f, 0.0f, 0.0f);
		TestModel->colour(Ogre::ColourValue::Red);
		TestModel->position(lAxeSize, 0.0f, 0.0f);
		TestModel->colour(Ogre::ColourValue::Red);
		TestModel->position(0.0f, 0.0f, 0.0f);
		TestModel->colour(Ogre::ColourValue::Green);
		TestModel->position(0.0, lAxeSize, 0.0);
		TestModel->colour(Ogre::ColourValue::Green);
		TestModel->position(0.0f, 0.0f, 0.0f);
		TestModel->colour(Ogre::ColourValue::Blue);
		TestModel->position(0.0, 0.0, lAxeSize);
		TestModel->colour(Ogre::ColourValue::Blue);

		TestModel->index(0);
		TestModel->index(1);
		TestModel->index(2);
		TestModel->index(3);
		TestModel->index(4);
		TestModel->index(5);
	}
	TestModel->end();

	TestModel->convertToMesh(meshname, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
}

DLLEXPORT void Leviathan::ObjectLoader::AddTestCubeToScenePositions(Ogre::SceneManager* scene, vector<Float3> &positions, const string &meshname){

	for(size_t i = 0; i < positions.size(); i++){

		Ogre::Entity* Entity = scene->createEntity(meshname);
		Ogre::SceneNode* Node = scene->getRootSceneNode()->createChildSceneNode();
		Node->attachObject(Entity);
		Node->translate(positions[i].X, positions[i].Y, positions[i].Z);
	}
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



