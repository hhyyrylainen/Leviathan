// Leviathan Game Engine
// Copyright (c) 2012-2016 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "../Common/ThreadSafe.h"

#ifdef _WIN32
#include "WindowsInclude.h"
#endif

#include <string>
#include <functional>
#include <thread>
#include <atomic>

namespace Leviathan {

class ConsoleInput : public ThreadSafe {
public:
    
    DLLEXPORT ~ConsoleInput();
    
    //! \brief Starts listening for input from std in
    //!
    //! When input is received the callback is called
    //! \param canopenconsole If true will open a new console for this process if one doesn't
    //! exist. Does nothing on linux
    //! \param callback Called once a full line has been read. If returns true will stop
    //! listening
    DLLEXPORT bool Init(std::function<bool (const std::string&)> callback,
        bool canopenconsole = true);

    //! Stops listening.
    //! \param waitquit If true after this finishes the callback won't be called anymore
    //! If false callback can be called until the destructor runs, which will block until
    //! the read ends
    DLLEXPORT void Release(bool waitquit = false);

    //! \returns True if this process is controlled by an interactive console
    //! Can be used to quit linux servers if accidentally not started from a terminal
    DLLEXPORT static bool IsAttachedToConsole();
    
protected:

    //! Called by the listening thread when a line from input has been read
    //! \returns True if wants to stop listening, false if listening should continue
    virtual bool OnReceivedInput(const std::string &str);

    //! Must be called before WaitForInput starts in a new thread
    //! \returns True if can start waiting, false if something is wrong
    bool PrepareWait();

    //! Stops reading 
    void StopWaiting();

private:
    //! This is the main function for the input listening thread
    void WaitForInput();
    
private:

    //! True if this is actually used
    //! Set to true in Init
    bool Initialized = false;

    //! This is called when input is received
    std::function<bool (const std::string&)> Callback;

    //! Thread for the input
    std::thread StdInThread;
    //! True when StdInThread is active and listening
    std::atomic<bool> ReadingInput { false };


    //! Will make sure only one reading instance exists
    static std::atomic<bool> StdInUse;

    // Variables used from StdInThread //
    //! Contains raw read data
    std::string InputBuffer;

    //! Will contain the finalized input. This is passed to the handling function
    std::string InputWorkBuffer;

#ifdef _WIN32

    //! Used to close created consoles
    bool CreatedNewConsole = false;

    HANDLE ConsoleInputFile = nullptr;

    //! Creates a new console window and redirects io
    void CreateConsoleWindow();
    //! Detaches a console from this process
    void DestroyConsoleWindow();

#elif defined(__unix__)

    //! Pipe for notifying waiting thread
    int ReadCancelPipe[2];

#endif //_WIN32
};
}
