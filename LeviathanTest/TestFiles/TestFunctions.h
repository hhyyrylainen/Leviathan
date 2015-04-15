
bool TestStringIterator(const int &tests){
	bool Failed = false;

	// test each one of WstringIterator's get functions and verify that they work correctly //

	itr.ReInit(L" o object type");

	results = itr.GetNextCharacterSequence<wstring>(UNNORMALCHARACTER_TYPE_WHITESPACE | UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS);

	if(!results || *results != L"o"){
		TESTFAIL;
	}

	itr.ReInit(L"get-this nice_prefix[but not this!");

	results = itr.GetNextCharacterSequence<wstring>(UNNORMALCHARACTER_TYPE_WHITESPACE | UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS);

	if(!results || *results != L"get-this"){
		TESTFAIL;
	}

	results = itr.GetNextCharacterSequence<wstring>(UNNORMALCHARACTER_TYPE_WHITESPACE | UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS);

	if(!results || *results != L"nice_prefix"){
		TESTFAIL;
	}


	itr.ReInit(L"aib val: = 243.12al toi() a 2456,12.5");

	results = itr.GetNextNumber<wstring>(DECIMALSEPARATORTYPE_DOT);
	if(!results || *results != L"243.12"){
		TESTFAIL;
	}

	results = itr.GetNextNumber<wstring>(DECIMALSEPARATORTYPE_DOT);
	if(!results || *results != L"2456"){
		TESTFAIL;
	}

	results = itr.GetNextNumber<wstring>(DECIMALSEPARATORTYPE_DOT);
	if(!results || *results != L"12.5"){
		TESTFAIL;
	}

	itr.ReInit(L"	aib val: = 243.12al toi() a 2456,12.5");


	results = itr.GetUntilEqualityAssignment<wstring>(EQUALITYCHARACTER_TYPE_EQUALITY);
	if(!results || *results != L"aib val:"){
		TESTFAIL;
	}

	itr.ReInit(L"StartCount = [[245]];");

	results = itr.GetUntilEqualityAssignment<wstring>(EQUALITYCHARACTER_TYPE_EQUALITY);
	if(!results || *results != L"StartCount"){
		TESTFAIL;
	}
	itr.SkipWhiteSpace();

	results = itr.GetUntilNextCharacterOrAll<wstring>(L';');
	if(!results || *results != L"[[245]]"){
		TESTFAIL;
	}

	itr.ReInit(L" adis told as\\; this still ; and no this");

	results = itr.GetUntilNextCharacterOrNothing<wstring>(L';');
	if(!results || *results != L" adis told as\\; this still "){
		TESTFAIL;
	}

	itr.ReInit(L"not][ this<out>");

	results = itr.GetNextCharacterSequence<wstring>(UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS);
	if(!results || *results != L"not"){
		TESTFAIL;
	}

	results = itr.GetNextCharacterSequence<wstring>(UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS);
	if(!results || *results != L" this"){
		TESTFAIL;
	}

	// some specific cases //
	itr.ReInit(L"\"JellyCube\";");
	results = itr.GetUntilNextCharacterOrAll<wstring>(L';');
	if(!results || *results != L"\"JellyCube\""){
		TESTFAIL;
	}


	// Comment handling test //
	itr.ReInit("asdf // This is a comment! //\n 25.44 /*2 .*/\n12\n\n// 1\n  a /*42.1*/12");

	auto sresults = itr.GetNextNumber<string>(DECIMALSEPARATORTYPE_DOT, SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	if(!sresults || *sresults != "25.44"){
		TESTFAIL;
	}

	sresults = itr.GetNextNumber<string>(DECIMALSEPARATORTYPE_DOT, SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	if(!sresults || *sresults != "12"){
		TESTFAIL;
	}

	sresults = itr.GetNextNumber<string>(DECIMALSEPARATORTYPE_DOT, SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

	if(!sresults || *sresults != "12"){
		TESTFAIL;
	}

	// End line testing //
	itr.ReInit(L"Don\\'t get anything from here\n42, but here it is1\n4 get until this\n and not this[as\n;] \"how it cu\nts\"");

	results = itr.GetNextNumber<wstring>(DECIMALSEPARATORTYPE_NONE, SPECIAL_ITERATOR_ONNEWLINE_STOP);

	if(results){
		TESTFAIL;
	}


	results = itr.GetUntilNextCharacterOrNothing<wstring>(',');

	if(!results){
		TESTFAIL;
	}

	results = itr.GetNextNumber<wstring>(DECIMALSEPARATORTYPE_NONE, SPECIAL_ITERATOR_ONNEWLINE_STOP);

	if(!results || *results != L"1"){
		TESTFAIL;
	}

	results = itr.GetNextCharacterSequence<wstring>(UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS, SPECIAL_ITERATOR_ONNEWLINE_STOP);

	if(!results || *results != L"4 get until this"){
		TESTFAIL;
	}

	results = itr.GetUntilNextCharacterOrNothing<wstring>('[');

	results = itr.GetUntilNextCharacterOrNothing<wstring>(';', SPECIAL_ITERATOR_ONNEWLINE_STOP);

	if(results){
		TESTFAIL;
	}

	results = itr.GetStringInQuotes<wstring>(QUOTETYPE_BOTH, SPECIAL_ITERATOR_ONNEWLINE_STOP);

	if(!results || *results != L"how it cu"){
		TESTFAIL;
	}

	itr.ReInit(L"oh=2;");

	results = itr.GetUntilEqualityAssignment<wstring>(EQUALITYCHARACTER_TYPE_ALL);

	if(!results || *results != L"oh"){

		TESTFAIL;
	}

	itr.SkipWhiteSpace(SPECIAL_ITERATOR_FILEHANDLING);


	results = itr.GetUntilNextCharacterOrNothing<wstring>(';');

	if(!results || *results != L"2"){

		TESTFAIL;
	}

	// Getting until line end //
	itr.ReInit(L"This is my line \\\r that has some things\r\n that are cut off");

	results = itr.GetUntilLineEnd<wstring>();

	if(!results || *results != L"This is my line \\\r that has some things"){
		TESTFAIL;
	}

	itr.ReInit(L"This is another line\nwith the right separator\nlast thing...");

	results = itr.GetUntilLineEnd<wstring>();

	if(!results || *results != L"This is another line"){
		TESTFAIL;
	}

	results = itr.GetUntilLineEnd<wstring>();
	
	if(!results || *results != L"with the right separator"){
		TESTFAIL;
	}

	results = itr.GetUntilLineEnd<wstring>();

	if(!results || *results != L"last thing..."){
		TESTFAIL;
	}

	// Test until string handling //
	itr.ReInit(L"Get until ('bird') the word bird to remove junkwords");

	results = itr.GetUntilCharacterSequence<wstring>(L"bird");

	if(!results || *results != L"Get until ('bird') the word "){
		TESTFAIL;
	}

	itr.MoveToNext();

	results = itr.GetUntilCharacterSequence<wstring>(L"word");

	if(!results || *results != L" to remove junk"){
		TESTFAIL;
	}

	return Failed;
}

