#include "DummyLog.h"

#include "catch.hpp"

void DummyReporter::Write(const std::string &Text) {
}

void DummyReporter::WriteLine(const std::string &Text) {
}

void DummyReporter::Info(const std::string &Text) {
}

void DummyReporter::Warning(const std::string &Text) {
}

void DummyReporter::Error(const std::string &Text) {
    CHECK(false);
}

void DummyReporter::Fatal(const std::string &Text) {
    REQUIRE(false);
}
