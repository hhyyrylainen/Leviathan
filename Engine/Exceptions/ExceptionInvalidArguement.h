#ifndef LEVIATHAN_EXCEPTIONINVALIDARGUEMENT
#define LEVIATHAN_EXCEPTIONINVALIDARGUEMENT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Exceptions\ExceptionBase.h"

namespace Leviathan{
	// base exception for other exceptions //
	class ExceptionInvalidArguement : public ExceptionBase{
	public:
		DLLEXPORT ExceptionInvalidArguement::ExceptionInvalidArguement(const wstring &message, int val, const wstring& sourcefunction, 
			const wstring &InvalidArg, const wstring &invalidvalue);
		DLLEXPORT ExceptionInvalidArguement::ExceptionInvalidArguement(const ExceptionInvalidArguement &other);
		DLLEXPORT ExceptionInvalidArguement::~ExceptionInvalidArguement();

		DLLEXPORT const wstring* GetInvalidAsPtr() const;
		DLLEXPORT wstring GetInvalid() const;

		DLLEXPORT const wstring* GetInvalidValueAsPtr() const;
		DLLEXPORT wstring GetInvalidAsWstring() const;

		DLLEXPORT void PrintToLog() const;
	protected:
		// specific to this exception //
		unique_ptr<wstring> InvalidParameter;
		unique_ptr<wstring> VisualizedValue;

	};

}
#endif