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
	enum EXCEPTIONTYPE {EXCEPTIONTYPE_BASE, EXCEPTIONTYPE_NULLPTR, EXCEPTIONTYPE_INVALIDARGUEMENT, EXCEPTIONTYPE_INVALIDTYPE, 
		EXCEPTIONTYPE_INVALIDACCESS};

	class ExceptionBase /*: public Object*/{
	public:
		DLLEXPORT ExceptionBase::ExceptionBase();
		DLLEXPORT ExceptionBase::ExceptionBase(const wstring &message, int val, const wstring &source);
		DLLEXPORT ExceptionBase::ExceptionBase(const ExceptionBase &other);
		DLLEXPORT virtual ExceptionBase::~ExceptionBase();

		DLLEXPORT virtual wstring* GetMessage();
		DLLEXPORT virtual wstring Get() const;
		DLLEXPORT virtual wstring* GetSource();
		DLLEXPORT virtual wstring Source() const;
		DLLEXPORT virtual int GetValue() const;

		DLLEXPORT virtual void PrintToLog() const = 0;

		DLLEXPORT virtual EXCEPTIONTYPE GetType() const;
	protected:
		unique_ptr<wstring> Message;
		unique_ptr<wstring> SourceFunction;
		int ErrorValue;

		// store type //
		EXCEPTIONTYPE type;
	};

}
#endif