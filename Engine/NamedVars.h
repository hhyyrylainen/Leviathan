#ifndef LEVIATHAN_NAMEDVARS
#define LEVIATHAN_NAMEDVARS
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#ifndef LEVIATHAN_MISC
#include "Misc.h"
#endif
#ifndef LEVIATHAN_LOGGER
#include "Logger.h"
#endif
namespace Leviathan{

	class NamedVar{
	public:
		DLLEXPORT NamedVar();
		DLLEXPORT NamedVar(const NamedVar &other);
		DLLEXPORT NamedVar(const wstring &name, int val);
		DLLEXPORT NamedVar(const wstring &name, const wstring &val);
		DLLEXPORT NamedVar(const wstring &line);
		DLLEXPORT ~NamedVar();
		// ------------------------------------ //
		DLLEXPORT void SetValue(int val);
		DLLEXPORT void SetValue(const wstring& val);
		DLLEXPORT int GetValue(int& val1, wstring& val2) const;

		DLLEXPORT bool IsIntValue() const;

		DLLEXPORT wstring& GetName();
		DLLEXPORT void SetName(const wstring& name);
		DLLEXPORT bool CompareName(const wstring& name) const;
		// ------------------------------------ //
		DLLEXPORT wstring ToText(int WhichSeparator = 0) const;
		// process functions //
		DLLEXPORT static int ProcessDataDumb(wstring& data, vector<shared_ptr<NamedVar>>& vec, vector<IntWstring*>* specialintvalues = NULL);

	private:
		bool Isint;
		int iValue;
		wstring *wValue;

		wstring Name;
	};

	class NamedVars{
	public:
		DLLEXPORT NamedVars();
		DLLEXPORT NamedVars(const NamedVars& other);
		DLLEXPORT ~NamedVars();
		// ------------------------------------ //
		DLLEXPORT void SetValue(const wstring &name, int val);
		DLLEXPORT void SetValue(const wstring &name, wstring& val);
		DLLEXPORT int GetValue(const wstring &name, int& val1, wstring& val2) const;
		DLLEXPORT int GetValue(const wstring &name, int& val1) const;

		DLLEXPORT bool IsIntValue(const wstring &name) const;
		DLLEXPORT bool IsIntValue(unsigned int index);

		DLLEXPORT wstring& GetName(unsigned int index) const;
		DLLEXPORT void SetName(unsigned int index, const wstring &name);
		DLLEXPORT void SetName(const wstring &oldname, const wstring &name);
		DLLEXPORT bool CompareName(unsigned int index, const wstring &name) const;
		DLLEXPORT bool CompareName(const wstring &name1, const wstring &name2) const;

		DLLEXPORT void AddVar(const wstring &name, int val, const wstring &wval, bool isint);
		DLLEXPORT void Remove(unsigned int index);

		// ------------------------------------ //

		DLLEXPORT int LoadVarsFromFile(const wstring &file);

		DLLEXPORT vector<shared_ptr<NamedVar>>* GetVec();
		DLLEXPORT void SetVec(vector<shared_ptr<NamedVar>>& vec);

		// ------- //
		DLLEXPORT int Find(const wstring &name) const;

	private:
		vector<shared_ptr<NamedVar>> variables;
	};

}
#endif