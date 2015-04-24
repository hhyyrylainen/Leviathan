// ------------------------------------ //
#include "Logger.h"
#include "Common/ThreadSafe.h"
#include "FileSystem.h"
#include <chrono>
#include "Exceptions.h"
#include <fstream>
#include "Utility/Convert.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::Logger::Logger(const std::string &file):
    Path(file)
{
	// Get time for putting to the  beginning of the  log file //
    auto curtime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    std::stringstream formatedtime;

    formatedtime << std::put_time(&curtime, "%S:%M:%H %A %d.%m.%Y (%Z)");
    
	string write = "Start of Leviathan log for leviathan version: " + VERSIONS;

    write += "\nWriting to file \""+file+L"\"";
    write += "\n------------------------TIME: "+formatedtime.str()+"------------------------\n";

    std::ofstream file(Path);

    if(!file.is_open()){

        throw Exception("Cannot open log file");
    }

    file << write;
    
    file.close();
    
    PendingLog = "";
	LatestLogger = this;
}

Leviathan::Logger::~Logger(){
	// Save if unsaved //
	Save(guard);
    
	// Reset latest logger (this allows to create new logger,
    // which is quite bad, but won't crash
	LatestLogger = NULL;
}

Logger* Leviathan::Logger::LatestLogger = NULL;

//! \brief Lock when using the logger singleton
static Mutex LoggerWriteMutex;
// ------------------------------------ //
DLLEXPORT void Leviathan::Logger::Write(const std::string &data){

    const auto message = data.c_str()+"\n";

    Lock lock(LoggerWriteMutex);

    SendDebugMessage(message);
    
    PendingLog += message;

    _LogUpdateEndPart();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Logger::Info(const std::string &data){
    
    const auto message = "[INFO] " + data + "\n";

    Lock lock(LoggerWriteMutex);

    SendDebugMessage(message);
    
    PendingLog += message;

    _LogUpdateEndPart();
}
// ------------------------------------ //
DLLEXPORT void Logger::Error(const std::string &data){

    const auto message = "[ERROR] " + data + "\n";

    Lock lock(LoggerWriteMutex);

    SendDebugMessage(message);
    
    PendingLog += message;

	_LogUpdateEndPart();
}
// ------------------------------------ //
DLLEXPORT void Logger::Warning(const std::string &data){

    const auto message = "[WARNING] " + data + "\n";

    Lock lock(LoggerWriteMutex);
    
    SendDebugMessage(message);
    
    PendingLog += message;

	_LogUpdateEndPart();
}
// ------------------------------------ //
void Leviathan::Logger::Save(){

    Lock lock(LoggerWriteMutex);

    _Save();
}

void Logger::_Save(){

    if(PendingLog.empty())
        return;

    std::ofstream file(Path);

    file << PendingLog;
    file.close();
    
    PendingLog.clear();
}
// -------------------------------- //
void Leviathan::Logger::Print(const string &message){
	Get()->Write(message);
}

DLLEXPORT void Leviathan::Logger::SendDebugMessage(const string &str){
#ifdef _WIN32
	const wstring converted = Convert::StringToWstring(str);
	OutputDebugString(&*converted.begin());
#endif // _WIN32
	// We also want standard output messages //
	// Using cout should be fine for most other platforms //
	cout << str;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Logger::DirectWriteBuffer(const std::string &data){

    Lock guard(LoggerWriteMutex);

	PendingLog += data;
}
// ------------------------------------ //
void Leviathan::Logger::_LogUpdateEndPart(){

    _Save();
}
// ------------------------------------ //
DLLEXPORT Logger* Leviathan::Logger::Get(){

    return LatestLogger;
}




