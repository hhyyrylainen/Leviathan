// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "CustomCommandHandler.h"

#include <list>
#include <memory>
#include <vector>


namespace Leviathan{

enum COMMANDSENDER_PERMISSIONMODE {
    COMMANDSENDER_PERMISSIONMODE_NORMAL,
    COMMANDSENDER_PERMISSIONMODE_IGNORE
};

//! \brief Represents an entity that can execute commands
//!
//! All player objects, console etc. should implement this
class CommandSender{
public:

    //! \brief Returns the unique name of this entity, this can be something like a steam id
    //! or some other static name
    DLLEXPORT virtual const std::string& GetUniqueName() = 0;

    //! \brief Returns a friendly name, nickname used to represent this entity
    //! \note This doesn't have to be unique
    DLLEXPORT virtual const std::string& GetNickname() = 0;


    //! \brief Returns the permissions model used by this object
    DLLEXPORT virtual COMMANDSENDER_PERMISSIONMODE GetPermissionMode() = 0;

    //! \brief This should send a message to this entity for them to
    //! see what happened when they executed a command
    DLLEXPORT virtual void SendPrivateMessage(const std::string &message);


    //! \brief Marks the start of a time period during which this object needs to report
    //! if it is released
    //! 
    //! \warning It needs to be notified that calls with the same commander may NOT be
    //! ignored as the EndOwnership call of first would end the
    //! ownership when in reality the single command handler still wants this object
    DLLEXPORT virtual void StartOwnership(CommandHandler* commander);


    //! \brief Marks the end of ownership, it is no longer required to
    //! report that this object is released
    DLLEXPORT virtual void EndOwnership(CommandHandler* which);

protected:

    //! \brief Call when releasing, this is only required if
    //! StartOwnership and EndOwnership aren't overloaded
    DLLEXPORT virtual void _OnReleaseParentCommanders();


    //! \brief Should be the actual implementation of SendPrivateMessage
    //! \return Return false when unable to send (the message will them appear in the server
    //! log where someone hopefully will see it)
    DLLEXPORT virtual bool _OnSendPrivateMessage(const std::string &message) = 0;


    //! Holds the command handler parents that we need to notify
    //! \note This might hold multiple instances
    std::list<CommandHandler*> CommandHandlersToNotify;
};


//! \brief Handles all commands sent by the players on the server
class CommandHandler{
public:
    //! \brief Constructs a CommandHandler for use by a single server network interface
    DLLEXPORT CommandHandler(NetworkServerInterface* owneraccess);
    DLLEXPORT virtual ~CommandHandler();



    //! \brief Queues a command to be executed
    DLLEXPORT virtual void QueueCommand(const std::string &command, CommandSender* issuer);


    //! \brief Call this periodically to perform cleanup tasks 
    //!
    //! and depending on actual implementation the command handling. The default
    //! implementation doesn't really need this, but it should still be called
    DLLEXPORT virtual void UpdateStatus();


    //! \brief This should be called by CommandSender subclasses when they are no longer
    //! available to remove their commands from the queue
    DLLEXPORT virtual void RemoveMe(CommandSender* object);


    //! \brief Called by an actual command callback to make sure that the sender
    //! is still active
    //! \param retlock Will contain a lock for the object so hold onto it while
    //! using the object
    DLLEXPORT virtual bool IsSenderStillValid(CommandSender* checkthis);


    //! \brief Called by a command handler when a CommandSender is no longer needed
    //! \param stillgotthis Is the lock received from IsSenderStillValid
    DLLEXPORT virtual void SenderNoLongerRequired(CommandSender* checkthis);


    //! \brief Registers a new custom command handler
    //! \param handler The object to use for handling. The object will be owned by this and
    //! will be deleted when it is no longer used
    //! \see CustomCommandHandler
    DLLEXPORT virtual bool RegisterCustomCommandHandler(
        std::shared_ptr<CustomCommandHandler> handler);


    // ------------------ The default command handler part ------------------ //

    //! \brief Returns true when the default command handling function can process this
    DLLEXPORT virtual bool IsThisDefaultCommand(const std::string &firstword) const;


    DLLEXPORT static CommandHandler* Get();

protected:

    //! \brief Lets go of all the CommandSenders
    void _LetGoOfAll();

    //! \brief Adds a CommandSender to the list of active ones
    void _AddSender(CommandSender* object);

    // ------------------------------------ //


    //! Holds all the command dispatchers who are currently executing commands
    std::list<CommandSender*> SendersInUse;


    //! Pointer to the owning interface for fetching various other things
    NetworkServerInterface* Owner;


    //! All the custom command providers
    std::vector<std::shared_ptr<CustomCommandHandler>> CustomHandlers;


    //! A static access member for command executing functions
    static CommandHandler* Staticaccess;
};

}

#ifdef LEAK_INTO_GLOBAL
using Leviathan::CommandSender;
using Leviathan::CommandHandler;
#endif

