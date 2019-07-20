// ------------------------------------ //
#include "CEFLocalResourceRequest.h"

#include "Common/MimeTypes.h"
#include "Common/StringOperations.h"

#include "include/wrapper/cef_helpers.h"

#include <fstream>

using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //

namespace Leviathan { namespace GUI {

//! \brief Custom file reader for serving local files
//! \todo This needs blocking against reading files outside the game/Data directory
class CefFileResourceHandler : public CefResourceHandler {
public:
    CefFileResourceHandler(CefRefPtr<CefRequest> request) : Request(request) {}

    void Cancel() override {}

    // bool CanGetCookie(const CefCookie& cookie) override
    // {
    //     return false;
    // }

    // bool CanSetCookie(const CefCookie& cookie) override
    // {
    //     return false;
    // }

    void GetResponseHeaders(CefRefPtr<CefResponse> response, int64& response_length,
        CefString& redirectUrl) override
    {
        // Count response size
        FileReader.seekg(0, std::ios::beg);
        std::streampos size = FileReader.tellg();
        FileReader.seekg(0, std::ios::end);
        size = FileReader.tellg() - size;
        FileReader.seekg(0, std::ios::beg);

        response_length = size;

        // Set status code
        response->SetStatus(200);
        response->SetStatusText("OK");

        // Set type header
        response->SetMimeType(
            GetMimeTypeFromPath(StringOperations::URLPath(Request->GetURL())).data());
    }

    bool ProcessRequest(
        CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback) override
    {
        // Check if it is good //
        const auto localFile = StringOperations::URLPath(Request->GetURL());
        FileReader.open(localFile, std::ios::binary);

        if(!FileReader.good()) {
            LOG(WARNING) << "GET missing local file: " << localFile;
            return false;
        }

        // LOG(INFO) << "GET local file: " << localFile;

        // We are immediately ready to return the headers
        callback->Continue();
        return true;
    }

    bool ReadResponse(void* data_out, int bytes_to_read, int& bytes_read,
        CefRefPtr<CefCallback> callback) override
    {
        if(!FileReader.good() || FileReader.eof()) {
            bytes_read = 0;
            return false;
        }

        FileReader.read(reinterpret_cast<char*>(data_out), bytes_to_read);
        bytes_read = FileReader.gcount();

        // TODO: check if this is needed. Without this this doesn't seem to get stuck
        // // We are always good to read
        // if(bytes_read == 0)
        //     callback->Continue();
        return true;
    }

    IMPLEMENT_REFCOUNTING(CefFileResourceHandler);

private:
    CefRefPtr<CefRequest> Request;
    std::ifstream FileReader;
};

}} // namespace Leviathan::GUI


// ------------------------------------ //
// CEFLocalResourceRequestHandlerFactory
CefRefPtr<CefResourceHandler> CefLocalResourceRequestHandlerFactory::Create(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& scheme_name,
    CefRefPtr<CefRequest> request)
{
    CEF_REQUIRE_IO_THREAD();
    return new CefFileResourceHandler(request);
}
