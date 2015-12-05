// ------------------------------------ //
#include "OutOfMemoryHandler.h"

#include "Define.h"
#include "../Logger.h"
#include <cstdio>
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::OutOfMemoryHandler::OutOfMemoryHandler(){
	// Allocate memory to use when handling //
	try{
		ReservedMemory = new char[OUTOFMEMORY_REQUIREDMEMORY_AMOUNT];
	}
	catch (bad_alloc&){
		// Quite embarrassing, really //
		exit(321);
	}
	// Set static instance //
	staticinstance = this;
}

DLLEXPORT Leviathan::OutOfMemoryHandler::~OutOfMemoryHandler(){
	// release allocated memory //
	delete[] ReservedMemory;
	ReservedMemory = NULL;
}

OutOfMemoryHandler* Leviathan::OutOfMemoryHandler::staticinstance = NULL;
// ------------------------------------ //
DLLEXPORT  OutOfMemoryHandler* Leviathan::OutOfMemoryHandler::Get(){
	return staticinstance;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::OutOfMemoryHandler::HandleException(std::bad_alloc &except){
	// release reserved memory //
	if(ReservedMemory == NULL){
		// this is rather bad //
		DEBUG_BREAK;
	}

	SAFE_DELETE(ReservedMemory);

	// we probably now have enough memory to write to log and close and pop up a error window //
	Logger::Get()->Error("Out of memory! "+string(except.what()));
	Logger::Get()->Save();

	// maybe tell engine to try to free up memory //

	// debug build should never continue out of here
	DEBUG_BREAK;
}

// ------------------------------------ //

// ------------------------------------ //







