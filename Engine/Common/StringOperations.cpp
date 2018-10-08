// ------------------------------------ //
#include "StringOperations.h"
using namespace Leviathan;
// ------------------------------------ //

template<>
DLLEXPORT void StringOperations::MakeString(
    std::wstring& str, const char* characters, size_t count)
{
    // Skip copying null terminator
    const size_t copysize = count - 1;
    str.resize(copysize);

    for(size_t i = 0; i < copysize; ++i)
        str[i] = (wchar_t)characters[i];
}

DLLEXPORT std::string StringOperations::URLProtocol(const std::string& url)
{
    const auto colonpos = url.find_first_of(':');

    if(colonpos == std::string::npos)
        return "";

    return url.substr(0, colonpos);
}

DLLEXPORT std::string StringOperations::BaseHostName(const std::string& url)
{
    if(url.empty())
        return "";

    // Start scanning until a '/' is found that is not preceeded by ':' or '/'
    size_t length = 0;

    for(size_t i = 0; i < url.size(); ++i) {

        length = i + 1;

        if(url[i] == '/') {

            if(i < 1)
                continue;

            if(url[i - 1] == ':' || url[i - 1] == '/')
                continue;

            // found it //
            break;
        }
    }

    // Make sure it has an ending '/'
    if(length == url.size() && url.back() != '/')
        return url + "/";

    return url.substr(0, length);
}

DLLEXPORT std::string StringOperations::URLPath(
    const std::string& url, bool stripoptions /*= true*/)
{
    if(url.empty())
        return "";

    // Start scanning until a '/' is found that is not preceeded by ':' or '/'
    size_t startCopy = 0;

    for(size_t i = 0; i < url.size(); ++i) {

        if(url[i] == '/') {

            if(i < 1)
                continue;

            if(url[i - 1] == ':' || url[i - 1] == '/')
                continue;

            // found it //
            startCopy = i + 1;
            break;
        }
    }

    // Make sure the string doesn't end there
    if(startCopy >= url.size())
        return "";

    size_t endCopy = url.size();

    // Scan backwards for cutting options
    if(stripoptions) {
        for(size_t cut = endCopy; cut > startCopy; --cut) {
            if(url[cut] == '?') {
                // Found options
                endCopy = cut;
                break;
            }
        }
    }

    return url.substr(startCopy, endCopy - startCopy);
}

DLLEXPORT std::string StringOperations::CombineURL(
    const std::string& first, const std::string& second)
{
    // To fix messed up urls we always do this cleanup
    const auto cleanedUpSecond = RemovePartsBeforeAbsoluteURLParts(second);

    if(first.empty())
        return cleanedUpSecond;

    if(cleanedUpSecond.empty())
        return first;

    // If second is an absolute URL just return it //
    if(cleanedUpSecond.find("://") != std::string::npos)
        return cleanedUpSecond;

    // If the other starts with double '//' then we just need to grab the protocol from the
    // first and add the second
    if(cleanedUpSecond.find("//") == 0)
        return URLProtocol(first) + ":" + second;

    // Simplest case: first ends with '/' and second doesn't begin with '/'
    if(first.back() == '/' && cleanedUpSecond.front() != '/')
        return first + cleanedUpSecond;

    // Second begins with '/': trim the first to the base url and then append the second
    if(cleanedUpSecond.front() == '/')
        return BaseHostName(first) + cleanedUpSecond.substr(1);

    // An error catching function
    // If first is the basehostname then just combine them
    if(first.back() != '/' && BaseHostName(first).length() == first.length() + 1)
        return first + "/" + cleanedUpSecond;

    // Most complex case: trim from the end of first until the last '/' and then append second
    const auto lastpos = first.find_last_of('/');
    return first.substr(0, lastpos + 1) + cleanedUpSecond;
}

DLLEXPORT std::string StringOperations::RemovePartsBeforeAbsoluteURLParts(
    const std::string& url)
{
    // Detect two '//'s in a path
    const auto colonPos = url.find_first_of(':');

    if(colonPos != std::string::npos) {

        // First double slash
        auto firstDouble = url.find("//", colonPos + 3);
        // Second
        const auto secondDouble = url.find("//", firstDouble + 2);

        if(firstDouble != std::string::npos && secondDouble != std::string::npos) {

            // If the part between the double slashes looks like a
            // domain then we cut the part between the protocol and
            // the first double slash
            firstDouble += 2;

            if(IsURLDomain(url.substr(firstDouble, secondDouble - firstDouble))) {

                return URLProtocol(url) + "://" +
                       url.substr(firstDouble, secondDouble - firstDouble) + "/" +
                       url.substr(secondDouble + 2);
            }
        }
    }

    return url;
}

DLLEXPORT bool StringOperations::IsURLDomain(const std::string& str)
{
    // Must have a dot
    bool dotSeen = false;

    for(char c : str) {
        if(c == '.') {
            dotSeen = true;
            continue;
        }

        if(c >= '0' && c <= '9')
            continue;

        if(c >= 'A' && c <= 'Z')
            continue;

        if(c >= 'a' && c <= 'z')
            continue;

        return false;
    }

    return dotSeen;
}
