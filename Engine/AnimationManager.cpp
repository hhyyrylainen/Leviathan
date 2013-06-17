#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_ANIMATIONMANAGER
#include "AnimationManager.h"
#endif
#include "NamedVars.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::AnimationManager::AnimationManager(){
	instance = this;

}

DLLEXPORT Leviathan::AnimationManager::~AnimationManager(){
	// just in case //
	if(Inited)
		Release();
}
// ------------------------------------ //
AnimationManager* Leviathan::AnimationManager::instance = NULL;

DLLEXPORT AnimationManager* Leviathan::AnimationManager::Get(){
	return instance;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::AnimationManager::Init(){
	// false to not load ALL animations into memory //
	if(!IndexAllAnimations(false)){

		Logger::Get()->Error(L"AnimationManager: Init: failed to index animation files");
		return false;
	}
	Inited = true;
	return true;
}

DLLEXPORT void Leviathan::AnimationManager::Release(){
	// release all animations stored in memory, models will release their instances once they stop playing //
	AnimationsInMemory.clear();
	Inited = false;
}
// ------------------------------------ //
DLLEXPORT shared_ptr<AnimationBlock> Leviathan::AnimationManager::GetAnimation(const wstring& name){
	// try to find correct animation //
	// need to split name and base model name //
	WstringIterator itr(name);

	unique_ptr<wstring> modelname = itr.GetNextCharacterSequence(UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS);
	unique_ptr<wstring> animationname = itr.GetNextCharacterSequence(UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS);

	for(unsigned int i = 0; i < AnimationsInMemory.size(); i++){
		if(AnimationsInMemory[i]->GetBaseModelName() == *modelname){
			// right model animation "group" //
			if(AnimationsInMemory[i]->GetName() == *animationname){
				// right one found! //
				return AnimationBlock::CreateBlockFromLoadedAnimation(AnimationsInMemory[i]);
			}
		}
	}

	// none found, needs to create new //
	if(AnimationFiles.size() == 0){
		// needs to load the index //
		Logger::Get()->Warning(L"AnimationManager: GetAnimation: No indexed animation files", false);
		if(!IndexAllAnimations()){

			Logger::Get()->Error(L"AnimationManager: GetAnimation: failed to index animation files");
			return NULL;
		}
	}
	// lets look for a animation from our index //
	for(unsigned int i = 0; i < AnimationFiles.size(); i++){
		if(AnimationFiles[i]->AnimationName == name){
			// found //
			// load it //
			VerifyAnimLoaded(AnimationFiles[i]->SourceFile);

			// link //
			// find the current, shouldn't have moved //
			shared_ptr<LoadedAnimation> CurAnim(AnimationsInMemory.back());

			// direct link //
			AnimationFiles[i]->CorrespondingAnimation = CurAnim;

			// call again //
			return GetAnimation(name);
		}
	}

	return NULL;
}

DLLEXPORT shared_ptr<AnimationBlock> Leviathan::AnimationManager::GetAnimation(int ID){
	return NULL;
}

DLLEXPORT shared_ptr<AnimationBlock> Leviathan::AnimationManager::GetAnimationFromIndex(int ind){
	return NULL;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::AnimationManager::UnloadAnimation(const wstring& name){
	return false;
}

DLLEXPORT bool Leviathan::AnimationManager::UnloadAnimation(int ID){
	return false;
}

DLLEXPORT bool Leviathan::AnimationManager::UnloadAnimationFromIndex(int ind){
	return false;
}
// ------------------------------------ //
int Leviathan::AnimationManager::VerifyAnimLoaded(const wstring &file, bool SkipCheck /*= false*/){
	// needs to see if this file is already loaded //
	if(!SkipCheck){
		if(IsSourceFileLoaded(file)){
			// file is loaded, needs to do nothing //
			return 1;
		}
	}

	// actual loading of the file //
	vector<shared_ptr<NamedVar>> HeaderVars;

	vector<shared_ptr<ObjectFileObject>> Objects = ObjectFileProcessor::ProcessObjectFile(file, HeaderVars);

	wstring twval = L"";
	int tval = 0;
	// file header variables //
	wstring AnimationName = L"";
	wstring BaseModelName = L"";
	bool NeedToChangeCoordinateSystem = false;
	bool IsUnCompiled = false;




	vector<shared_ptr<GameObject::SkeletonBone>> LoadedBones;
	shared_ptr<LoadedAnimation> CurrentlyLoading(new LoadedAnimation());

	int RetVal = 1;

	// handle header variables //
	for(unsigned int i = 0; i < HeaderVars.size(); i++){
		if(HeaderVars[i]->CompareName(L"FileType")){
			// check is this right type //
			HeaderVars[i]->GetValue(tval, twval);

			if(twval != L"BoneAnimation"){
				// invalid file //
				Logger::Get()->Error(L"AnimationManager: VerifyAnimLoaded: FileType is invalid BoneAnimation expected, got: "+twval);
				RetVal = 7;
				// cleanup not required //
				return RetVal;
			}

			continue;
		}
		if(HeaderVars[i]->CompareName(L"Animation-Name")){
			HeaderVars[i]->GetValue(tval, twval);
			// store animation name //
			AnimationName = twval;

			continue;
		}
		if(HeaderVars[i]->CompareName(L"Base-Model-Name")){
			HeaderVars[i]->GetValue(tval, twval);
			// store animation name //
			BaseModelName = twval;

			continue;
		}
		if(HeaderVars[i]->CompareName(L"AnimationType")){
			HeaderVars[i]->GetValue(tval, twval);
			// maybe do something with this //

			continue;
		}
		if(HeaderVars[i]->CompareName(L"CoordinateSystem")){
			HeaderVars[i]->GetValue(tval, twval);
			// check coordinates //
			if(twval != L"LEVIATHAN"){
				// needs to change coordinates //
				NeedToChangeCoordinateSystem = true;
			} else {
				NeedToChangeCoordinateSystem = false;
			}
			continue;
		}
		if(HeaderVars[i]->CompareName(L"Model-UnCompiled")){
			HeaderVars[i]->GetValue(tval, twval);
			if(tval != 0){
				// needs to save compiled version //
				IsUnCompiled = true;
			}
			continue;
		}
	}

	// check for data validness //
	if(AnimationName.size() < 1){
		// no name found //
		// invalid file //
		Logger::Get()->Error(L"AnimationManager: VerifyAnimLoaded: File is invalid, no name for animation found, file: "+file);
		RetVal = 7;
		// cleanup not required //
		return RetVal;
	}
	if(BaseModelName.size() < 1){
		// no name found //
		// invalid file //
		Logger::Get()->Error(L"AnimationManager: VerifyAnimLoaded: File is invalid, no base model name for animation found, file: "+file);
		RetVal = 7;
		// cleanup not required //
		return RetVal;
	}

	// set data //
	CurrentlyLoading->SetName(AnimationName);
	CurrentlyLoading->SetSourceFile(file);
	CurrentlyLoading->SetBaseModelName(BaseModelName);

	// check file structure //
	if(Objects.size() != 1){
		// invalid file //
		Logger::Get()->Error(L"AnimationManager: VerifyAnimLoaded: File is invalid, didn't find exactly one BoneAnimation object, count: ", Objects.size());
		RetVal = 7;
		// cleanup not required //
		return RetVal;
	}

	ObjectFileObject* curobj = Objects[0].get();
	if(curobj == NULL){
		// maybe a bug //
		DEBUG_BREAK;
	}
	
	if(curobj->TName != L"BoneAnimation"){
		// invalid file //
		Logger::Get()->Error(L"AnimationManager: VerifyAnimLoaded: File is invalid, objects type is wrong, expected BoneAnimation, got: "+curobj->TName);
		RetVal = 7;
		
		// cleanup not required //
		return RetVal;
	}
	// find properties first //
	int KeyFrameCount = -1;
	int KeyFrameInterval = -1;
	int AnimationSpeed = -1;
	// ignore frame length if animation speed defined //
	int FrameLength = -1;



	for(unsigned int i = 0; i < curobj->Contents.size(); i++){
		ObjectFileList* Curlist = curobj->Contents[i];

		// handle based on name //
		if(Curlist->Name == L"Properties"){
			// get properties //
			if(Curlist->Variables->GetValue(L"KeyFrames", tval) > 1){
				KeyFrameCount = -1;
			} else {
				KeyFrameCount = tval;
			}

			if(Curlist->Variables->GetValue(L"KeyFrameInterval", tval) > 1){
				KeyFrameInterval = -1;
			} else {
				KeyFrameInterval = tval;
			}

			if(Curlist->Variables->GetValue(L"AnimationSpeed", tval) > 1){
				AnimationSpeed = -1;
			} else {
				AnimationSpeed = tval;
			}

			if(Curlist->Variables->GetValue(L"FrameLength", tval) > 1){
				FrameLength = -1;
			} else {
				FrameLength = tval;
			}


			continue;
		}
	}


	if(AnimationSpeed == -1 && FrameLength == -1){
		// both undefined, use default //
		// 24 frames per second //
		AnimationSpeed = 30;
		// notify //
		Logger::Get()->Warning(L"AnimationManager: VerifyAnimLoaded: File doesn't have AnimationSpeed (frames per second) or FrameLength (ms per frame) defined! "
			L"using default AnimationSpeed of 30 frames per second", false);

		CurrentlyLoading->SetAnimationFPS(AnimationSpeed);
	} else {

		// check which one to set //
		if(AnimationSpeed == -1){

			CurrentlyLoading->SetFrameMSLength(FrameLength);
		} else {

			CurrentlyLoading->SetAnimationFPS(AnimationSpeed);
		}
	}

	// set variables //
	


	// now process text blocks which contain the bones and the frames //
	for(unsigned int i = 0; i < curobj->TextBlocks.size(); i++){
		ObjectFileTextBlock* CurTB = curobj->TextBlocks[i];

		// check for bones //
		if(CurTB->Name == L"Bones"){
			// loop through lines and load bones //
			for(unsigned int ind = 0; ind < CurTB->Lines.size(); ind++){
				vector<wstring*> Tokens;

				shared_ptr<GameObject::SkeletonBone> LoadingBone(new GameObject::SkeletonBone);

				// split to tokens //
				if(LineTokeNizer::TokeNizeLine(*CurTB->Lines[ind], Tokens)){
					// error //
					DEBUG_BREAK;
				}

				// handle tokens //
				for(unsigned int tokenind = 0; tokenind < Tokens.size(); tokenind++){
					// get what this token is //
					if(Misc::WstringStartsWith(*Tokens[tokenind], L"name")){
						// get name //
						WstringIterator itr(Tokens[tokenind], false);

						unique_ptr<wstring> resultstr = itr.GetStringInQuotes(QUOTETYPE_BOTH);

						LoadingBone->SetName(*resultstr);

						continue;
					}
					if(Misc::WstringStartsWith(*Tokens[tokenind], L"parent")){
						// get parent name //
						WstringIterator itr(Tokens[tokenind], false);

						unique_ptr<wstring> resultstr = itr.GetStringInQuotes(QUOTETYPE_BOTH);

						LoadingBone->SetParentName(*resultstr);

						continue;
					}
					if(Misc::WstringStartsWith(*Tokens[tokenind], L"id")){
						// store used id as BoneGroup //
						WstringIterator itr(Tokens[tokenind], false);

						unique_ptr<wstring> resultstr = itr.GetNextNumber(DECIMALSEPARATORTYPE_NONE);

						LoadingBone->SetBoneGroup(Convert::WstringToInt(*resultstr));

						continue;
					}
					if(Misc::WstringStartsWith(*Tokens[tokenind], L"pos")){
						// expecting to load 3 floats //
						WstringIterator itr(Tokens[tokenind], false);

						unique_ptr<wstring> resultstr = itr.GetNextNumber(DECIMALSEPARATORTYPE_DOT);

						Float3 curvalue;

						curvalue.X = Convert::WstringToFloat(*resultstr);

						resultstr = itr.GetNextNumber(DECIMALSEPARATORTYPE_DOT);
						curvalue.Y = Convert::WstringToFloat(*resultstr);

						resultstr = itr.GetNextNumber(DECIMALSEPARATORTYPE_DOT);
						curvalue.Z = Convert::WstringToFloat(*resultstr);

						if(NeedToChangeCoordinateSystem){
							// swap y and z to convert from blender coordinates //
							swap(curvalue.Y, curvalue.Z);
						}

						LoadingBone->SetRestPosition(curvalue);

						continue;
					}
					if(Misc::WstringStartsWith(*Tokens[tokenind], L"dir")){
						// expecting to load 3 floats //
						WstringIterator itr(Tokens[tokenind], false);

						unique_ptr<wstring> resultstr = itr.GetNextNumber(DECIMALSEPARATORTYPE_DOT);

						Float4 curvalue;

						curvalue.X = Convert::WstringToFloat(*resultstr);

						resultstr = itr.GetNextNumber(DECIMALSEPARATORTYPE_DOT);
						curvalue.Y = Convert::WstringToFloat(*resultstr);

						resultstr = itr.GetNextNumber(DECIMALSEPARATORTYPE_DOT);
						curvalue.Z = Convert::WstringToFloat(*resultstr);

						resultstr = itr.GetNextNumber(DECIMALSEPARATORTYPE_DOT);
						curvalue.W = Convert::WstringToFloat(*resultstr);

						if(NeedToChangeCoordinateSystem){
							// swap y and z to convert from blender coordinates //
							swap(curvalue.Y, curvalue.Z);
						}

						LoadingBone->SetRestDirection(curvalue);


						continue;
					}

				}

				// add //
				LoadedBones.push_back(LoadingBone);
			}
			// set bones //
			CurrentlyLoading->SetBones(LoadedBones);
			LoadedBones.clear();
			continue;
		}

		// is a frame //
		if(Misc::WstringStartsWith(CurTB->Name, L"Frame")){
			// get frame number //
			WstringIterator itr(CurTB->Name);

			unique_ptr<wstring> framenbr = itr.GetNextNumber(DECIMALSEPARATORTYPE_NONE);
			int CurrentFrameNumber = Convert::WstringToInt(*framenbr);

			// create new frame //
			shared_ptr<AnimationFrameData> LoadingFrame(new AnimationFrameData(CurrentFrameNumber));

			// set number //
			LoadingFrame->FrameNumber = CurrentFrameNumber;

			// loop through lines and load bones //
			for(unsigned int ind = 0; ind < CurTB->Lines.size(); ind++){
				vector<wstring*> Tokens;

				unique_ptr<AnimationBoneData> LoadingBone(new AnimationBoneData());

				// split to tokens //
				if(LineTokeNizer::TokeNizeLine(*CurTB->Lines[ind], Tokens)){
					// error //
					DEBUG_BREAK;
				}

				// used to determine has a position been found //
				bool PosFound = false;

				// handle tokens //
				for(unsigned int tokenind = 0; tokenind < Tokens.size(); tokenind++){
					// get what this token is //
					if(Misc::WstringStartsWith(*Tokens[tokenind], L"id")){
						// store used id as BoneGroup //
						WstringIterator itrsecond(Tokens[tokenind], false);

						unique_ptr<wstring> resultstr = itrsecond.GetNextNumber(DECIMALSEPARATORTYPE_NONE);

						LoadingBone->BoneGroup = Convert::WstringToInt(*resultstr);

						continue;
					}
					if(Misc::WstringStartsWith(*Tokens[tokenind], L"pos")){
						// expecting to load 3 floats //
						WstringIterator itrsecond(Tokens[tokenind], false);

						unique_ptr<wstring> resultstr = itrsecond.GetNextNumber(DECIMALSEPARATORTYPE_DOT);

						Float3 curvalue;

						curvalue.X = Convert::WstringToFloat(*resultstr);

						resultstr = itrsecond.GetNextNumber(DECIMALSEPARATORTYPE_DOT);
						curvalue.Y = Convert::WstringToFloat(*resultstr);

						resultstr = itrsecond.GetNextNumber(DECIMALSEPARATORTYPE_DOT);
						curvalue.Z = Convert::WstringToFloat(*resultstr);

						if(NeedToChangeCoordinateSystem){
							// swap y and z to convert from blender coordinates //
							swap(curvalue.Y, curvalue.Z);
						}

						// set it as rest position even though it actually isn't //
						LoadingBone->Position = curvalue;

						PosFound = true;

						continue;
					}
					if(Misc::WstringStartsWith(*Tokens[tokenind], L"dir")){
						// expecting to load 3 floats //
						WstringIterator itrsecond(Tokens[tokenind], false);

						unique_ptr<wstring> resultstr = itrsecond.GetNextNumber(DECIMALSEPARATORTYPE_DOT);

						Float4 curvalue;

						curvalue.X = Convert::WstringToFloat(*resultstr);

						resultstr = itrsecond.GetNextNumber(DECIMALSEPARATORTYPE_DOT);
						curvalue.Y = Convert::WstringToFloat(*resultstr);

						resultstr = itrsecond.GetNextNumber(DECIMALSEPARATORTYPE_DOT);
						curvalue.Z = Convert::WstringToFloat(*resultstr);

						resultstr = itrsecond.GetNextNumber(DECIMALSEPARATORTYPE_DOT);
						curvalue.W = Convert::WstringToFloat(*resultstr);

						if(NeedToChangeCoordinateSystem){
							// swap y and z to convert from blender coordinates //
							swap(curvalue.Y, curvalue.Z);
						}

						LoadingBone->Direction = curvalue;

						continue;
					}
				}

				if(!PosFound){
					// no position change, copy base bone position here //

					for(size_t index = 0; i < LoadedBones.size(); index++){

						if(LoadedBones[i]->GetBoneGroup() == LoadingBone->BoneGroup)
							LoadingBone->Position = LoadedBones[i]->GetRestPosition();
					}
				}

				// put bone into frame //
				LoadingFrame->FrameBones.push_back(LoadingBone.get());
				// unlink from smart pointer //
				LoadingBone.release();
			}
			// push frame //

			CurrentlyLoading->AddNewFrame(LoadingFrame);

			continue;
		}

		// wasn't a frame? //
		DEBUG_BREAK;
	}

	// let the animation object process input //

	int retcode = CurrentlyLoading->ProcessLoadedData();
	if(retcode != 0){
		// error occurred //
		DEBUG_BREAK;
	}

	// store loaded data //
	AnimationsInMemory.push_back(CurrentlyLoading);


	// cleanup not required //
	return RetVal;
}
bool Leviathan::AnimationManager::IsSourceFileLoaded(const wstring &sourcefile){
	// maybe write a indexing vector for animations to speed up searching //
	for(unsigned int i = 0; i < AnimationsInMemory.size(); i++){
		// check does match //
		if(AnimationsInMemory[i]->GetSourceFile() == sourcefile)
			return true;
	}
	return false;
}

DLLEXPORT bool Leviathan::AnimationManager::IndexAllAnimations(bool LoadToMemory/*=false*/){
	// clear old animations, TOOO: create a method to update //
	AnimationFiles.clear();

	// lets search for all of animation files in models directory //
	vector<wstring> Files;
	FileSystem::GetFilesInDirectory(Files, FileSystem::GetModelsFolder(), L"*.levba");

	// process //
	for(unsigned int i = 0; i < Files.size(); i++){
		// check is this loaded //
		if(IsSourceFileLoaded(Files[i]))
			continue;
		if(LoadToMemory){
			// needs to load, force load //
			if(VerifyAnimLoaded(Files[i], true) != 1){
				// failed //
				Logger::Get()->Error(L"AnimationManager: IndexAllAnimations: can't index file: "+Files[i]);
				continue;
			}

			// find the current, shouldn't have moved //
			shared_ptr<LoadedAnimation> CurAnim(AnimationsInMemory.back());

			// link //
			AnimationFiles.push_back(unique_ptr<IndexedAnimation>(new IndexedAnimation(CurAnim->GetName(), Files[i])));

			// direct link //
			AnimationFiles.back()->CorrespondingAnimation = CurAnim;
		} else {

			wstring Animname = GetAnimationNameFromFile(Files[i]);

			AnimationFiles.push_back(unique_ptr<IndexedAnimation>(new IndexedAnimation(Animname, Files[i])));
		}
	}

	// in the future maybe update index here //

	// done //
	return true;
}

wstring Leviathan::AnimationManager::GetAnimationNameFromFile(const wstring &file){
	// needs to read lines until animation name has been found //

	wifstream reader;
	reader.open(file);

	wstring Name = L"";
	wstring BaseModelName = L"";

	if(!reader.is_open()){
		DEBUG_BREAK;
	}

	bool Found = false;
	bool FoundBase = false;

	unique_ptr<wchar_t> ReadCharacters(new wchar_t[400]);

	int junkftch;

	while(!(Found && FoundBase) && reader.good()){
		reader.getline(ReadCharacters.get(), 400);

		// generate wstring //
		wstring line(ReadCharacters.get());

		// check is correct line //
		if(Misc::WstringStartsWith(line, L"Animation-Name")){
			// correct line //

			// generate named var from this line //
			NamedVar linevar(line);

			linevar.GetValue(junkftch, Name);
			// done //
			Found = true;
			continue;
		}
		if(Misc::WstringStartsWith(line, L"Base-Model-Name")){
			// correct line //

			// generate named var from this line //
			NamedVar linevar(line);

			linevar.GetValue(junkftch, BaseModelName);
			// done //
			FoundBase = true;
			continue;
		}
	}

	if(!Found || !FoundBase){
		// no name, invalid file //

		DEBUG_BREAK;
		return Name;
	}
	return BaseModelName+L"[]"+Name;
}
