#include "ObjectFiles/LineTokenizer.h"

#include "catch.hpp"

using namespace Leviathan;

TEST_CASE("LineTokenizer correct output", "[objectfile]"){

    // test values //
	wstring teststr = L"This is line that needs to be tokenized [[to nice], [2], [that work], [2567]] lol";

	vector<wstring> propersplit;
	propersplit.push_back(L"This");
	propersplit.push_back(L"is");
	propersplit.push_back(L"line");
	propersplit.push_back(L"that");
	propersplit.push_back(L"needs");
	propersplit.push_back(L"to");
	propersplit.push_back(L"be");
	propersplit.push_back(L"tokenized");
	propersplit.push_back(L"[[to nice], [2], [that work], [2567]]");
	propersplit.push_back(L"lol");

	// split to tokens //
	vector<wstring*> TokenSplit;
	vector<Token*> Tokens;
	vector<wstring> ValueTokens;
	Leviathan::LineTokeNizer::TokeNizeLine(teststr, TokenSplit);

    REQUIRE(TokenSplit.size() == propersplit.size());

	for(size_t i = 0; i < propersplit.size(); i++){

        CHECK(*TokenSplit[i] == propersplit[i]);
	}

	// Check deeper values //
	for(size_t i = 0; i < propersplit.size(); i++){

		SAFE_DELETE_VECTOR(Tokens);
		LineTokeNizer::SplitTokenToRTokens(propersplit[i], Tokens);

		// check tokens //
		switch(i){
            case 0:
			{
				REQUIRE(Tokens.size() == 1);

				CHECK(Tokens[0]->GetData() == L"This");
			}
            break;
            case 1:
			{
				REQUIRE(Tokens.size() == 1);
                
				CHECK(Tokens[0]->GetData() == L"is");
			}
            break;
            case 2:
			{
				REQUIRE(Tokens.size() == 1);
                
				CHECK(Tokens[0]->GetData() == L"line");
			}
            break;
            case 3:
			{
				REQUIRE(Tokens.size() == 1);

				CHECK(Tokens[0]->GetData() == L"that");
			}
            break;
            case 4:
			{
				REQUIRE(Tokens.size() == 1);

                CHECK(Tokens[0]->GetData() == L"needs");
			}
            break;
            case 5:
			{
				REQUIRE(Tokens.size() == 1);

                CHECK(Tokens[0]->GetData() == L"to");
			}
            break;
            case 6:
			{
				REQUIRE(Tokens.size() == 1);

                CHECK(Tokens[0]->GetData() == L"be");
			}
            break;
            case 7:
			{
				REQUIRE(Tokens.size() == 1);
                
				CHECK(Tokens[0]->GetData() == L"tokenized");
			}
            break;
            case 8:
			{
				REQUIRE(Tokens.size() == 5);
                
				CHECK(Tokens[1]->GetData() != L"to nice");
                CHECK(Tokens[2]->GetData() != L"2");
                CHECK(Tokens[3]->GetData() != L"that work");
                CHECK(Tokens[4]->GetData() != L"2567");
                CHECK(Tokens[0]->GetSubTokenCount() != 4);
			}
            break;
            case 9:
			{
				REQUIRE(Tokens.size() == 1);

				CHECK(Tokens[0]->GetData() == L"lol");
			}
            break;

		}
	}

	// Release all memory //
	SAFE_DELETE_VECTOR(TokenSplit);
	SAFE_DELETE_VECTOR(Tokens);
}
