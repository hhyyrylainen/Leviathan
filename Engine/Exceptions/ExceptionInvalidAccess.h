#ifndef LEVIATHAN_EXCEPTIONINVALIDACCESS
#define LEVIATHAN_EXCEPTIONINVALIDACCESS
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Exceptions\ExceptionBase.h"

namespace Leviathan{
	// base exception for other exceptions //
	class ExceptionInvalidAccess : public ExceptionBase{
	public:
		DLLEXPORT ExceptionInvalidAccess::ExceptionInvalidAccess(const wstring &message, int val, const wstring& sourcefunction, 
			const wstring &Invalidaccesstype, const wstring &invaliddefinition);
		DLLEXPORT ExceptionInvalidAccess::ExceptionInvalidAccess(const ExceptionInvalidAccess &other);
		DLLEXPORT ExceptionInvalidAccess::~ExceptionInvalidAccess();

		DLLEXPORT wstring* GetInvalidAsPtr();
		DLLEXPORT wstring GetInvalid() const;

		DLLEXPORT wstring* GetInvalidMessageAsPtr();
		DLLEXPORT wstring GetInvalidMessageAsWstring() const;

		DLLEXPORT void PrintToLog() const;
	protected:
		// specific to this exception //
		unique_ptr<wstring> InvalidAccessType;
		unique_ptr<wstring> InvalidDefinition;

	};

}
#endif