// ------------------------------------ //
#include "MimeTypes.h"

#include "Common/StringOperations.h"

using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT std::string_view Leviathan::GetMimeTypeFromPath(const std::string_view& path)
{
    const auto extension = StringOperations::GetExtension(path);

    if(extension == "js" || extension == "mjs")
        return "application/javascript";
    if(extension == "txt")
        return "text/plain";
    if(extension == "png")
        return "image/png";
    if(extension == "jpeg" || extension == "jpg")
        return "image/jpeg";
    if(extension == "gif")
        return "image/gif";
    if(extension == "svg")
        return "image/svg+xml";
    if(extension == "ogg" || extension == "opus")
        return "audio/ogg";
    if(extension == "css")
        return "text/css";
    if(extension == "html")
        return "text/html";
    if(extension == "json")
        return "application/json";
    if(extension == "xml")
        return "application/xml";
    if(extension == "mpg")
        return "audio/mpeg";
    if(extension == "xml")
        return "text/xml";
    if(extension == "csv")
        return "text/csv";
    if(extension == "mkv")
        return "video/x-matroska";
    if(extension == "webm")
        return "video/webm";
    if(extension == "mp4" || extension == "mpg4" || extension == "m4v")
        return "video/mp4";

    // Didn't match anything so this is just a guess
    return "text/plain";
}
