#pragma once
// ------------------------------------ //
#include "Include.h"
#include <exception>
#include "ErrorReporter.h"

namespace Leviathan{

//! \brief Base class for all exceptions thrown by Leviathan
class Exception : public std::exception{
public:
    //! \param description Description of the error in utf8 encoding
    DLLEXPORT Exception(const char* description) noexcept;
    DLLEXPORT Exception(const std::string &description) noexcept;
    DLLEXPORT Exception(const Exception &e) noexcept;

    ~Exception() = default;

    DLLEXPORT Exception& operator=(const Exception &other) noexcept;

    DLLEXPORT const char* what() const noexcept override;

#ifndef LEVIATHAN_UE_PLUGIN
    DLLEXPORT virtual void PrintToLog() const noexcept;
#endif

    DLLEXPORT virtual void Print(LErrorReporter* errorreport) const noexcept;
        
protected:
        
    std::string Message;
};
    
    
class InvalidAccess : public Exception{
public:
    DLLEXPORT InvalidAccess(const char* description) noexcept;
    DLLEXPORT InvalidAccess(const std::string &description) noexcept;
};

class InvalidArgument : public Exception{
public:
    DLLEXPORT InvalidArgument(const char* description) noexcept;
    DLLEXPORT InvalidArgument(const std::string &description) noexcept;
};

class InvalidState : public Exception{
public:
    DLLEXPORT InvalidState(const char* description) noexcept;
    DLLEXPORT InvalidState(const std::string &description) noexcept;
};

class InvalidType : public Exception{
public:
    DLLEXPORT InvalidType(const char* description) noexcept;
    DLLEXPORT InvalidType(const std::string &description) noexcept;
};

class NotFound : public Exception{
public:
    DLLEXPORT NotFound(const char* description) noexcept;
    DLLEXPORT NotFound(const std::string &description) noexcept;
};

class NULLPtr : public Exception{
public:
    DLLEXPORT NULLPtr(const char* description) noexcept;
    DLLEXPORT NULLPtr(const std::string &description) noexcept;
};
}

#ifdef LEAK_INTO_GLOBAL
using Leviathan::Exception;
using Leviathan::InvalidArgument;
using Leviathan::InvalidType;
using Leviathan::InvalidState;
using Leviathan::InvalidAccess;
#endif
