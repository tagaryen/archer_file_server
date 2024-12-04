#pragma once

#include "ServerRequest.h"
#include <libcommon/Common.h>

namespace fs 
{
namespace server 
{
class HttpHandler
{
public:
    HttpHandler() = default;

    virtual ~HttpHandler(){};

    virtual void handleRequest(std::shared_ptr<ServerRequest> request) = 0;

    virtual std::string const& getMappingUri() = 0;
    
    virtual std::string const& getMappingMethod() = 0;
    
    virtual bool needAuth() = 0;

    RequestType getRequestType() {
        std::string uri = getMappingUri();
        if(uri == FS_OPEN_API_FILE_UPLOAD || 
           uri == FS_OPEN_API_FILE_VIEW || 
           uri == FS_OPEN_API_FILE_DOWNLOAD || 
           uri == FS_API_FILE_UPLOAD || 
           uri == FS_API_FILE_VIEW || 
           uri == FS_API_FILE_DOWNLOAD) {
               return FILE_REQUEST;
        } else if(
           uri == FS_OPEN_API_JSON || 
           uri == FS_API_JSON) {
                return JSON_REQUEST;
        }
        return JSON_REQUEST;
    }
};
}
}