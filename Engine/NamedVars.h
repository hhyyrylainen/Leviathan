#ifndef LEVIATHAN_NAMEDVARS
#define LEVIATHAN_NAMEDVARS
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ExceptionInvalidArguement.h"

namespace Leviathan{

#define NAMEDVAR_RETURNVALUE_IS_INT			1
#define NAMEDVAR_RETURNVALUE_IS_WSTRING		0

	class NamedVars;

	class NamedVar{
	public:
		DLLEXPORT NamedVar();
		DLLEXPORT NamedVar(const NamedVar &other);
		DLLEXPORT NamedVar(const wstring &name, int val);
		DLLEXPORT NamedVar(const wstring &name, const wstring &val);
		DLLEXPORT NamedVar(wstring &line, vector<IntWstring*> *specialintvalues = NULL) throw (...);
		DLLEXPORT ~NamedVar();
		// ------------------------------------ //
		DLLEXPORT void SetValue(int val);
		DLLEXPORT void SetValue(const wstring &val);
		DLLEXPORT int GetValue(int &val1, wstring &val2) const;
		DLLEXPORT bool GetValue(int &val) const;
		DLLEXPORT bool GetValue(wstring &val) const;

		DLLEXPORT bool IsIntValue() const;

		DLLEXPORT wstring& GetName();
		DLLEXPORT void GetName(wstring &name) const;

		DLLEXPORT wstring* GetPointedValue();

		DLLEXPORT void SetName(const wstring &name);
		DLLEXPORT bool CompareName(const wstring &name) const;
		// ------------------------------------ //
		DLLEXPORT wstring ToText(int WhichSeparator = 0) const;
		// process functions //
		DLLEXPORT static int ProcessDataDump(const wstring &data, vector<shared_ptr<NamedVar>> &vec, vector<IntWstring*>* specialintvalues = NULL);
		// operators //
		DLLEXPORT NamedVar& operator=(const NamedVar &other);

		//************************************
		// Method:    SwitchValues
		// FullName:  Leviathan::NamedVar::SwitchValues
		// Access:    public static 
		// Returns:   DLLEXPORT  void
		// Qualifier:
		// Parameter: NamedVar & receiver
		// Parameter: NamedVar & donator
		// Usage: Efficient way to copy new values to existing NamedVar, WARNING Will demolish the value that's values are copied
		//************************************
		DLLEXPORT static void SwitchValues(NamedVar &receiver, NamedVar &donator);

	private:

		bool Isint;

		int iValue;
		wstring *wValue;

		wstring Name;

		// friends //
		friend NamedVars;
	};



	class NamedVars{
	public:
		DLLEXPORT NamedVars();
		DLLEXPORT NamedVars(const NamedVars &other);
		DLLEXPORT NamedVars(const wstring &datadump);
		DLLEXPORT ~NamedVars();
		// ------------------------------------ //
		DLLEXPORT bool SetValue(const wstring &name, int val);
		DLLEXPORT bool SetValue(const wstring &name, wstring& val);
		DLLEXPORT bool SetValue(NamedVar &nameandvalues);

		DLLEXPORT int GetValue(const wstring &name, int& val1, wstring& val2) const;
		DLLEXPORT int GetValue(const wstring &name, int& val1) const;
		DLLEXPORT int GetValue(const wstring &name, wstring& val) const;

		// warning should be used with caution //
		DLLEXPORT wstring* ReturnValue(const wstring &name);
		// ------------------------------------ //
		DLLEXPORT bool IsIntValue(const wstring &name) const;
		DLLEXPORT bool IsIntValue(unsigned int index) const;

		DLLEXPORT wstring& GetName(unsigned int index);
		DLLEXPORT bool GetName(unsigned int index, wstring &name) const;

		DLLEXPORT void SetName(unsigned int index, const wstring &name);
		DLLEXPORT void SetName(const wstring &oldname, const wstring &name);
		DLLEXPORT bool CompareName(unsigned int index, const wstring &name) const;

		// ------------------------------------ //
		DLLEXPORT void AddVar(const wstring &name, int val, const wstring &wval, bool isint);
		DLLEXPORT void AddVar(shared_ptr<NamedVar> values);
		DLLEXPORT void Remove(unsigned int index);
		DLLEXPORT void Remove(const wstring &name);
		// ------------------------------------ //
		DLLEXPORT int LoadVarsFromFile(const wstring &file);

		DLLEXPORT vector<shared_ptr<NamedVar>>* GetVec();
		DLLEXPORT void SetVec(vector<shared_ptr<NamedVar>> &vec);

		// ------------------------------------ //
		DLLEXPORT int Find(const wstring &name) const;

	private:
		vector<shared_ptr<NamedVar>> Variables;
	};

}

#endif