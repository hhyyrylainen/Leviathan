// ------------------------------------ //
#include "ConsoleInput.h"

#include <stdio.h>
using namespace Leviathan;
// ------------------------------------ //
#ifdef __unix__

#include <unistd.h>

#endif


ConsoleInput::~ConsoleInput(){

    if(!Initialized)
        return;

    if (StdInThread.joinable()) {

        // If this is true this hasn't been closed properly //
        LEVIATHAN_ASSERT(!ReadingInput, "ConsoleInput not closed properly");

        StdInThread.join();
    }

    StdInUse = false;
}

std::atomic<bool> ConsoleInput::StdInUse { false };
// ------------------------------------ //
bool ConsoleInput::Init(std::function<bool (const std::string&)> callback,
    bool canopenconsole /*= true*/)
{
    Callback = callback;
    
    // Check is already in use //
    if(StdInUse){

        DEBUG_BREAK;
        return false;
    }

    const bool oldvalue = StdInUse.exchange(true);

    if(oldvalue){

        // Somebody else got it //
        return false;
    }

    // Guaranteed to be the only object that gets here before our destructor //
    Initialized = true;
    
#ifdef _WIN32
    // Open a console if not already open //
    if (GetConsoleWindow() == nullptr && canopenconsole) {

        // Create a new console //
        Logger::Get()->Info("Creating a console window for input");
        CreatedNewConsole = true;
        CreateConsoleWindow();

        if (GetConsoleWindow() != nullptr) {

            Logger::Get()->Info("Leviathan Console");
        }
    }
#endif // _WIN32

    if(!PrepareWait()){

        DEBUG_BREAK;
        return false;
    }
    
    ReadingInput = true;
    StdInThread = std::thread(std::bind(&ConsoleInput::WaitForInput, this));

    
    return true;
}
// ------------------------------------ //
void ConsoleInput::Release(bool waitquit /*= false*/){

    if (ReadingInput) {

        StopWaiting();

        ReadingInput = false;

        if(waitquit && StdInThread.joinable())
            StdInThread.join();
    }
#ifdef _WIN32

    DestroyConsoleWindow();

#endif // _WIN32
}
// ------------------------------------ //
bool ConsoleInput::IsAttachedToConsole(){
#ifdef _WIN32

    return GetConsoleWindow() != nullptr;
    
#else
    if(isatty(fileno(stdin)))
        return true;
    
    return false;
#endif
}
// ------------------------------------ //
bool ConsoleInput::OnReceivedInput(const std::string &str){

    return Callback(str);
}
// ------------------------------------ //
#ifdef _WIN32

#include <processenv.h>
#include <winbase.h>
#include <iostream>


void ConsoleInput::CreateConsoleWindow() {

    constexpr int MAX_CONSOLE_LINES = 500;

    // Method from http://www.halcyon.com/~ast/dload/guicon.htm
    // Better method from
    // http://stackoverflow.com/questions/311955/redirecting-cout-to-a-console-in-windows

    // Allocate a console for this app
    if (!AllocConsole()) {

        LOG_ERROR("ConsoleInput: AllocConsole failed");
        return;
    }

    // set the screen buffer to be big enough to let us scroll text
    CONSOLE_SCREEN_BUFFER_INFO coninfo;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);

    coninfo.dwSize.Y = MAX_CONSOLE_LINES;

    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

    // Redirect the CRT standard input, output, and error handles to the console
    //freopen("CONIN$", "r", stdin);
    //freopen("CONOUT$", "w", stdout);
    //freopen("CONOUT$", "w", stderr);

    FILE* ResultStream;

    if(freopen_s(&ResultStream, "CONIN$", "r", stdin) != 0)
        LOG_ERROR("ConsoleInput: freopen failed");
    if (freopen_s(&ResultStream, "CONOUT$", "w", stdout) != 0)
        LOG_ERROR("ConsoleInput: freopen failed");
    if (freopen_s(&ResultStream, "CONOUT$", "w", stderr) != 0)
        LOG_ERROR("ConsoleInput: freopen failed");

    // Clear the error state for each of the C++ standard stream
    // objects. We need to do this, as attempts to access the standard
    // streams before they refer to a valid target will cause the
    // iostream objects to enter an error state. In versions of Visual
    // Studio after 2005, this seems to always occur during startup
    // regardless of whether anything has been read from or written to
    // the console or not.
    std::wcout.clear();
    std::cout.clear();
    std::wcerr.clear();
    std::cerr.clear();
    std::wcin.clear();
    std::cin.clear();

    std::ios::sync_with_stdio();
}

