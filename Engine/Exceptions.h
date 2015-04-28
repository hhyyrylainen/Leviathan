#pragma once
#define LEVIATHAN_EXCEPTIONS
// ------------------------------------ //
#include "Include.h"
#include <exception>


namespace Leviathan{

    //! \brief Base class for all exceptions thrown by Leviathan
    class Exception : public std::exception{
    public:
        //! \param description Description of the error in utf8 encoding
        DLLEXPORT Exception(const char* description) noexcept;
        DLLEXPORT Exception(const Exception &e) noexcept;

        DLLEXPORT Exception& operator=(const Exception &other) noexcept;

        DLLEXPORT const char* what() const noexcept override;

        DLLEXPORT virtual void PrintToLog() const noexcept;
        
    protected:
        
        const char* Message;
    };
    
    
    class InvalidAccess : public Exception{
    public:
        DLLEXPORT InvalidAccess(const char* description) noexcept;
    };

    class InvalidArgument : public Exception{
    public:
        DLLEXPORT InvalidArgument(const char* description) noexcept;
    };

    class InvalidState : public Exception{
    public:
        DLLEXPORT InvalidState(const char* description) noexcept;
    };

    class InvalidType : public Exception{
    public:
        DLLEXPORT InvalidType(const char* description) noexcept;
    };

    class NotFound : public Exception{
    public:
        DLLEXPORT NotFound(const char* description) noexcept;
    };

    class NULLPtr : public Exception{
    public:
        DLLEXPORT NULLPtr(const char* description) noexcept;
    };
}
