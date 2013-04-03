#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_MULTIFLAG
#include "MultiFlag.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
MultiFlag::MultiFlag(){

}
MultiFlag::MultiFlag(vector<shared_ptr<Flag>>& toset){
	for(unsigned int i = 0; i < toset.size(); i++){
		Flags.push_back(toset[i]);
	}
	// clear input array //
	toset.clear();
}
MultiFlag::~MultiFlag(){
	//while(Flags.size() != 0){
	//	SAFE_DELETE(Flags[0]);
	//	Flags.erase(Flags.begin());
	//}
	// smart pointers, clear
	Flags.clear();
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
vector<shared_ptr<Flag>> MultiFlag::GetFlags(){
	return Flags;
}
// ------------------------------------ //