void ConsoleInput::DestroyConsoleWindow() {

    if (CreatedNewConsole) {

        // unredirect output from the console //
        FILE* ResultStream;
        freopen_s(&ResultStream, "nul", "r", stdin);
        freopen_s(&ResultStream, "nul", "w", stdout);
        freopen_s(&ResultStream, "nul", "w", stderr);

        CreatedNewConsole = false;
        while (!FreeConsole()) {

            Logger::Get()->Error("ConsoleInput: Closing Windows console window failed, retrying");
        }
    }
}

void ConsoleInput::WaitForInput() {

    char ReadBuffer[200];

    while (ConsoleInputFile > 0) {

        DWORD BytesRead;
        if (!ReadFile(ConsoleInputFile, &ReadBuffer, 199, &BytesRead, NULL)) {

            // If our read was canceled, quit without a message //
            if (GetLastError() == ERROR_OPERATION_ABORTED)
                break;

            Logger::Get()->Warning("ConsoleInput: Stdin read failed, stopping input thread");
            break;
        }

        if(BytesRead < 1)
            continue;

        ReadBuffer[BytesRead] = '\0';

        OnReceivedInput(std::string(ReadBuffer, BytesRead));
    }
}

bool ConsoleInput::PrepareWait() {

    ConsoleInputFile = CreateFile(TEXT("CONIN$"), GENERIC_READ, FILE_SHARE_READ,
        NULL, OPEN_EXISTING, NULL, NULL);

    if (!ConsoleInputFile) {

        DEBUG_BREAK;
        return false;
    }

    return true;
}

void ConsoleInput::StopWaiting() {

    if(StdInThread.joinable())
        CancelSynchronousIo(StdInThread.native_handle());

    if (ConsoleInputFile > 0) {

        CloseHandle(ConsoleInputFile);
        ConsoleInputFile = 0;
    }
}

#elif defined(__unix__)

#include <unistd.h>
#include <sys/select.h>

void ConsoleInput::WaitForInput() {

    const auto StdIn = fileno(stdin);

    char ReadBuffer[200];

    fd_set rfds;

    while (true) {

        FD_ZERO(&rfds);
        FD_SET(ReadCancelPipe[0], &rfds);
        FD_SET(StdIn, &rfds);

        const auto MaxFD = std::max(ReadCancelPipe[0], StdIn) + 1;

        int SelectRes = select(MaxFD, &rfds, NULL, NULL, NULL);

        if (SelectRes == -1) {

            LOG_ERROR("ConsoleInput: select failed, stopping input thread");
            break;
        }

        if(SelectRes == 0)
            continue;

        // Quit check //
        if (FD_ISSET(ReadCancelPipe[0], &rfds)) {

            // Received quit message //
            break;
        }

        // The other must be valid //
        LEVIATHAN_ASSERT(FD_ISSET(StdIn, &rfds), "FD set was all empty");

        // This should not block as it is ready for reading //
        const auto ReadBytes = read(StdIn, &ReadBuffer, 199);

        if (ReadBytes == -1) {

            // Error //
            LOG_WARNING("ConsoleInput: read on stdin failed, stopping input thread");
            break;
        }

        if (ReadBytes == 0) {

            // End of std in //
            LOG_INFO("ConsoleInput: stdin read end reached, stopping input thread");
            break;
        }

        ReadBuffer[ReadBytes] = '\0';

        OnReceivedInput(std::string(ReadBuffer, ReadBytes));
    }

    close(ReadCancelPipe[0]);
}

bool ConsoleInput::PrepareWait() {

    if (pipe(ReadCancelPipe) == -1) {

        LOG_ERROR("ConsoleInput: Failed to create pipe");
        return false;
    }

    return true;
}

void ConsoleInput::StopWaiting() {

    static const char CloseMessage[] = { 'c' };

    if (write(ReadCancelPipe[1], CloseMessage, 1) != 1) {

        LOG_ERROR("ConsoleInput: Failed to signal input thread pipe");
    }

    // Close the write end, WaitForInput will close the other end
    close(ReadCancelPipe[1]);
}


#else
#error ConsoleInput not implemented for platform
#endif 
