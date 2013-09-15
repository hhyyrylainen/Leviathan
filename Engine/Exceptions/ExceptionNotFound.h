#ifndef LEVIATHAN_EXCEPTIONNOTFOUND
#define LEVIATHAN_EXCEPTIONNOTFOUND
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Exceptions\ExceptionBase.h"

namespace Leviathan{
	// base exception for other exceptions //
	class ExceptionNotFound : public ExceptionBase{
	public:
		DLLEXPORT ExceptionNotFound::ExceptionNotFound(const wstring &message, int val, const wstring& sourcefunction, 
			const wstring &invalidtype, const wstring &invalidvalue);
		DLLEXPORT ExceptionNotFound::ExceptionNotFound(const ExceptionNotFound &other);
		DLLEXPORT ExceptionNotFound::~ExceptionNotFound();

		DLLEXPORT const wstring* GetInvalidAsPtr() const;
		DLLEXPORT wstring GetInvalid() const;

		DLLEXPORT const wstring* GetInvalidValueAsPtr() const;
		DLLEXPORT wstring GetInvalidAsWstring() const;

		DLLEXPORT void PrintToLog() const;
	protected:
		// specific to this exception //
		unique_ptr<wstring> InvalidType;
		unique_ptr<wstring> InvalidValue;
	};

}
#endif