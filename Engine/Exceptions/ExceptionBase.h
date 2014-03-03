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
		EXCEPTIONTYPE_INVALIDACCESS, EXCEPTIONTYPE_NOTFOUND};

	class ExceptionBase : public exception{
	public:
		DLLEXPORT ExceptionBase();
		DLLEXPORT ExceptionBase(const wstring &message, int val, const wstring &source);
		DLLEXPORT ExceptionBase(const ExceptionBase &other);
		DLLEXPORT virtual ~ExceptionBase();

		DLLEXPORT virtual wstring* GetMessage();
		DLLEXPORT virtual wstring Get() const;
		DLLEXPORT virtual wstring* GetSource();
		DLLEXPORT virtual wstring Source() const;
		DLLEXPORT virtual int GetValue() const;

		DLLEXPORT virtual const char* what();
		//DLLEXPORT virtual const wchar_t* what() const;

		DLLEXPORT virtual void PrintToLog() const;

		DLLEXPORT virtual EXCEPTIONTYPE GetType() const;
	protected:
		unique_ptr<wstring> Message;
		unique_ptr<wstring> SourceFunction;
		string ConvertUtility;

		int ErrorValue;

		// store type //
		EXCEPTIONTYPE type;
	};

}
#endif
