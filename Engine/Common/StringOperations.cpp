// ------------------------------------ //
#include "StringOperations.h"
using namespace Leviathan;
// ------------------------------------ //

template<>
    DLLEXPORT void StringOperations::MakeString(std::wstring &str, const char* characters,
        size_t count)
{
    // Skip copying null terminator
    const size_t copysize = count - 1;
    str.resize(copysize);

    for (size_t i = 0; i < copysize; ++i)
        str[i] = (wchar_t)characters[i];
}

std::string StringOperations::URLProtocol(const std::string &url){

    const auto colonpos = url.find_first_of(':');

    if(colonpos == std::string::npos)
        return "";

    return url.substr(0, colonpos);
}

std::string StringOperations::BaseHostName(const std::string &url){

    if(url.empty())
        return "";

    // Start scanning until a '/' is found that is not preceeded by ':' or '/'
    size_t length = 0;

    for(size_t i = 0; i < url.size(); ++i){

        length = i + 1;
        
        if(url[i] == '/'){

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

std::string StringOperations::URLPath(const std::string &url){

    if(url.empty())
        return "";

    // Start scanning until a '/' is found that is not preceeded by ':' or '/'
    size_t startCopy = 0;

    for(size_t i = 0; i < url.size(); ++i){

        if(url[i] == '/'){

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
    
    return url.substr(startCopy, url.size() - startCopy);
}

std::string StringOperations::CombineURL(const std::string &first, const std::string &second){

    if(first.empty())
        return second;

    if(second.empty())
        return first;

    // If second is an absolute URL just return it //
    if(second.find("://") != std::string::npos)
        return second;

    // Simplest case: first ends with '/' and second doesn't begin with '/'
    if(first.back() == '/' && second.front() != '/')
        return first + second;

    // Second begins with '/': trim the first to the base url and then append the second
    if(second.front() == '/')
        return BaseHostName(first) + second.substr(1);

    // An error catching function
    // If first is the basehostname then just combine them
    if(first.back() != '/' && BaseHostName(first).length() == first.length() + 1)
        return first + "/" + second;

    // Most complex case: trim from the end of first until the last '/' and then append second
    const auto lastpos = first.find_last_of('/');
    return first.substr(0, lastpos + 1) + second;
}
