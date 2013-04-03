#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RENDER_BASEMODEL
#include "BaseModel.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
BaseModel::BaseModel(){
	mtype = MBASETYPE;
}
BaseModel::~BaseModel(){

}
// ------------------------------------ //
bool BaseModel::Init(ID3D11Device* dev, wstring modelfile, wstring texturefile1){
	return false;
}
void BaseModel::Release(){

}
// ------------------------------------ //
void BaseModel::Render(ID3D11DeviceContext* devcont){

}
// ------------------------------------ //
int BaseModel::GetIndexCount(){
	return 0;
}
ID3D11ShaderResourceView* BaseModel::GetColorTexture(){
	return NULL;
}
// ------------------------------------ //