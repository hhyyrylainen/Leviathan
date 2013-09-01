#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_OBJECT_MANAGER
#include "ObjectManager.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
ObjectManager::ObjectManager(){
	Updated = false;
}
ObjectManager::~ObjectManager(){
	Release();
}
// ------------------------------------ //
bool ObjectManager::Init(){

	return true;
}
void ObjectManager::Release(){
	RenderObjects.clear();

	// delete objects //
	// smart pointers!, don't actually delete //
	Objects.clear();
}
// ------------------------------------ //
bool ObjectManager::AddObject(BaseObject* obj){
	if(obj == NULL){

		Logger::Get()->Error(L"ObjectManager: AddObject: Trying to add NULL as object");
		return false;
	}
	Updated = true;

	Objects.push_back(shared_ptr<BaseObject>(obj));
	return true;
}
bool ObjectManager::AddObject(shared_ptr<BaseObject> obj){
	Updated = true;

	Objects.push_back(shared_ptr<BaseObject>(obj));
	return true;
}

bool ObjectManager::Delete(int id){

	for(unsigned int i = 0; i < Objects.size(); i++){
		if(Objects[i]->ID == id){
			Updated = true;
#ifdef _DEBUG
			Logger::Get()->Info(L"Object deleted, id:"+Convert::IntToWstring(id), true);
#endif
			// set id to ERROR_DELETED so that reference holders can check is it deleted //
			Objects[i]->ID = ERROR_DELETED;
			Objects.erase(Objects.begin()+i);

			return true;
		}
	}
	return false;
}
bool ObjectManager::DeleteByIndex(unsigned int index){
	ARR_INDEX_CHECK(index, Objects.size()){
		// valid //
		Objects.erase(Objects.begin()+index);

		return true;
	}
	return false;
}
// ------------------------------------ //
vector<BaseRenderable*>* ObjectManager::GetRenderableObjects(){
	UpdateArrays();
	return &RenderObjects;
}
int ObjectManager::SearchGetIndex(int id){
	for(unsigned int i = 0; i < Objects.size(); i++){
		if(Objects[i]->ID == id){
			return i;
		}
	}
	return -1;
}
const shared_ptr<BaseObject>& ObjectManager::Search(int id){
	for(unsigned int i = 0; i < Objects.size(); i++){
		if(Objects[i]->ID == id){
			return Objects[i];
		}
	}
	//return shared_ptr<BaseObject>(NULL);
	return Nullptr;
}

const shared_ptr<BaseObject>& ObjectManager::Get(unsigned int index){
	ARR_INDEX_CHECK(index,Objects.size()){
		return Objects[index];
	}
	return Nullptr;
}
// ------------------------------------ //
inline int ObjectManager::GetObjectCount(){
	return Objects.size();
}

// ------------------------------------ //
void ObjectManager::UpdateArrays(){
	if(!Updated)
		return;

	Updated = false;

	// clear renderable objects, and read objects //
	RenderObjects.clear();

	for(unsigned int i = 0; i  < Objects.size(); i++){

		DEBUG_BREAK;
		// objects to have flags

		// do dynamic cast //
		BaseRenderable* robj = NULL;
		try {
			robj = dynamic_cast<BaseRenderable*>(Objects[i].get());
		}
		catch(...){
			// cast failed, remove object and give error //
			Logger::Get()->Error(L"ObjectManager: UpdateArrays: object cast failed, id:"+Convert::IntToWstring(Objects[i]->ID)+L" type: "+Convert::IntToWstring(Objects[i]->Type), Objects[i]->ID, true);

			// smart pointer should delete memory //
			if(Objects[i].use_count() > 1){
				// somebody else has this too! //
				Logger::Get()->Error(L"ObjectManager: UpdateArrays object cast failed: object is referenced elsewhere! id:"+Convert::IntToWstring(Objects[i]->ID), Objects[i].use_count(), true);
			}
			Objects.erase(Objects.begin()+i);
			i--;
			continue;
		}
		RenderObjects.push_back(robj);
			
		
	}

}

shared_ptr<BaseObject> Leviathan::ObjectManager::Nullptr(NULL);
