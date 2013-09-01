#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_LEVIATHAN_IDFACTORY
#include "IDFactory.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "Logger.h"
IDFactory::IDFactory(){

}
IDFactory::~IDFactory(){

}

int IDFactory::SystemID = 1;
int IDFactory::GlobalID = 1;

bool IDFactory::Busy = false;
// ------------------------------------ //
int IDFactory::GetID(){
	Wait();
	Busy = true;

	GlobalID++;
	if(GlobalID == INT_MAX){
		Logger::Get()->Error(L"IDFactory GlobalID overflow", INT_MAX);
	}

	Busy = false;
	return GlobalID;
}

int IDFactory::GetSystemID(){
	Wait();
	Busy = true;

	SystemID++;

	Busy = false;
	return SystemID;
}
// ------------------------------------ //
void IDFactory::Wait(){
	int Waittime = 0;
	while(Busy){
		Waittime++;
		if(Waittime > 100){
			Logger::Get()->Error(L"IDFactory waiting too long!", Waittime);
			break;
		}
		Sleep(1);
	}
}
// ------------------------------------ //