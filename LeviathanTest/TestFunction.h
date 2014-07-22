#include "TestFunctions.h"
#include "Common/StringOperations.h"
#define FUNCTIONRUNS_BASE	1
//#define FUNCTIONRUNS_BASE	10000

bool TestPreEngine(){
	bool Failed = false;

	int tests = FUNCTIONRUNS_BASE;
	wstring timername = L"";


	// -------------------- Core parameter checking -------------------- //
	// --------- Float casting test --------- //
	if(TestFloatsCasts()){
		Logger::Get()->Error(L"Invalid casting on FloatX classes");
		Failed = true;
	}

	// --------- Float casting test --------- //
	timername = L"StringOperations tests running "+Convert::IntToWstring(tests);

	if(StringOperations::PerformTesting(tests)){
		Logger::Get()->Error(L"Invalid string operations");
		Failed = true;
	}

	TimingMonitor::StopTiming(timername);
	// -------------------- MultiFlag testing -------------------- //
	timername = L"MultiFlag tests running "+Convert::IntToWstring(tests);

	TimingMonitor::StartTiming(timername);

	if(TestMultiFlag(tests)){
		Logger::Get()->Error(L"Test did NOT succeed, test:");
		Failed = true;
	}

	TimingMonitor::StopTiming(timername);

	// --------- WstringIterator Function --------- //
	timername = L"TestWstringIterator running "+Convert::IntToWstring(tests);

	TimingMonitor::StartTiming(timername);
	if(TestStringIterator(tests)){
		Logger::Get()->Error(L"Test did NOT succeed, test:");
		Failed = true;
	}

	TimingMonitor::StopTiming(timername);
	// --------- Tokenizer Function --------- //
	timername = L"Linetokenizer test";

	TimingMonitor::StartTiming(timername);

	if(LineTokenizerTest(tests)){
		Logger::Get()->Error(L"Test did NOT succeed, test:");
		Failed = true;
	}

	TimingMonitor::StopTiming(timername);
	// --------- DataBlock testing --------- //
	timername = L"DataBlock test running "+Convert::IntToWstring(tests);

	TimingMonitor::StartTiming(timername);
	if(DataBlockTestVerifier(tests)){
		Logger::Get()->Error(L"Test did NOT succeed, test:");
		Failed = true;
	}

	TimingMonitor::StopTiming(timername);

	// -------------------- NamedVars testing -------------------- //
	timername = L"NamedVars tests running "+Convert::IntToWstring(tests);

	TimingMonitor::StartTiming(timername);

	if(TestNamedVars(tests)){
		Logger::Get()->Error(L"Test did NOT succeed, test:");
		Failed = true;
	}

	TimingMonitor::StopTiming(timername);


	// -------------------- TestFileReaderSimple testing -------------------- //
	//timername = L"TestFileReaderSimple tests running "+Convert::IntToWstring(tests);

	//TimingMonitor::StartTiming(timername);

	//if(TestFileReaderSimple(tests)){
	//	Logger::Get()->Error(L"Test did NOT succeed, test:");
	//	Failed = true;
	//}

	//TimingMonitor::StopTiming(timername);

	// -------------------- MISC Functions testing -------------------- //
	// --------- Replace Function --------- //
	timername = L"CutWstring test running "+Convert::IntToWstring(tests);

	TimingMonitor::StartTiming(timername);
	if(TestMiscCutWstring(tests)){
		Logger::Get()->Error(L"Test did NOT succeed, test:");
		Failed = true;
	}

	TimingMonitor::StopTiming(timername);
	// --------- Replace Function --------- //
	timername = L"Replace test running "+Convert::IntToWstring(tests);

	TimingMonitor::StartTiming(timername);

	if(TestMiscReplace(tests)){
		Logger::Get()->Error(L"Test did NOT succeed, test:");
		Failed = true;
	}

	TimingMonitor::StopTiming(timername);
	// --------- TestMiscWstringRemovePreceedingTrailingSpaces Function --------- //
	timername = L"TestMiscWstringRemovePreceedingTrailingSpaces test running "+Convert::IntToWstring(tests);

	TimingMonitor::StartTiming(timername);

	if(TestMiscWstringRemovePreceedingTrailingSpaces(tests)){
		Logger::Get()->Error(L"Test did NOT succeed, test:");
		Failed = true;
	}

	TimingMonitor::StopTiming(timername);

	// -------------------- MD5Testing testing -------------------- //
	timername = L"MD5Testing tests running "+Convert::IntToWstring(tests);

	TimingMonitor::StartTiming(timername);

	if(MD5Testing(tests)){
		Logger::Get()->Error(L"Test did NOT succeed, test:");
		Failed = true;
	}

	TimingMonitor::StopTiming(timername);

	return Failed;
}

