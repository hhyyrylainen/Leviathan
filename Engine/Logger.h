#pragma once
// ------------------------------------ //
#include "Include.h"
#include "ErrorReporter.h"

namespace Leviathan{

//! \brief Logger class for all text output
//! \todo Allow logs that don't save to a file
class Logger : public LErrorReporter{
public:
        
    DLLEXPORT Logger(const std::string &file);
    DLLEXPORT virtual ~Logger();

    // Logging functions
    DLLEXPORT void Write(const std::string &data) override;
    DLLEXPORT void Info(const std::string &data) override;
    DLLEXPORT void Error(const std::string &data) override;
    DLLEXPORT void Warning(const std::string &data) override;
    DLLEXPORT void WriteLine(const std::string &Text) override;
    DLLEXPORT void Fatal(const std::string &Text) override;

    //! \brief Prints output without extra newlines (make sure to add them manually)
    //!
    //! This doesn't play nice with tests so use sparingly
    DLLEXPORT void WriteRaw(const std::string &data);
    
        
    DLLEXPORT static void SendDebugMessage(const std::string &str);

    //! \brief Script wrapper
    DLLEXPORT static void Print(const std::string &message);

    DLLEXPORT void Save();

    //! \brief Adds raw data to the queue unmodified
    //! \note You will need to add new lines '\n' manually
    DLLEXPORT void DirectWriteBuffer(const std::string &data);

    //! \brief Gets the file the log is being written to
    DLLEXPORT std::string GetLogFile() const;


    DLLEXPORT static Logger* Get();
        
private:

    //! Call only after locking
    void _Save();

    //! \note The global logger mutex needs to be locked before this call
    void _LogUpdateEndPart();

private:
    
    std::string PendingLog;
    std::string Path;

    static Logger* LatestLogger;
};
}

#ifdef LEAK_INTO_GLOBAL
using Leviathan::Logger;
#endif

