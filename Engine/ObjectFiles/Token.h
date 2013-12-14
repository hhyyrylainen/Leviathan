#ifndef LEVIATHAN_TOKEN
#define LEVIATHAN_TOKEN
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{

	class Token : public Object{
	public:
		DLLEXPORT Token();
		DLLEXPORT Token(const wstring& data);
		DLLEXPORT ~Token();

		DLLEXPORT bool CreateSubToken(Token* toadd);
		DLLEXPORT Token* GetParentToken();

		DLLEXPORT int GetSubTokenCount();
		DLLEXPORT Token* GetSubToken(int slot);
		DLLEXPORT void ClearSubPointers(bool dodelete = true);

		DLLEXPORT const wstring& GetData() const;
		DLLEXPORT void SetData(const wstring &val);
		DLLEXPORT wstring& GetChangeableData();
	private:
		// flag to delete sub tokens on deletion //
		bool _IsCleaner;

		// real data //
		wstring Data;

		// sub tokens //
		vector<Token*> SubTokens;
		Token* parenttoken;
	};

}
#endif
