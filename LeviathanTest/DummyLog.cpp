#include "DummyLog.h"

#include "catch.hpp"

using namespace Leviathan::Test;

void DummyReporter::Write(const std::string &text) {
}

void DummyReporter::WriteLine(const std::string &text) {
}

void DummyReporter::Info(const std::string &text) {

    INFO(text);
}

void DummyReporter::Warning(const std::string &text) {

    WARN(text);
}

void DummyReporter::Error(const std::string &text) {

    FAIL(text);
}

void DummyReporter::Fatal(const std::string &text) {
    FAIL(text);
    REQUIRE(false);
}
// ------------------------------------ //
RequireErrorReporter::~RequireErrorReporter(){

    CHECK(ErrorOccured);
}

void RequireErrorReporter::Write(const std::string &text) {
}

void RequireErrorReporter::WriteLine(const std::string &text) {
}

void RequireErrorReporter::Info(const std::string &text) {

    INFO(text);
}

void RequireErrorReporter::Warning(const std::string &text) {

    WARN(text);
}

void RequireErrorReporter::Error(const std::string &text) {
    ErrorOccured = true;
}

void RequireErrorReporter::Fatal(const std::string &text) {
    FAIL(text);
    REQUIRE(false);
}
