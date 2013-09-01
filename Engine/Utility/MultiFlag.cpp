#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_MULTIFLAG
#include "MultiFlag.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
MultiFlag::MultiFlag(){

}
MultiFlag::MultiFlag(vector<shared_ptr<Flag>>& toset) : Flags(toset){
}
MultiFlag::~MultiFlag(){
	// no need to clear //
}
// ------------------------------------ //
void MultiFlag::SetFlag(const Flag &flag){
	Flags.push_back(shared_ptr<Flag>(new Flag(flag)));
}
void MultiFlag::UnsetFlag(const Flag &flag){
	for(unsigned int i = 0; i < Flags.size(); i++){
		if(*Flags[i] == flag){
			//SAFE_DELETE(Flags[i]);
			Flags.erase(Flags.begin()+i);
		}
	}
}
// ------------------------------------ //
bool MultiFlag::IsSet(int value){
	for(unsigned int i = 0; i < Flags.size(); i++){
		if(Flags[i]->Value == value){
			return true;
		}
	}
	return false;
}
// ------------------------------------ //

DLLEXPORT void Leviathan::MultiFlag::ClearFlags(){
	// using smart pointers, safe to just clear //
	Flags.clear();
}

// ------------------------------------ //
vector<shared_ptr<Flag>> MultiFlag::GetFlags(){
	return Flags;
}