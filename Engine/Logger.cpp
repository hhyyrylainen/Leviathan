// ------------------------------------ //
#include "Logger.h"

#include "Define.h"
#include "Common/ThreadSafe.h"
#include "Exceptions.h"
#include "FileSystem.h"
#include "Utility/Convert.h"
#include <chrono>
#include <fstream>
#include <ctime>
#include <iomanip>
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::Logger::Logger(const std::string &file):
    Path(file)
{
	// Get time for putting to the  beginning of the  log file //
    auto curtime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    std::stringstream formatedtime;

    //formatedtime << std::put_time(&curtime, "%S:%M:%H %A %d.%m.%Y (%Z)");
    formatedtime << "waiting for GCC 5";
    
	string write = "Start of Leviathan log for leviathan version: " + VERSIONS;

    write += "\nWriting to file \""+file+"\"";
    write += "\n------------------------TIME: "+formatedtime.str()+"------------------------\n";

    std::ofstream writer(Path);

    if(!writer.is_open()){

        throw Exception("Cannot open log file");
    }

    writer << write;
    
    writer.close();
    
    PendingLog = "";
	LatestLogger = this;
}

//! \brief Lock when using the logger singleton
static Mutex LoggerWriteMutex;

Leviathan::Logger::~Logger(){
	// Save if unsaved //
    Lock lock(LoggerWriteMutex);
    
	_Save();
    
	// Reset latest logger (this allows to create new logger,
    // which is quite bad, but won't crash
	LatestLogger = NULL;
}

Logger* Leviathan::Logger::LatestLogger = NULL;
// ------------------------------------ //
DLLEXPORT void Leviathan::Logger::Write(const std::string &data){

    const auto message = data+"\n";

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




