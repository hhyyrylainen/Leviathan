#ifndef LEVIATHAN_EXCEPTIONNULLPTR
#define LEVIATHAN_EXCEPTIONNULLPTR
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ExceptionBase.h"

namespace Leviathan{
	// base exception for other exceptions //
	class ExceptionNULLPtr : public ExceptionBase{
	public:
		DLLEXPORT ExceptionNULLPtr::ExceptionNULLPtr(const wstring& message, int val, void* InvalidPtr);
		DLLEXPORT ExceptionNULLPtr::ExceptionNULLPtr(const ExceptionNULLPtr &other);
		DLLEXPORT ExceptionNULLPtr::~ExceptionNULLPtr();

		DLLEXPORT wstring* GetMessage();
		DLLEXPORT wstring Get();
		DLLEXPORT int GetValue();
		DLLEXPORT void* GetInvalidPtr();

		DLLEXPORT void PrintToLog();

	private:
		unique_ptr<wstring> Message;
		int ErrorValue;

		void* InvalidPointer;

	};

}
#endif