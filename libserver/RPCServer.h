#pragma once

#include <libcommon/Common.h>
#include <libcommon/Icon.h>
#include <libcommon/GlobalConfig.h>
#include <libhandler/RPCHandler.h>

#include "archer_net.h"
#include "HttpHandler.h"
#include "SignatureFilter.h"

namespace fs 
{
namespace server 
{
class RPCServer
{
    enum MsgType {CONNECT, JSON, FILE};
public:

    RPCServer();
    ~RPCServer();

    void listen(std::string const& host, std::uint16_t const& port);

    void close();

    void useMultiThreads(uint16_t const& threadNums);

    void handle(Channel *channel, char *data, size_t data_len);

    void closeChannel(Channel *channel);
    
    void setHandler(std::shared_ptr<fs::handler::RPCHandler> const& handler) {
        m_handler = handler;
    }

private:

    std::string                                   m_iconUri = FS_ICON_API;
    ServerChannel                                *m_server;
    BaseHandler                                  *m_baseHandler;
    uint16_t                                      m_httpThreadNum = 0;
    std::shared_ptr<fs::handler::RPCHandler>      m_handler;
};
}
}