bool TestEngine(Leviathan::Engine* engine){
	bool Failed = false;
	int tests = FUNCTIONRUNS_BASE;
	wstring timername = L"";

	// -------------------- Tasks Functions testing -------------------- //
	timername = L"QueuedTasks tests running "+Convert::IntToWstring(tests);

	TimingMonitor::StartTiming(timername);

	if(TestTaskTiming(tests, engine)){
		Logger::Get()->Error(L"Test did NOT succeed, test:");
		Failed = true;
	}

	TimingMonitor::StopTiming(timername);
	// -------------------- Autoupdateable Functions testing -------------------- //
	timername = L"Autoupdateable tests running "+Convert::IntToWstring(tests);

	TimingMonitor::StartTiming(timername);

	if(TestAutoUpdateableFunctions(tests, engine)){
		Logger::Get()->Error(L"Test did NOT succeed, test:");
		Failed = true;
	}

	TimingMonitor::StopTiming(timername);
	//// -------------------- TextRenderer Functions testing -------------------- //
	//timername = L"TextRenderer tests running "+Convert::IntToWstring(tests);

	//TimingMonitor::StartTiming(timername);

	//if(TestTextRenderer(tests, engine)){
	//	Logger::Get()->Error(L"Test did NOT succeed, test:");
	//	Failed = true;
	//}

	//TimingMonitor::StopTiming(timername);
	// -------------------- Events Functions testing -------------------- //
	timername = L"Eventsfunctions tests running "+Convert::IntToWstring(tests);

	TimingMonitor::StartTiming(timername);

	if(TestEventsFunctions(tests, engine)){
		Logger::Get()->Error(L"Test did NOT succeed, test:");
		Failed = true;
	}

	TimingMonitor::StopTiming(timername);
	// -------------------- IDFactory testing -------------------- //
	timername = L"IDFactory tests running "+Convert::IntToWstring(tests);

	TimingMonitor::StartTiming(timername);

	if(TestIDFactory(tests)){
		Logger::Get()->Error(L"Test did NOT succeed, test:");
		Failed = true;
	}

	TimingMonitor::StopTiming(timername);
	// -------------------- Script testing -------------------- //
	timername = L"Script tests running "+Convert::IntToWstring(tests);

	TimingMonitor::StartTiming(timername);

	if(TestScripting(tests, engine)){
		Logger::Get()->Error(L"Test did NOT succeed, test:");
		Failed = true;
	}

	TimingMonitor::StopTiming(timername);

	// -------------------- RandomTesting testing -------------------- //
	timername = L"RandomTesting tests running "+Convert::IntToWstring(tests);

	TimingMonitor::StartTiming(timername);

	RandomTesting(tests);


	TimingMonitor::StopTiming(timername);


	// -------------------- Object files testing -------------------- //
	Logger::Get()->Write(L"\n", false);
	timername = L"Object files main tests running "+Convert::IntToWstring(tests);

	TimingMonitor::StartTiming(timername);
	// --------- Object file parser Function --------- //
	wstring level2timer = L"Object files: ObjectFileParser test";

	TimingMonitor::StartTiming(level2timer);

	if(ObjectFileParserTest(tests/13)){
		Logger::Get()->Error(L"Test did NOT succeed, test:");
		Failed = true;
	}

	TimingMonitor::StopTiming(level2timer);

	Logger::Get()->Write(L"\n", false);
	TimingMonitor::StopTiming(timername);
	Logger::Get()->Write(L"\n", false);



	// tests passed //
	return Failed;
}



