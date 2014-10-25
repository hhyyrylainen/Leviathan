#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_LOGGER
#include "Logger.h"
#endif
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "FileSystem.h"
#include "Application/AppDefine.h"

DLLEXPORT Leviathan::Logger::Logger(const wstring &file):
    FirstSaveDone(false), Saved(false), Autosave(false), Path(file)
{
	// get time for putting to the  beginning of the  log file //
#ifdef _WIN32
	SYSTEMTIME tdate;
	GetLocalTime(&tdate);

	wstring times = Convert::IntToWstring(tdate.wDay)+L"."+Convert::IntToWstring(tdate.wMonth)+L"."+
        Convert::IntToWstring(tdate.wYear)+L" "
		+Convert::IntToWstring(tdate.wHour)+L":"+Convert::IntToWstring(tdate.wMinute);

#else
	// \todo Linux time get function
	wstring times = L"TODO: add time get";

#endif

	PendingLog = L"Start of Leviathan log for leviathan version :" + VERSIONS +
		L"\n------------------------TIME: "+times+L"----------------------\n";

	LatestLogger = this;
}
DLLEXPORT Leviathan::Logger::Logger(const wstring &file, const wstring &start, const bool &autosave) :
    FirstSaveDone(false), Saved(false), Autosave(autosave), Path(file)
{
#ifdef _WIN32
	SYSTEMTIME tdate;
	GetLocalTime(&tdate);

	wstring times = Convert::IntToWstring(tdate.wDay)+L"."+Convert::IntToWstring(tdate.wMonth)+L"."+
        Convert::IntToWstring(tdate.wYear)+L" "
		+Convert::IntToWstring(tdate.wHour)+L":"+Convert::IntToWstring(tdate.wMinute);

#else
	wstring times = L"TODO: add time get";

#endif

	PendingLog = start+L"Start of Leviathan log for leviathan version :" + VERSIONS +
		L"\n------------------------TIME: "+times+L"----------------------\n";

	LatestLogger = this;
}


Leviathan::Logger::~Logger(){
	// thread safety //
	boost::strict_lock<Logger> guard(*this);

	// Reset latest logger (this allows to create new logger, which is quite bad, but won't crash) //
	LatestLogger = NULL;

	// check is something in queue //
	CheckQueue(guard);
	// save if unsaved //
	Save(guard);
}

std::wstring Leviathan::Logger::QueuedLog = L"";

Logger* Leviathan::Logger::LatestLogger = NULL;
// ------------------------------------ //
DLLEXPORT void Leviathan::Logger::Write(const wstring &data, const bool &save /*= false*/){
	// thread safety //
	boost::strict_lock<Logger> guard(*this);

	// create message string //
	const wstring message = data + L"\n";

	// if debug build send it to debug output //
	SendDebugMessage(message, guard);

	PendingLog += message;

	_LogUpdateEndPart(save, guard);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Logger::Info(const wstring &data, const bool &save /*= false*/){
	// thread safety //
	boost::strict_lock<Logger> guard(*this);

	// create message string //
	wstring message = L"[INFO] "+data + L"\n";

	// if debug build send it to debug output //
	SendDebugMessage(message, guard);

	PendingLog += message;

	_LogUpdateEndPart(save, guard);
}

DLLEXPORT void Leviathan::Logger::Info(const string &data, const bool &save){

    boost::strict_lock<Logger> guard(*this);

    // TODO: change this to whole class to use utf8 strings...
    wstringstream sstream;

    sstream << L"[INFO] ";
    sstream << data.c_str();
    sstream << L"\n";

    SendDebugMessage(sstream.str(), guard);
    
    PendingLog += sstream.str();

    _LogUpdateEndPart(save, guard);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Logger::Error(const wstring &data, const int &pvalue /*= 0*/, const bool &save /*= false*/){
	// thread safety //
	boost::strict_lock<Logger> guard(*this);

	// create message string //
	wstring message = L"[ERROR] "+data+L"\n";

	// if debug build send it to debug output //
	SendDebugMessage(message, guard);
	PendingLog += message;


	_LogUpdateEndPart(save, guard);
}

DLLEXPORT void Leviathan::Logger::Error(const string &data, const int &pvalue /*= 0*/, const bool &save /*= false*/){
	// thread safety //
	boost::strict_lock<Logger> guard(*this);

    // TODO: change this to whole class to use utf8 strings...
    wstringstream sstream;

    sstream << L"[ERROR] ";
    sstream << data.c_str();
    sstream << L"\n";

    SendDebugMessage(sstream.str(), guard);
    PendingLog += sstream.str();


	_LogUpdateEndPart(save, guard);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Logger::Warning(const wstring &data, bool save /*= false*/){
	// thread safety //
	boost::strict_lock<Logger> guard(*this);

	// create message string //
	wstring message = L"[WARNING] "+data+L"\n";

	// if debug build send it to debug output //
	SendDebugMessage(message, guard);
	PendingLog += message;

	_LogUpdateEndPart(save, guard);
}
// ------------------------------------ //
void Leviathan::Logger::Save(boost::strict_lock<Logger> &guard){
	if(!Saved){
		if(!FirstSaveDone){
			FileSystem::WriteToFile(PendingLog, Path);
			PendingLog.clear();
			FirstSaveDone = true;

		} else {
			// append to file //
			FileSystem::AppendToFile(PendingLog, Path);
			PendingLog.clear();
		}
		Saved = true;
	}
}

DLLEXPORT void Leviathan::Logger::SetSavePath(const wstring& path){
	// thread safety //
	boost::strict_lock<Logger> guard(*this);

	Path = path;
}
// -------------------------------- //
void Leviathan::Logger::Print(string message, bool save){
	Get()->Write(Convert::StringToWstring(message), save);
}

void Leviathan::Logger::SendDebugMessage(const wstring& str, boost::strict_lock<Logger> &guard){
#ifdef _WIN32
	OutputDebugString(&*str.begin());
#endif // _WIN32
	// We also want standard output messages //
	// Using cout should be fine for most other platforms //
	cout << Convert::WstringToString(str);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Logger::QueueErrorMessage(const wstring& str){
	Logger* tmp = GetIfExists();
	if(tmp == NULL){
		// add to queue //
		QueuedLog += L"[ERROR][DELAYED] "+str+L"\n";
		return;
	}
	// send to logger //
	tmp->Error(str);
}

DLLEXPORT void Leviathan::Logger::DirectWriteBuffer(const wstring &data){
	// thread safety //
	boost::strict_lock<Logger> guard(*this);

	// directly just append to buffers //
	PendingLog += data;
	Saved = false;
}
// ------------------------------------ //
void Leviathan::Logger::_LogUpdateEndPart(const bool &save, boost::strict_lock<Logger> &guard){
	// check is something in queue //
	CheckQueue(guard);
	// unsaved //
	Saved = false;

	if(save)
		Save();
}
// ------------------------------------ //
DLLEXPORT Logger* Leviathan::Logger::GetIfExists(){
	return LatestLogger ? LatestLogger: NULL;
}

DLLEXPORT Logger* Leviathan::Logger::Get(){
	if(LatestLogger){
		return LatestLogger;
	}
	// create emergency logger //
	if(!AppDef::GetDefault()){
		// We need some dummy logger //
		return NULL;
	}

	wstring ourlogfile = AppDef::GetDefault()->GetLogFile();
	ourlogfile += L".txt";
	LatestLogger = new Logger(ourlogfile, L"(W) ", true);
	return LatestLogger;
}




