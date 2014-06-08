#ifndef LEVIATHAN_EXCEPTIONINVALIDSTATE
#define LEVIATHAN_EXCEPTIONINVALIDSTATE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Exceptions/ExceptionBase.h"

namespace Leviathan{
	// base exception for other exceptions //
	class ExceptionInvalidState : public ExceptionBase{
	public:
		DLLEXPORT ExceptionInvalidState(const wstring &message, int val, const wstring& sourcefunction, const wstring &invalidstate);
		DLLEXPORT ExceptionInvalidState(const ExceptionInvalidState &other);
		DLLEXPORT ~ExceptionInvalidState();

		DLLEXPORT wstring* GetInvalidStateAsPtr();
		DLLEXPORT wstring GetInvalidStateAsWstring() const;

		DLLEXPORT void PrintToLog() const;
	protected:
		// specific to this exception //
		unique_ptr<wstring> InvalidStateDescription;

	};

}
#endif
