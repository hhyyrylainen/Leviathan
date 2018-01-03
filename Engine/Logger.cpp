#include "Include.h"
// ------------------------------------ //
#include "Logger.h"

#include "Define.h"
#include "Common/ThreadSafe.h"
#ifndef ALTERNATIVE_EXCEPTIONS_FATAL
#include "Exceptions.h"
#endif //ALTERNATIVE_EXCEPTIONS_FATAL
#include "FileSystem.h"

#ifdef _WIN32
#include "Utility/Convert.h"
#endif //_WIN32

#include <chrono>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <time.h>
#ifdef _WIN32
#include "WindowsInclude.h"
#endif //_WIN32

#include <boost/filesystem.hpp>

#include <stdlib.h>

using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Logger::Logger(const std::string &file):
    Path(file)
{

    // Get time for putting to the  beginning of the  log file //
    auto t = std::time(nullptr);

    std::stringstream formatedtime;
    
#ifdef _WIN32
    struct tm curtime;
    localtime_s(&curtime, &t);
    formatedtime << std::put_time(&curtime, "%H:%M:%S %A %d.%m.%Y (%Z)");
#else
    struct tm* curtime = localtime(&t);
    formatedtime << std::put_time(curtime, "%H:%M:%S %A %d.%m.%Y (%Z)");
#endif //_WIN32

    string write = "Start of Leviathan log for leviathan version: " + VERSIONS;

    write += "\nWriting to file \"" + file + "\"";
    write += "\n------------------------TIME: " + formatedtime.str() + "------------------------\n";

    // Create the target folder if it doesn't exist
    const auto path = boost::filesystem::path(file).parent_path();

    if(!path.empty()){
        try{
            boost::filesystem::create_directories(path);
        } catch(const std::exception &e){
        	
            SendDebugMessage("Failed to create folder for log file, exception: " + 
                std::string(e.what()));
        }
    }

    std::ofstream writer(Path);

    if (!writer.is_open()) {

    #ifndef ALTERNATIVE_EXCEPTIONS_FATAL
        throw Exception("Cannot open log file");
    #else
        SendDebugMessage("Cannot open log file, quitting");
        abort(6);
    #endif //ALTERNATIVE_EXCEPTIONS_FATAL
    }

    writer << write;

    writer.close();

    
    PendingLog = "";
    LatestLogger = this;
}

//! \brief Lock when using the logger singleton
static Mutex LoggerWriteMutex;

Logger::~Logger(){
    // Save if unsaved //
    Lock lock(LoggerWriteMutex);
    
    _Save();
    
    // Reset latest logger (this allows to create new logger,
    // which is quite bad, but won't crash
    // There is also probably a race condition here
    if(LatestLogger == this)
        LatestLogger = nullptr;
}

DLLEXPORT Logger* Logger::Get(){

    return LatestLogger;
}

Logger* Logger::LatestLogger = NULL;
// ------------------------------------ //
DLLEXPORT void Logger::Write(const std::string &data){

    const auto message = data+"\n";

    Lock lock(LoggerWriteMutex);

    SendDebugMessage(message);
    
    PendingLog += message;

    _LogUpdateEndPart();
}

DLLEXPORT void Logger::WriteRaw(const std::string &data){

    Lock lock(LoggerWriteMutex);

    SendDebugMessage(data);
    
    PendingLog += data;

    _LogUpdateEndPart();
}

void Logger::WriteLine(const std::string &Text) {

    Write(Text);
}

void Logger::Fatal(const std::string &data) {

    const auto message = "[FATAL] " + data + "\n";

    {
        Lock lock(LoggerWriteMutex);

        SendDebugMessage(message);

        PendingLog += message;

        _LogUpdateEndPart();

        // This might call Save for the second time, but better call
        // it twice than miss saving the log before aborting
        _Save();
    }

    // Exit process //
    abort();
}
// ------------------------------------ //
DLLEXPORT void Logger::Info(const std::string &data){
    
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
void Logger::Save(){

    Lock lock(LoggerWriteMutex);

    _Save();
}

void Logger::_Save(){

    if(PendingLog.empty())
        return;

    std::ofstream file(Path, std::ofstream::out | std::ofstream::app);

    file << PendingLog;
    file.close();
    
    PendingLog.clear();
}
// -------------------------------- //
void Logger::Print(const string &message){
    Get()->Write(message);
}

DLLEXPORT void Logger::SendDebugMessage(const string &str){
#ifdef _WIN32
    const wstring converted = Convert::Utf8ToUtf16(str);
    OutputDebugString(&*converted.begin());
#endif // _WIN32
    // We also want standard output messages //
    // Using cout should be fine for most other platforms //
    std::cout << str;
}
// ------------------------------------ //
DLLEXPORT void Logger::DirectWriteBuffer(const std::string &data){

    Lock guard(LoggerWriteMutex);

    PendingLog += data;
}
// ------------------------------------ //
DLLEXPORT std::string Logger::GetLogFile() const{

    return Path;
}
// ------------------------------------ //
void Logger::_LogUpdateEndPart(){

    _Save();
}
// ------------------------------------ //


