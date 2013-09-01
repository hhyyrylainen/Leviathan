#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RENDERING_TEXTUREMANAGER
#include "TextureManager.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "Graphics.h"

TextureManager::TextureManager(bool Main, Graphics* graph){
	assert(Main);
	// store for later use //
	GraphInter = graph;

	// generate error texture //

	// make 500x500 blue and black texture //

	// colors for error //
	vector<Int3> colors;
	colors.push_back(Int3(114,194,255));
	colors.push_back(Int3(0,0,0));

	ManagedTexture* errorimage = TextureGenerator::GenerateCheckerBoard(500,500, 2, 10, colors);
	// store as error texture ptr //
	ErrorTexture = shared_ptr<ManagedTexture>(errorimage);
}
TextureManager::~TextureManager(){
	Release();
}
// ------------------------------------ //
bool TextureManager::Init(const wstring &basedir, int texttimeout, int textunload){
	BaseDir = basedir;
	InActiveTime = texttimeout;
	UnLoadTime = textunload;
	return true;
}
void TextureManager::Release(){
	// release textures, smart pointers should do this when we clear arrays //
	NonInitialized.clear(); 

	LatestFound.clear();

	LastUsed.clear();
	Expiring.clear();
	Unloaded.clear();

	Utility.clear();
	ErrorTexture.reset();
	VolatileGenerated.clear();
}
// ------------------------------------ //
void TextureManager::TimePass(int mspassed){
	// these should be in this order to not increase time too much //
	for(unsigned int i = 0; i < Unloaded.size(); i++){
		Unloaded[i]->UnusedTime += mspassed;
		// no where to move, except maybe erase completely //
	}

	for(unsigned int i = 0; i < Expiring.size(); i++){
		Expiring[i]->UnusedTime += mspassed;
		// check should it be moved to tertiary vector //
		if(Expiring[i]->UnusedTime > UnLoadTime){
			// unload and move to unloaded vector //
			Expiring[i]->UnLoad(false);
			Unloaded.push_back(Expiring[i]);
			Expiring.erase(Expiring.begin()+i);
			i--;
		}		
	}

	// add the time passed to textures //
	for(unsigned int i = 0; i < LastUsed.size(); i++){
		LastUsed[i]->UnusedTime += mspassed;
		// check should it be moved to secondary vector //
		if(LastUsed[i]->UnusedTime > InActiveTime){
			// move to expiring vector //
			Expiring.push_back(LastUsed[i]);
			LastUsed.erase(LastUsed.begin()+i);
			i--;
		}
	}

	// add time passed to volatile //
	for(size_t i = 0; i < VolatileGenerated.size(); i++){

		VolatileGenerated[i]->UnusedTime += mspassed;

		if(VolatileGenerated[i]->UnusedTime >= 60*1000){
			// hasn't been used for a minute //
			// erase it (if somebody misses it they will regenerate it //
#ifdef _DEBUG
			Logger::Get()->Info(L"TextureManager: unloading volatile, source: "+*VolatileGenerated[i]->GetSourceFile());
#endif // _DEBUG


			VolatileGenerated[i]->UnLoad(true);

			VolatileGenerated.erase(VolatileGenerated.begin()+i);
			i--;
		}
	}

	// utility vector won't be processed because it SHOULDN'T be moved anywhere //
}
// ------------------------------------ //
ID3D11ShaderResourceView* TextureManager::GetTextureView(int id, int whichfirst, bool nooldsearch){
	// call the other finding functions and just change output a little //
	ID3D11ShaderResourceView* temp = GetTexture(id, whichfirst, nooldsearch)->GetView();

	return temp;
}
shared_ptr<ManagedTexture> TextureManager::GetTexture(int id, int whichfirst, bool nooldsearch){
	if(!nooldsearch){
		// check TEXTUREMANAGER_LATEST_SEARCH_SIZE latest founds //
		for(unsigned int i = 0; i < LatestFound.size(); i++){
			if(LatestFound[i]->GetID() == id){
				// return //
				LatestFound[i]->UnusedTime = 0;
				if(LatestFound[i]->GetErrorState() != TEXTURE_ERROR_STATE_NONE)
					return ErrorTexture;
				return LatestFound[i];
			}
		}
	}
	// check the vector first that was specified with whichfirst //
	bool latestsearched = false, utilitysearched = false, oldersearched = false, unloadedsearched = false, volatilesearched = false;

	switch(whichfirst){
	case TEXTUREMANAGER_SEARCH_LATEST:
		{
			latestsearched = true;
			shared_ptr<ManagedTexture> tempresult = SearchLastUsed(id);
			if(tempresult.get() != NULL){
				// check error state and possibly return error //
				if(tempresult->GetErrorState() != TEXTURE_ERROR_STATE_NONE)
					return ErrorTexture;
				// return result //
				return tempresult;
			}
		}
	break;
	case TEXTUREMANAGER_SEARCH_UTILITY:
		{
			utilitysearched = true;
			shared_ptr<ManagedTexture> tempresult = SearchUtility(id);
			if(tempresult.get() != NULL){
				// check error state and possibly return error //
				if(tempresult->GetErrorState() != TEXTURE_ERROR_STATE_NONE)
					return ErrorTexture;
				// return result //
				return tempresult;
			}
		}
	break;
	case TEXTUREMANAGER_SEARCH_OLDER:
		{
			oldersearched = true;
			shared_ptr<ManagedTexture> tempresult = SearchExpiring(id);
			if(tempresult.get() != NULL){
				// check error state and possibly return error //
				if(tempresult->GetErrorState() != TEXTURE_ERROR_STATE_NONE)
					return ErrorTexture;
				// return result //
				return tempresult;
			}
		}
	break;
	case TEXTUREMANAGER_SEARCH_UNLOADED:
		{
			unloadedsearched = true;
			shared_ptr<ManagedTexture> tempresult = SearchUnloaded(id);
			if(tempresult.get() != NULL){
				// check error state and possibly return error //
				if(tempresult->GetErrorState() != TEXTURE_ERROR_STATE_NONE)
					return ErrorTexture;
				// return result //
				return tempresult;
			}
		}
	break;
	case TEXTUREMANAGER_SEARCH_VOLATILEGENERATED:
		{
			volatilesearched = true;
			shared_ptr<ManagedTexture> tempresult = SearchVolatile(id);
			if(tempresult.get() != NULL){
				// check error state and possibly return error //
				if(tempresult->GetErrorState() != TEXTURE_ERROR_STATE_NONE)
					return NULL;
				// return result //
				return tempresult;
			}
		}
		break;
	}
	// it haven't been found, so search from newest to oldest (if not already) //
	if(!latestsearched){
		shared_ptr<ManagedTexture> tempresult = SearchLastUsed(id);
		if(tempresult.get() != NULL){
			// check error state and possibly return error //
			if(tempresult->GetErrorState() != TEXTURE_ERROR_STATE_NONE)
				return ErrorTexture;
			// return result //
			return tempresult;
		}
	}
	if(!oldersearched){
		shared_ptr<ManagedTexture> tempresult = SearchExpiring(id);
		if(tempresult.get() != NULL){
			// check error state and possibly return error //
			if(tempresult->GetErrorState() != TEXTURE_ERROR_STATE_NONE)
				return ErrorTexture;
			// return result //
			return tempresult;
		}
	}
	if(!utilitysearched){
		shared_ptr<ManagedTexture> tempresult = SearchUtility(id);
		if(tempresult.get() != NULL){
			// check error state and possibly return error //
			if(tempresult->GetErrorState() != TEXTURE_ERROR_STATE_NONE)
				return ErrorTexture;
			// return result //
			return tempresult;
		}
	}
	if(!unloadedsearched){
		shared_ptr<ManagedTexture> tempresult = SearchUnloaded(id);
		if(tempresult.get() != NULL){
			// check error state and possibly return error //
			if(tempresult->GetErrorState() != TEXTURE_ERROR_STATE_NONE)
				return ErrorTexture;
			// return result //
			return tempresult;
		}
	}
	// nothing found, return error texture //
	return ErrorTexture;
}
// private searching functions //
shared_ptr<ManagedTexture> TextureManager::SearchLastUsed(int id){
	for(unsigned int i = 0; i < LastUsed.size(); i++){
		if(LastUsed[i]->GetID() == id){
			LastUsed[i]->UnusedTime = 0;
			// check load //
			if(!LastUsed[i]->IsLoaded()){
				// try to load //
#ifdef _DEBUG
				Logger::Get()->Info(L"TextureManager: Initializing GetTextureView value");
#endif
				LastUsed[i]->Load(NULL);
				RemoveFromUninitialized(id);
			}
			AddToLatest(LastUsed[i]);
			return LastUsed[i];
		}
	}
	return NULL;
}
shared_ptr<ManagedTexture> TextureManager::SearchExpiring(int id){
	for(unsigned int i = 0; i < Expiring.size(); i++){
		if(Expiring[i]->GetID() == id){
			// move to LastUsed vector, and reset unused time //
			Expiring[i]->UnusedTime = 0;
			// check load //
			if(!Expiring[i]->IsLoaded()){
				// try to load //
#ifdef _DEBUG
				Logger::Get()->Info(L"TextureManager: Initializing GetTextureView value");
#endif
				Expiring[i]->Load(NULL);
				RemoveFromUninitialized(id);
			}
			LastUsed.push_back(Expiring[i]);
			AddToLatest(Expiring[i]);
			Expiring.erase(Expiring.begin()+i);
			return LastUsed.back();
		}
	}
	return NULL;
}
shared_ptr<ManagedTexture> TextureManager::SearchUnloaded(int id){
	for(unsigned int i = 0; i < Unloaded.size(); i++){
		if(Unloaded[i]->GetID() == id){
			Unloaded[i]->UnusedTime = 0;
			// check load //
			if(!Unloaded[i]->IsLoaded()){
				// try to load //
#ifdef _DEBUG
				Logger::Get()->Info(L"TextureManager: Initializing GetTextureView value");
#endif
				Unloaded[i]->Load(NULL);
				RemoveFromUninitialized(id);
			}
			LastUsed.push_back(Unloaded[i]);
			Unloaded[i]->UnusedTime = 0;
			AddToLatest(Unloaded[i]);
			return LastUsed.back();
		}
	}
	return NULL;
}
shared_ptr<ManagedTexture> TextureManager::SearchUtility(int id){
	for(unsigned int i = 0; i < Utility.size(); i++){
		if(Utility[i]->GetID() == id){
			// check load //
			if(!Utility[i]->IsLoaded()){
				// try to load //
#ifdef _DEBUG
				Logger::Get()->Info(L"TextureManager: Initializing GetTextureView value");
#endif
				Utility[i]->Load(NULL);
				RemoveFromUninitialized(id);
			}
			AddToLatest(Utility[i]);
			return Utility[i];
		}
	}
	return NULL;
}
shared_ptr<ManagedTexture> Leviathan::TextureManager::SearchVolatile(int id){
	for(unsigned int i = 0; i < VolatileGenerated.size(); i++){
		if(VolatileGenerated[i]->GetID() == id){
			// reset unused time and return //
			VolatileGenerated[i]->UnusedTime = 0;
			return VolatileGenerated[i];
		}
	}
	return NULL;
}

