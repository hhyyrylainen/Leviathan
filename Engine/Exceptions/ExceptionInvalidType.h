#ifndef LEVIATHAN_EXCEPTIONINVALIDTYPE
#define LEVIATHAN_EXCEPTIONINVALIDTYPE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Exceptions/ExceptionBase.h"

namespace Leviathan{
	// exception thrown when variable is of invalid type //
	class ExceptionInvalidType : public ExceptionBase{
	public:
		DLLEXPORT ExceptionInvalidType::ExceptionInvalidType(const wstring &message, int val, const wstring& sourcefunction, 
			const wstring &invalidvariable, const wstring &invalidtypedescription);
		DLLEXPORT ExceptionInvalidType::ExceptionInvalidType(const ExceptionInvalidType &other);
		DLLEXPORT ExceptionInvalidType::~ExceptionInvalidType();

		DLLEXPORT wstring* GetInvalidAsPtr();
		DLLEXPORT wstring GetInvalid() const;

		DLLEXPORT wstring* GetInvalidDescriptionAsPtr();
		DLLEXPORT wstring GetInvalidDescription() const;

		DLLEXPORT void PrintToLog() const;
	protected:
		// specific to this exception //
		unique_ptr<wstring> InvalidVariable;
		unique_ptr<wstring> InvalidDescription;

	};

}
#endif