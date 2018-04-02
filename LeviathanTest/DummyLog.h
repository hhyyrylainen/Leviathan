#pragma once

#include "ErrorReporter.h"


#include <vector>
#include <regex>

namespace Leviathan{
namespace Test{

//! Base class for others in this file to make them shorter
class DummyReporter : public LErrorReporter{
public:
    
    virtual void Write(const std::string &text) override;


    virtual void WriteLine(const std::string &text) override;


    virtual void Info(const std::string &text) override;


    virtual void Warning(const std::string &text) override;


    virtual void Error(const std::string &text) override;


    virtual void Fatal(const std::string &text) override;
    
};

class RequireErrorReporter : public DummyReporter {
public:

    ~RequireErrorReporter();
    
    virtual void Warning(const std::string &text) override;


    virtual void Error(const std::string &text) override;


    virtual void Fatal(const std::string &text) override;

    bool ErrorOccured = false;
};


//! Detects line numbers in error messages
class ReporterLineNumberChecker : public DummyReporter{
public:

    virtual void Warning(const std::string &text) override;

    virtual void Error(const std::string &text) override;

    //! Captures error line number and saves it in ErrorLines
    void GetLine(const std::string &text);

    bool AlsoWarnings = false;

    //! \see GetLine
    std::vector<int> ErrorLines;
};


//! Detects printed lines matching regex
class ReporterMatchMessagesRegex : public DummyReporter{
public:

    struct MessageToLookFor{

        MessageToLookFor(const std::regex &regex);

        //! Checks that match count is 1 and then resets it
        void CheckAndResetCountIsOne();

        std::regex MatchRegex;
        bool CheckWarning = false;
        bool CheckInfo = false;
        bool CheckError = true;

        //! This is incremented when matched
        int MatchCount = 0;
    };
    
public:

    ReporterMatchMessagesRegex();
    ReporterMatchMessagesRegex(std::vector<MessageToLookFor>&& messages);

    virtual void Write(const std::string &text) override;
    
    virtual void Info(const std::string &text) override;

    virtual void Warning(const std::string &text) override;

    virtual void Error(const std::string &text) override;
    
    std::vector<MessageToLookFor> MessagesToDetect;

    bool CheckWrite = false;
};

}
}
