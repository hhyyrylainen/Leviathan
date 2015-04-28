#pragma once
// ------------------------------------ //
#include "Include.h"
#include <new>

#define OUTOFMEMORY_REQUIREDMEMORY_AMOUNT	1000

namespace Leviathan{

	class OutOfMemoryHandler{
	public:
		DLLEXPORT OutOfMemoryHandler();
		DLLEXPORT ~OutOfMemoryHandler();

		DLLEXPORT static OutOfMemoryHandler* Get();

		DLLEXPORT void HandleException(std::bad_alloc &except);

	private:
		static OutOfMemoryHandler* staticinstance;

		//! Memory that can be used to try to get through the error
		char* ReservedMemory;
	};

}

