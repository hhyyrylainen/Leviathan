#ifndef LEVIATHAN_OUTOFMEMORYHANDLER
#define LEVIATHAN_OUTOFMEMORYHANDLER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


#define OUTOFMEMORY_REQUIREDMEMORY_AMOUNT	1000

namespace Leviathan{

	class OutOfMemoryHandler /*: public Object*/{
	public:
		DLLEXPORT OutOfMemoryHandler();
		DLLEXPORT ~OutOfMemoryHandler();

		DLLEXPORT static OutOfMemoryHandler* Get();

		DLLEXPORT void HandleException(std::bad_alloc &except);

	private:
		static OutOfMemoryHandler* staticinstance;

		// memory that can be used to try to get through the error //
		char* ReservedMemory;
	};

}
#endif
