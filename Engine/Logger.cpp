#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_LOGGER
#include "Logger.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "FileSystem.h"

Logger* Leviathan::Logger::LatestLogger = NULL;
std::wstring Leviathan::Logger::QueuedLog = L"";

Leviathan::Logger::Logger(){
	log = L"";
	PendingLog = L"";
	FirstSaveDone = false;
	SYSTEMTIME tdate;
	GetLocalTime(&tdate);

	wstring times = Convert::IntToWstring(tdate.wDay)+L"."+Convert::IntToWstring(tdate.wMonth)+L"."+Convert::IntToWstring(tdate.wYear)+L" "+Convert::IntToWstring(tdate.wHour)+L":"+Convert::IntToWstring(tdate.wMinute);

	log = L"Start of Leviathan log. leviathan version :"+Convert::FloatToWstring(VERSION)+L"\n------------------------TIME: "+times+L"----------------------\n";

	Autosave = true;
	Saved = true;

	LatestLogger = this;
	Path = L".\\Log.txt";

	_inuse = false;
}
Leviathan::Logger::Logger(const wstring &start, bool autosave){
	log = L"";
	PendingLog = L"";
	FirstSaveDone = false;
	SYSTEMTIME tdate;
	GetLocalTime(&tdate);

	wstring times = Convert::IntToWstring(tdate.wDay)+L"."+Convert::IntToWstring(tdate.wMonth)+L"."+Convert::IntToWstring(tdate.wYear)+L" "+Convert::IntToWstring(tdate.wHour)+L":"+Convert::IntToWstring(tdate.wMinute);
	log += start;
	log += L"Start of Leviathan log. leviathan version :"+Convert::FloatToWstring(VERSION)+L"\n------------------------TIME: "+times+L"----------------------\n";

	Autosave = autosave;
	Saved = true;

	LatestLogger = this;
	Path = L".\\Log.txt";

	_inuse = false;
}
Leviathan::Logger::~Logger(){
	// check is something in queue //
	CheckQueue();
	// save if unsaved //
	if(!Saved)
		Save();
}

Logger* Leviathan::Logger::GetIfExists(){
	if(LatestLogger){
		return LatestLogger;
	}
	return NULL;
}

Logger* Leviathan::Logger::Get(){
	if(LatestLogger){
		return LatestLogger;
	}
	// create emergency logger //
	LatestLogger = new Logger(L" WARNING: EMERGENCY LOGGER CREATED! log called before engine init ",true);
	return LatestLogger;

}
void Leviathan::Logger::Write(const wstring &data){
	WaitToFinish();
	_inuse = true;

	// create message string //
	wstring message = data + L"\n";

	// if debug build send it to debug output //
	DEBUG_OUTPUT(message);

	log += message;
	PendingLog += message;
	// check is something in queue //
	CheckQueue();
	Saved = false;
	_inuse = false;
}
void Leviathan::Logger::Info(const wstring &data){
	WaitToFinish();
	_inuse = true;

	// create message string //
	wstring message = L"[INFO] "+data + L"\n";

	// if debug build send it to debug output //
	DEBUG_OUTPUT(message);

	log += message;
	PendingLog += message;
	// check is something in queue //
	CheckQueue();
	Saved = false;
	if(Autosave)
		Save();
	_inuse = false;
}
void Leviathan::Logger::Error(const wstring &data, int value){
	WaitToFinish();
	_inuse = true;

	// create message string //
	wstring message = L"[ERROR] "+data + L" value: "+Convert::IntToWstring(value)+L"\n";

	// if debug build send it to debug output //
	DEBUG_OUTPUT(message);

	log += message;
	PendingLog += message;
	// check is something in queue //
	CheckQueue();
	Saved = false;
	if(Autosave)
		Save();
	_inuse = false;

	// if specified throw exception //
#ifdef THROW_ON_PRINTERROR
	throw exception("error_message", 13);
#endif // THROW_ON_PRINTERROR
}
void Leviathan::Logger::Error(const wstring &data){
	WaitToFinish();
	_inuse = true;

	// create message string //
	wstring message = L"[ERROR] "+data+L"\n";

	// if debug build send it to debug output //
	DEBUG_OUTPUT(message);

	log += message;
	PendingLog += message;
	// check is something in queue //
	CheckQueue();
	Saved = false;
	if(Autosave)
		Save();
	_inuse = false;

	// if specified throw exception //
#ifdef THROW_ON_PRINTERROR
	throw exception("error_message", 13);
#endif // THROW_ON_PRINTERROR
}
void Leviathan::Logger::Write(const wstring &data, bool save){
	WaitToFinish();
	_inuse = true;

	// create message string //
	wstring message = data + L"\n";

	// if debug build send it to debug output //
	DEBUG_OUTPUT(message);

	log += message;
	PendingLog += message;
	// check is something in queue //
	CheckQueue();
	Saved = false;
	if(save)
		this->Save();
	_inuse = false;
}
void Leviathan::Logger::Info(const wstring &data, bool save){
	WaitToFinish();
	_inuse = true;

	// create message string //
	wstring message = L"[INFO] "+data + L"\n";

	// if debug build send it to debug output //
	DEBUG_OUTPUT(message);

	log += message;
	PendingLog += message;
	// check is something in queue //
	CheckQueue();
	Saved = false;
	if(save)
		this->Save();
	_inuse = false;
}
void Leviathan::Logger::Error(const wstring &data, int value, bool save){
	WaitToFinish();
	_inuse = true;

	// create message string //
	wstring message = L"[ERROR] "+data + L" value: "+Convert::IntToWstring(value)+L"\n";

	// if debug build send it to debug output //
	DEBUG_OUTPUT(message);

	log += message;
	PendingLog += message;
	// check is something in queue //
	CheckQueue();

	Saved = false;
	if(save)
		this->Save();
	_inuse = false;

	// if specified throw exception //
#ifdef THROW_ON_PRINTERROR
	throw exception("error_message", 13);
#endif // THROW_ON_PRINTERROR
}

DLLEXPORT void Leviathan::Logger::Warning(const wstring &data, bool save /*= false*/){
	WaitToFinish();
	_inuse = true;

	// create message string //
	wstring message = L"[WARNING] "+data+L"\n";

	// if debug build send it to debug output //
	DEBUG_OUTPUT(message);

	log += message;
	PendingLog += message;
	// check is something in queue //
	CheckQueue();

	Saved = false;
	if(save)
		this->Save();
	_inuse = false;
}

void Leviathan::Logger::Save(){
	if(!Saved){
		if(!FirstSaveDone){
			FileSystem::WriteToFile(log, Path);
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
	Path = path;
}

void Leviathan::Logger::WaitToFinish(){
	int count = 0;
	while(_inuse){
		Sleep(1);
		count++;
		if(count > 1000){
			
			log += L"[ERROR] logger waiting too long!\n";
			PendingLog += L"[ERROR] logger waiting too long!\n";
			Saved = false;
			break;
		}
	}

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

DLLEXPORT void Leviathan::Logger::QueueErrorMessage(const wstring& str){
	Logger* tmp = GetIfExists();
	if(tmp == NULL){
		// add to queue //
		QueuedLog += L"[ERROR] [DELAYED]"+str+L"\n";
		return;
	}
	// send to logger //
	tmp->Error(str, false);
}

void Leviathan::Logger::CheckQueue(){
	if(LatestLogger != NULL){
		// send it to error log //
		// just write to avoid issues 
		//LatestLogger->Write(QueuedLog, true);
		LatestLogger->log += QueuedLog;
		LatestLogger->PendingLog += QueuedLog;
		// clear //
		QueuedLog.clear();
	}
}