DLLEXPORT bool Leviathan::TextureManager::AddVolatileGenerated(const int &ID, const wstring &source, ID3D11ShaderResourceView* texture, 
	const TEXTURETYPE &type)
{
	// add to vector //
	//VolatileGenerated.push_back(shared_ptr<ManagedTexture>(new ManagedTexture(ID, texture, source, type)));
	DEBUG_BREAK;

	return true;
}

DLLEXPORT void Leviathan::TextureManager::UnloadVolatile(const int &ID){
	// loop the vector and remove matching id //
	for(size_t i = 0; i < VolatileGenerated.size(); i++){
		// compare id //
		if(VolatileGenerated[i]->GetID() == ID){
			// erase it (if somebody misses it they will regenerate it //
			// needs to use force unload //
			VolatileGenerated[i]->UnLoad(true);

			VolatileGenerated.erase(VolatileGenerated.begin()+i);
			break;
		}
	}
}

void TextureManager::RemoveFromUninitialized(int id){
	for(unsigned int i = 0; i < NonInitialized.size(); i++){
		if(NonInitialized[i]->GetID() == id){

			NonInitialized.erase(NonInitialized.begin()+i);
			return;
		}
	}

}
void TextureManager::AddToLatest(const shared_ptr<ManagedTexture>& toadd){
	// check is it already there //
	for(unsigned int i = 0; i < LatestFound.size(); i++){
		if(LatestFound[i]->GetID() == toadd->GetID()){
			// it's already there! //
			return;
		}
	}

	// check is it at capacity //
	if(LatestFound.size() >= TEXTUREMANAGER_LATEST_SEARCH_SIZE){
		// remove oldest element (first element) //
		LatestFound.erase(LatestFound.begin());
	}
	// add //
	LatestFound.push_back(toadd);
}

// ------------------------------------ //
int TextureManager::LoadTexture(wstring& path, const TEXTURETYPE &type, bool loadnow){
	// create new texture object //
	int id = IDFactory::GetID();
	ManagedTexture* temp = new ManagedTexture(path, id, type);

	// add to last used //
	shared_ptr<ManagedTexture> tempptr = shared_ptr<ManagedTexture>(temp);

	LastUsed.push_back(tempptr);

	if(!loadnow){
		NonInitialized.push_back(tempptr);
	} else {
		// call texture to load itself //
		temp->Load(NULL);
	}

	return id;
}



// ------------------------------------ //