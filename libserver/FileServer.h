#pragma once

#include <libcommon/Common.h>
#include <libcommon/Icon.h>
#include <libcommon/GlobalConfig.h>

#include "archer_net.h"
#include "HttpHandler.h"
#include "SignatureFilter.h"

namespace fs 
{
namespace server 
{
class FileServer
{
public:

    FileServer();
    ~FileServer();

    void listen(std::string const& host, std::uint16_t const& port);
    
    void close();

    void useMultiThreads(uint16_t const& threadNums);

    void addHandler(std::shared_ptr<HttpHandler> const& handler);

    void onRequest(std::shared_ptr<ServerRequest> const& httpReq, std::shared_ptr<HttpHandler> handler);

    std::shared_ptr<HttpHandler> findHandler(std::string const& method, std::string const& uri);

    std::string const& getIconUri() {return m_iconUri;};

private:
    void sendNotFound(HttpResponse *res);
    
    void sendIcon(HttpResponse *res);

    std::string                                   m_iconUri = FS_ICON_API;
    HttpServer                                   *m_http;
    std::vector<std::shared_ptr<HttpHandler>>     m_handlerList;
    uint16_t                                      m_httpThreadNum;
    
    std::shared_ptr<SignatureFilter>              m_filter;
};
}
}