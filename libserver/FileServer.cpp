#include "FileServer.h"

using namespace fs::server;

static std::function<void(HttpRequest *req, HttpResponse *res)> fileServerOnRequestCallback;

void file_server_message_handler_cb(HttpRequest *req, HttpResponse *res) {
    fileServerOnRequestCallback(req, res);
}

FileServer::FileServer() {
    m_http = http_server_new();
    m_filter = std::make_shared<SignatureFilter>(128);
}

FileServer::~FileServer() {
    http_server_close(m_http);
    http_server_free(m_http);
}


void FileServer::listen(std::string const& host, std::uint16_t const& port) {
    
    http_server_set_message_handler(m_http, file_server_message_handler_cb);

    FileServer *self = this;
    fileServerOnRequestCallback = [self](HttpRequest *req, HttpResponse *res) {
        
        std::string uri(http_request_get_uri(req));
        std::string method(http_request_get_method(req));

        if(uri == self->getIconUri()) {
            self->sendIcon(res);
            return ;
        }

        std::shared_ptr<HttpHandler> handler = self->findHandler(method, uri);
        if(!handler) {
            LOG_warn("Handler not found -Method=%s, -Uri=%s", method.c_str(), uri.c_str());
            self->sendNotFound(res);
            return ;
        }

        LOG_trace("Handler found: %s", handler->getMappingUri().c_str());

        std::shared_ptr<ServerRequest> request = std::make_shared<ServerRequest>(req, res, handler->getRequestType());
        if(!request->isValid()) {
            LOG_trace("FileServer request:%s is not valid", uri.c_str());
            return ;
        }
        self->onRequest(request, handler);
    };

    if(m_httpThreadNum > 0) {
        http_server_use_threads_pool(m_http, m_httpThreadNum);
    }
    
    console_out("HTTP Server listenning on %s:%d", host.c_str(), port);
    LOG_info("HTTP Server listenned on %s:%d", host.c_str(), port);
    if(!http_server_listen(m_http, host.c_str(), port)) {
        char *errstr;
        http_server_get_errstr(m_http, &errstr);
        console_err("HTTP Server listen on %s:%d error, %s", host.c_str(), port, errstr);
        LOG_error("HTTP Server listen on %s:%d error, %s", host.c_str(), port, errstr);
        free(errstr);
    }
}

void FileServer::close() {
    http_server_close(m_http);
}

void FileServer::useMultiThreads(uint16_t const& threadNum) {
    m_httpThreadNum = threadNum;
}

void FileServer::addHandler(std::shared_ptr<HttpHandler> const& handler) {
    LOG_trace("Add http file api handler: %s", handler->getMappingUri().c_str());
    m_handlerList.push_back(handler);
}


void FileServer::onRequest(std::shared_ptr<ServerRequest> const& httpReq, std::shared_ptr<HttpHandler> handler) {
    if(handler->needAuth() && !httpReq->verifyAuth(m_filter)) {
        LOG_warn("request Auth failed -Method=%s, -URI=%s, -File=%s", httpReq->getMethod().c_str(), 
                httpReq->getUri().c_str(), httpReq->getFilename().c_str());
        httpReq->sendAuthFailed();
        return ;
    }
    handler->handleRequest(httpReq);
}


std::shared_ptr<HttpHandler> FileServer::findHandler(std::string const& method, std::string const& uri) {
    for(const auto& h: m_handlerList) {
        if(h->getMappingMethod() == method && h->getMappingUri() == uri) {
            return h;
        }
    }
    return std::shared_ptr<HttpHandler>();
}

void FileServer::sendNotFound(HttpResponse *res) {
    http_response_set_status(res, 404);
    http_response_set_content_type(res, "application/json");
    const char *body = "{\"success\":false,\"error\":\"404 NotFound\"}";
    http_response_send_response(res, body, strlen(body));
}


void FileServer::sendIcon(HttpResponse *res) {
    http_response_set_status(res, 200);
    http_response_set_content_type(res, "image/icon");
    const char *body = fs::common::ICON_SRC;
    size_t len = fs::common::ICON_SRC_LEN;
    http_response_send_response(res, body, len);
}

