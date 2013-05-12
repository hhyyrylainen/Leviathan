#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RENDERMODELHANDLER
#include "ModelHandler.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
ModelHandler::ModelHandler(){
	vector<BaseModel*> Models = vector<BaseModel*>();
	vector<int> Modelshadernumber = vector<int>();

}
ModelHandler::~ModelHandler(){
	// release all models, if not already released //
	if(Models.size() > 0){
		for(unsigned int i = 0; i < Models.size(); i++){
			Models[i]->Release();
			delete Models[i];
			Models[i] = NULL;
		}
		Models.clear();
	}

}
// ------------------------------------ //
bool ModelHandler::Init(){
	// nothing to do here //
	return true;
}
void ModelHandler::Release(){
	if(Models.size() > 0){
		for(unsigned int i = 0; i < Models.size(); i++){
			Models[i]->Release();
			delete Models[i];
			Models[i] = NULL;
		}
		Models.clear();
		Modelshadernumber.clear();
	}

}
// ------------------------------------ //
bool ModelHandler::AddModel(BaseModel* model, int shadernumber, int& returnindex){
	Models.push_back(model);
	Modelshadernumber.push_back(shadernumber);
	returnindex = Models.size()-1;
	return true;
}
void ModelHandler::ChangeShader(int returnindex, int newshaderindex){
	Modelshadernumber[returnindex] = newshaderindex;
}
void ModelHandler::RemoveModel(int returnindex){
	Models[returnindex]->Release();
	delete Models[returnindex];
	Models[returnindex] = NULL;

	Models.erase(Models.begin()+returnindex);
	Modelshadernumber.erase(Modelshadernumber.begin()+returnindex);
}
BaseModel* ModelHandler::GetModel(int returnindex){
	return Models[returnindex];
}
// ------------------------------------ //

// ------------------------------------ //