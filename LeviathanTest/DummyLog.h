#pragma once

#include "ErrorReporter.h"

class DummyReporter : public LErrorReporter {
public:
    virtual void Write(const std::string &Text) override;


    virtual void WriteLine(const std::string &Text) override;


    virtual void Info(const std::string &Text) override;


    virtual void Warning(const std::string &Text) override;


    virtual void Error(const std::string &Text) override;


    virtual void Fatal(const std::string &Text) override;

};