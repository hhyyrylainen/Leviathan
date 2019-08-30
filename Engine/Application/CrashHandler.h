// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include <functional>

namespace Leviathan {

//! \brief An orchestrator for crashing
//!
//! Manages the disparencies between BSF and Breakpad (if application main has Breakpad
//! enabled)
class CrashHandler {
public:
    CrashHandler() = delete;

    //! \brief Triggers Breakpad callback if registered
    static void DoBreakpadCrashDumpIfRegistered();

    //! \brief Triggers Breakpad callback if registered
    static void DoBreakpadSEHCrashDumpIfRegistered(void* data);

    //! \brief Handler for when BSF gets called first, should call the
    //! google_breakpad::ExceptionHandler::SimulateSignalDelivery or Windows equivalent
    DLLEXPORT static void RegisterBreakpadGenericHandler(std::function<void()> callback);

    //! \brief Handler for SEH, this is separate to allow the data pointer to be passed
    DLLEXPORT static void RegisterBreakpadWindowsSEHHandler(
        std::function<void(void* data)> callback);

    //! \brief Called after creating a Breakpad dump in order to also print the accessible
    //! callstack with BSF
    DLLEXPORT static void DoBSFCallStackAfterBreakpad();

    //! \returns True when Breakpad is registered
    //!
    //! This is used to skip writing the crash reports to disk (and showing the popup dialog on
    //! windows) from BSF.
    DLLEXPORT static bool IsBreakpadRegistered();

private:
    static std::function<void()> GenericCallback;
    static std::function<void(void* data)> SEHCallback;
    static bool BSFTriggered;
};

} // namespace Leviathan
