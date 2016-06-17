// (c) Henri Hyyryläinen 2015-2016
#pragma once
#include "Include.h"
#include <string>

// Error reporting interface //
class LErrorReporter {
public:

    virtual void Write(const std::string &Text) = 0;
    virtual void WriteLine(const std::string &Text) = 0;

    virtual void Info(const std::string &Text) = 0;
    virtual void Warning(const std::string &Text) = 0;
    virtual void Error(const std::string &Text) = 0;

    //! Quits the current game with an error message
    virtual void Fatal(const std::string &Text) = 0;
};

#if UE_BUILD_DEBUG == 1 || UE_BUILD_DEVELOPMENT == 1 || UE_BUILD_TEST == 1 || UE_BUILD_SHIPPING == 1
#ifndef LEVIATHAN_UE_PLUGIN
#error One of UE build macros is defined but LEVIATHAN_UE_PLUGIN is not defined
#endif //LEVIATHAN_UE_PLUGIN
#include "Platform.h"


class FErrorReporter : public LErrorReporter {
public:

    virtual void WriteF(const FString &Text) = 0;
    virtual void WriteLineF(const FString &Text) = 0;

    virtual void InfoF(const FString &Text) = 0;
    virtual void WarningF(const FString &Text) = 0;
    virtual void ErrorF(const FString &Text) = 0;

    //! Quits the current game with an error message
    virtual void FatalF(const FString &Text) = 0;

};
#endif // UE4

