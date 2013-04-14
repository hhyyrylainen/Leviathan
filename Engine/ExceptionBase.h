#ifndef LEVIATHAN_EXCEPTIONBASE
#define LEVIATHAN_EXCEPTIONBASE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{
	// base exception for other exceptions //
	enum EXCEPTIONTYPE {EXCEPTIONTYPE_BASE, EXCEPTIONTYPE_NULLPTR};

	class ExceptionBase /*: public Object*/{
	public:
		DLLEXPORT ExceptionBase::ExceptionBase();
		DLLEXPORT ExceptionBase::ExceptionBase(const wstring& message, int val);
		DLLEXPORT ExceptionBase::ExceptionBase(const ExceptionBase &other);
		DLLEXPORT virtual ExceptionBase::~ExceptionBase();

		DLLEXPORT wstring* GetMessage();
		DLLEXPORT wstring Get();
		DLLEXPORT int GetValue();

		DLLEXPORT void PrintToLog();

		DLLEXPORT EXCEPTIONTYPE GetType();
	protected:
		unique_ptr<wstring> Message;
		int ErrorValue;

		// store type //
		EXCEPTIONTYPE type;
	};

}
#endif