// ------------------------------------ //
#include "Exceptions.h"
#include "Logger.h"
#include "Define.h"

using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::Exception::Exception(const char* description) noexcept : Message(description){

}

DLLEXPORT Leviathan::Exception::Exception(const std::string &description) noexcept :
    Message(description)
{
    
}

DLLEXPORT Leviathan::Exception::Exception(const Exception &e) noexcept : Message(e.Message){

}
// ------------------------------------ //
DLLEXPORT Exception& Leviathan::Exception::operator=(const Exception &other) noexcept{

    Message = other.Message;
    return *this;
}
// ------------------------------------ //
DLLEXPORT const char* Leviathan::Exception::what() const noexcept{

    return Message.c_str();
}
// ------------------------------------ //
#ifndef LEVIATHAN_UE_PLUGIN
DLLEXPORT void Leviathan::Exception::PrintToLog() const noexcept{

    Logger::Get()->Write(string("[EXCEPTION] ")+Message);
}
#endif

DLLEXPORT void Leviathan::Exception::Print(LErrorReporter* errorreport) const noexcept {
   
    LEVIATHAN_ASSERT(errorreport != nullptr, "erroreport in Exception::Print may not be null");
    errorreport->Write(string("[EXCEPTION] ") + Message);
}

// ------------------------------------ //
DLLEXPORT Leviathan::InvalidAccess::InvalidAccess(const char* description) noexcept : Exception(description){
}

DLLEXPORT Leviathan::InvalidAccess::InvalidAccess(const std::string &description) noexcept : Exception(description){
}

DLLEXPORT Leviathan::InvalidArgument::InvalidArgument(const char* description) noexcept : Exception(description){
}

DLLEXPORT Leviathan::InvalidArgument::InvalidArgument(const std::string &description) noexcept : Exception(description){
}

DLLEXPORT Leviathan::InvalidState::InvalidState(const char* description) noexcept : Exception(description){
}

DLLEXPORT Leviathan::InvalidState::InvalidState(const std::string &description) noexcept : Exception(description){
}

DLLEXPORT Leviathan::InvalidType::InvalidType(const char* description) noexcept : Exception(description){
}

DLLEXPORT Leviathan::InvalidType::InvalidType(const std::string &description) noexcept : Exception(description){
}

DLLEXPORT Leviathan::NotFound::NotFound(const char* description) noexcept : Exception(description){
}

DLLEXPORT Leviathan::NotFound::NotFound(const std::string &description) noexcept : Exception(description){
}

DLLEXPORT Leviathan::NULLPtr::NULLPtr(const char* description) noexcept : Exception(description){
}

DLLEXPORT Leviathan::NULLPtr::NULLPtr(const std::string &description) noexcept : Exception(description){
}



