#pragma once

#include "ErrorReporter.h"

class DummyReporter : public LErrorReporter {
public:
    virtual void Write(const std::string &text) override;


    virtual void WriteLine(const std::string &text) override;


    virtual void Info(const std::string &text) override;


    virtual void Warning(const std::string &text) override;


    virtual void Error(const std::string &text) override;


    virtual void Fatal(const std::string &text) override;

};

class RequireErrorReporter : public LErrorReporter {
public:

    ~RequireErrorReporter();
    
    virtual void Write(const std::string &text) override;


    virtual void WriteLine(const std::string &text) override;


    virtual void Info(const std::string &text) override;


    virtual void Warning(const std::string &text) override;


    virtual void Error(const std::string &text) override;


    virtual void Fatal(const std::string &text) override;

    bool ErrorOccured = false;
};
