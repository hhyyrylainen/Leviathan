#pragma once
#include "Define.h"
// ------------------------------------ //
#include <string>

namespace Leviathan{

class CommandSender;
    
//! \brief Implement this interface to be able to handle custom commands
//! \see CommandHandler CommandHandler::RegisterCustomCommandHandler
//! \todo Allow scripts to register these
//! \todo Allow optional usage of an internal command table with descriptions etc.
class CustomCommandHandler{
public:
    DLLEXPORT CustomCommandHandler(){

    }
    DLLEXPORT virtual ~CustomCommandHandler(){

    }

    //! \brief Checks whether this handler can handle a command
    //! \param cmd Will be first word of the input without first preceding /,
    //! for example 'jump' or 'kickall'
    DLLEXPORT virtual bool CanHandleCommand(const std::string &cmd) const = 0;


    //! \brief Called by a worker thread that wants to execute a command
    //! \param wholecommand Will contain exactly what the user passed in including the
    //! preceding /'s
    //! \param sender The CommandSender who executed this command, the pointer is safe to use
    //! during this call as it is locked during the call
    //! \warning This function can easily deadlock the engine if not used with care, expensive
    //! operations (or dangerous ones) should be queued as a task to avoid deadlocking
    DLLEXPORT virtual void ExecuteCommand(const std::string &wholecommand,
        CommandSender* sender) = 0;
};

}

