#ifndef LEVIATHAN_EXCEPTIONNULLPTR
#define LEVIATHAN_EXCEPTIONNULLPTR
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Exceptions/ExceptionBase.h"

namespace Leviathan{
	// base exception for other exceptions //
	class ExceptionNULLPtr : public ExceptionBase{
	public:
		DLLEXPORT ExceptionNULLPtr(const wstring& message, int val, const wstring& sourcefunction, void* InvalidPtr);
		DLLEXPORT ExceptionNULLPtr(const ExceptionNULLPtr &other);
		DLLEXPORT ~ExceptionNULLPtr();

		DLLEXPORT void* GetInvalidPtr() const;

		DLLEXPORT void PrintToLog() const;
	private:
		// specific to this exception //
		void* InvalidPointer;

	};

}
#endif
