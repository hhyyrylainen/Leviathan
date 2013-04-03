#ifndef LEVIATHAN_LINETOKENIZER
#define LEVIATHAN_LINETOKENIZER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Token.h"

namespace Leviathan{
	class LineTokeNizer{
		struct DataForToken{
			DataForToken(Token* token, const Int2 &positions){
				ToToken = token;
				ToCopyCharacters = Int2(positions);
			};

			Token* ToToken;
			Int2 ToCopyCharacters;
		};
	public:
		DLLEXPORT static int TokeNizeLine(const wstring& str, vector<wstring*> &result);
		DLLEXPORT static int SplitTokenToValues(const wstring& str, vector<wstring> &result);

		DLLEXPORT static int SplitTokenToRTokens(const wstring& str, vector<Token*> &result);

	private:
		// private constructors //
		LineTokeNizer::LineTokeNizer();
		LineTokeNizer::~LineTokeNizer();

		// private functions //
		static bool TokenRTokenAddSubTokens(int curlevel, int maxlevel, Token* curtoken, vector<Token*>& destination);
		static void TokenRTokenRemoveAllSubTokensFromVec(Token* curtoken, vector<Token*>& destination);
	};

}
#endif