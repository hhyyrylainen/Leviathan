#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_TOKEN
#include "Token.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::Token::Token(){
	_IsCleaner = false;
	parenttoken = NULL;
}

DLLEXPORT Leviathan::Token::Token(const wstring& data){
	_IsCleaner = false;
	parenttoken = NULL;
	Data = data;
}

DLLEXPORT Leviathan::Token::~Token(){
	if(_IsCleaner){
		// delete sub tokens //
		SAFE_DELETE_VECTOR(SubTokens);
	}
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Token::CreateSubToken(Token* toadd){
	// set as parent //
	toadd->parenttoken = this;
	// add to sub tokens //
	SubTokens.push_back(toadd);
	return true;
}

DLLEXPORT Token* Leviathan::Token::GetParentToken(){
	return parenttoken;
}
// ------------------------------------ //
DLLEXPORT int Leviathan::Token::GetSubTokenCount(){
	return SubTokens.size();
}

DLLEXPORT Token* Leviathan::Token::GetSubToken(int slot){
	return SubTokens[(unsigned int)slot];
}

DLLEXPORT void Leviathan::Token::ClearSubPointers(bool dodelete /*= true*/){
	if(dodelete){
		// delete sub tokens //
		SAFE_DELETE_VECTOR(SubTokens);
	} else {
		// remove parent //
		for(unsigned int i = 0; i < SubTokens.size(); i++){
			SubTokens[i]->parenttoken = NULL;
		}
		SubTokens.clear();
	}
}

// ------------------------------------ //
DLLEXPORT const wstring& Leviathan::Token::GetData() const{
	return Data;
}

DLLEXPORT void Leviathan::Token::SetData(const wstring &val){
	Data = val;
}

DLLEXPORT wstring& Leviathan::Token::GetChangeableData(){
	return Data;
}

// ------------------------------------ //






