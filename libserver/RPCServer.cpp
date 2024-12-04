#include "RPCServer.h"

using namespace fs::server;

static void server_on_connect_cb(Channel *channel) {
    LOG_info("Remote %s:%d connected", channel_get_host(channel), channel_get_port(channel));
}

static void channel_on_close_cb(Channel *channel) {
    LOG_info("Remote %s:%d closed", channel_get_host(channel), channel_get_port(channel));
    RPCServer *server = (RPCServer *) channel_get_arg(channel);
    server->closeChannel(channel);
}

static void channel_on_read_cb(Channel *channel, char *data, size_t data_len) {
    RPCServer *server = (RPCServer *) channel_get_arg(channel);
    server->handle(channel, data, data_len);
}

static void channel_on_error_cb(Channel *channel, const char *err_msg) {
    LOG_error("Remote %s:%d error, %s", channel_get_host(channel), channel_get_port(channel), err_msg);
}


RPCServer::RPCServer() {
    m_server = server_channel_new();
    m_baseHandler = base_handler_new();
    server_channel_set_arg(m_server, this);
    
    base_handler_set_channel_on_connect(m_baseHandler, server_on_connect_cb);
    base_handler_set_channel_on_read(m_baseHandler, channel_on_read_cb);
    base_handler_set_channel_on_error(m_baseHandler, channel_on_error_cb);
    base_handler_set_channel_on_close(m_baseHandler, channel_on_close_cb);
    base_handler_handle_server_channel(m_baseHandler, m_server);

    // server_channel_set_channel_on_connect(m_server, server_on_connect_cb);
    // server_channel_set_channel_on_read(m_server, channel_on_read_cb);
    // server_channel_set_channel_on_close(m_server, channel_on_close_cb);
    // server_channel_set_channel_on_error(m_server, channel_on_error_cb);
}

RPCServer::~RPCServer() {
    server_channel_close(m_server);
    server_channel_free(m_server);
    base_handler_free(m_baseHandler);
}


void RPCServer::listen(std::string const& host, std::uint16_t const& port) {

    if(m_httpThreadNum > 0) {
        server_channel_set_eventloop_threads(m_server, m_httpThreadNum);
        server_channel_set_read_threads(m_server, m_httpThreadNum);
    }

    console_out("RPC Server listenning on %s:%d", host.c_str(), port);
    LOG_info("RPC Server listenned on %s:%d", host.c_str(), port);
    if(!server_channel_listen(m_server, host.c_str(), port)) {
        const char *errstr = server_channel_get_errstr(m_server);
        console_err("RPC Server listen on %s:%d error, %s", host.c_str(), port, errstr);
        LOG_error("RPC Server listen on %s:%d error, %s", host.c_str(), port, errstr);
    }
}

void RPCServer::close() {
    server_channel_close(m_server);
}

void RPCServer::useMultiThreads(uint16_t const& threadNum) {
    m_httpThreadNum = threadNum;
}

void RPCServer::handle(Channel *channel, char *data, size_t data_len) {
    m_handler->handleMessage(channel, data, data_len);
}

void RPCServer::closeChannel(Channel *channel) {
    m_handler->close(channel);
}

