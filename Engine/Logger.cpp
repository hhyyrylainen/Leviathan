#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_LOGGER
#include "Logger.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "FileSystem.h"

Leviathan::Logger::Logger(): FirstSaveDone(false), Saved(false), Autosave(false), Path(L"./Log.txt"){
	// get time for putting to beginning of log //
#ifdef _WIN32
	SYSTEMTIME tdate;
	GetLocalTime(&tdate);

	wstring times = Convert::IntToWstring(tdate.wDay)+L"."+Convert::IntToWstring(tdate.wMonth)+L"."+Convert::IntToWstring(tdate.wYear)+L" "
		+Convert::IntToWstring(tdate.wHour)+L":"+Convert::IntToWstring(tdate.wMinute);

#else
    wstring times = L"TODO: add time get";

#endif

	PendingLog = L"Start of Leviathan log for leviathan version :" VERSIONS L"\n------------------------TIME: "+times+L"----------------------\n";

	LatestLogger = this;
}
DLLEXPORT Leviathan::Logger::Logger(const wstring &start, const bool &autosave) : FirstSaveDone(false), Saved(false), Autosave(autosave),
	Path(L"./Log.txt")
{
#ifdef _WIN32
	SYSTEMTIME tdate;
	GetLocalTime(&tdate);

	wstring times = Convert::IntToWstring(tdate.wDay)+L"."+Convert::IntToWstring(tdate.wMonth)+L"."+Convert::IntToWstring(tdate.wYear)+L" "
		+Convert::IntToWstring(tdate.wHour)+L":"+Convert::IntToWstring(tdate.wMinute);

#else
    wstring times = L"TODO: add time get";

#endif

	PendingLog = start+L"Start of Leviathan log for leviathan version :" VERSIONS L"\n------------------------TIME: "+times+L"----------------------\n";

	LatestLogger = this;
}


Leviathan::Logger::~Logger(){
	// thread safety //
	boost::strict_lock<Logger> guard(*this);

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
	DEBUG_OUTPUT(message);

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
	DEBUG_OUTPUT(message);

	PendingLog += message;

	_LogUpdateEndPart(save, guard);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Logger::Error(const wstring &data, const int &pvalue /*= 0*/, const bool &save /*= false*/){
	// thread safety //
	boost::strict_lock<Logger> guard(*this);

	// create message string //
	wstring message = L"[ERROR] "+data+L"\n";

	// if debug build send it to debug output //
	DEBUG_OUTPUT(message);
	PendingLog += message;


	_LogUpdateEndPart(save, guard);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Logger::Warning(const wstring &data, bool save /*= false*/){
	// thread safety //
	boost::strict_lock<Logger> guard(*this);

	// create message string //
	wstring message = L"[WARNING] "+data+L"\n";

	// if debug build send it to debug output //
	DEBUG_OUTPUT(message);
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

void Leviathan::Logger::SendDebugMessage(const wstring& str){
#ifdef _DEBUG
	OutputDebugString(&*str.begin());
#else
	return;
#endif
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Logger::QueueErrorMessage(const wstring& str){
	Logger* tmp = GetIfExists();
	if(tmp == NULL){
		// add to queue //
		QueuedLog += L"[ERROR] [DELAYED]"+str+L"\n";
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
	LatestLogger = new Logger(L"(W) ", true);
	return LatestLogger;
}




