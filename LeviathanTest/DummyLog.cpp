#include "DummyLog.h"

#include "Utility/Convert.h"

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

void RequireErrorReporter::Warning(const std::string &text) {

    INFO(text);
}

void RequireErrorReporter::Error(const std::string &text) {
    ErrorOccured = true;
}

void RequireErrorReporter::Fatal(const std::string &text) {
    FAIL(text);
    REQUIRE(false);
}
// ------------------------------------ //
//! \brief Matches line numbers for ReporterLineNumberChecker
//!
//! First capture group is the line number
static const std::regex LineNumRegex {R"(\S:(\d+)\s*$)"};

void ReporterLineNumberChecker::Warning(const std::string &text){
            
    INFO(text);
        
    if(AlsoWarnings)
        GetLine(text);
}

void ReporterLineNumberChecker::Error(const std::string &text){
    
    INFO(text);
    GetLine(text);
}

void ReporterLineNumberChecker::GetLine(const std::string &text){

    std::smatch lineMatch;

    if(std::regex_search(text, lineMatch, LineNumRegex)){

        if(lineMatch.size() == 2){

            ErrorLines.push_back(Convert::StringTo<int>(lineMatch[1]));
        }
    }
}


// ------------------------------------ //
// ReporterMatchMessagesRegex
ReporterMatchMessagesRegex::ReporterMatchMessagesRegex(){

}

ReporterMatchMessagesRegex::ReporterMatchMessagesRegex(
    std::vector<MessageToLookFor>&& messages) : MessagesToDetect(std::move(messages))
{

}
// ------------------------------------ //
void ReporterMatchMessagesRegex::Write(const std::string &text){

    if(!CheckWrite)
        return;
    
    for(auto &messagechecker : MessagesToDetect){
        if(messagechecker.CheckInfo){

            if(std::regex_match(text, messagechecker.MatchRegex)){

                ++messagechecker.MatchCount;
            }
        }
    }
}

void ReporterMatchMessagesRegex::Info(const std::string &text){
    
    for(auto &messagechecker : MessagesToDetect){
        if(messagechecker.CheckInfo){

            if(std::regex_match(text, messagechecker.MatchRegex)){

                ++messagechecker.MatchCount;
            }
        }
    }
}

void ReporterMatchMessagesRegex::Warning(const std::string &text){
    
    for(auto &messagechecker : MessagesToDetect){
        if(messagechecker.CheckWarning){

            if(std::regex_match(text, messagechecker.MatchRegex)){

                ++messagechecker.MatchCount;
            }
        }
    }
}

void ReporterMatchMessagesRegex::Error(const std::string &text){
    
    for(auto &messagechecker : MessagesToDetect){
        if(messagechecker.CheckError){

            if(std::regex_match(text, messagechecker.MatchRegex)){

                ++messagechecker.MatchCount;
            }
        }
    }
}
// ------------------------------------ //
ReporterMatchMessagesRegex::MessageToLookFor::MessageToLookFor(const std::regex &regex) :
    MatchRegex(regex)
{
    
}

void ReporterMatchMessagesRegex::MessageToLookFor::CheckAndResetCountIsOne(){

    CHECK(MatchCount == 1);
    MatchCount = 0;
}
