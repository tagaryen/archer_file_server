#include "ServerRequest.h"

using namespace fs::server;


ServerRequest::ServerRequest(HttpRequest *req, HttpResponse *res, RequestType type) {
    m_isValid = true;
    m_req = req;
    m_res = res;
    m_uri = std::string(http_request_get_uri(req));
    m_method = std::string(http_request_get_method(req));
    if(type == FILE_REQUEST) {
        parseFileRequest();
    } else if(type == JSON_REQUEST) {
        parseJsonRequest();
    } else {
        m_isValid = false;
        LOG_warn("request Invalid RequestType");
        sendBadRequest("{\"success\":false,\"error\":\"invalid request\"}");
		return;
    }

    LOG_trace("Request search uri publicKeyHex and signature");
    const char *publicKeyHex = http_request_get_query_param(m_req, "publicKeyHex");
    if(publicKeyHex) {
        m_publicKeyHex = std::string(publicKeyHex);
    }
    const char *signature = http_request_get_query_param(req, "signature");
    if(signature) {
        m_signature = std::string(signature);
    }
}


void ServerRequest::parseFileRequest() {
    const char *filename = http_request_get_query_param(m_req, "filename");
    if(!filename) {
        m_isValid = false;
        LOG_warn("request Can not get a valid filename");
        sendBadRequest("{\"success\":false,\"error\":\"filename not found\"}");
		return;
    }
    m_filename = std::string(filename);

    const char *contentType = http_request_get_content_type(m_req);
    if(!contentType) {
        m_isValid = false;
        LOG_warn("request Can not get a valid content-type");
        sendBadRequest("{\"success\":false,\"error\":\"content-type not found\"}");
		return;
    }
    m_contentType = std::string(contentType);
    m_contentLength = http_request_get_content_length(m_req);
    
    LOG_info("request -METHOD=%s -URL=%s -FILE_NAME=%s", m_method.c_str(), m_uri.c_str(), m_filename.c_str());
}

void ServerRequest::parseJsonRequest() {
    const char *key = http_request_get_query_param(m_req, "key");
    if(!key) {
        m_isValid = false;
        LOG_warn("request Can not get a valid key");
        sendBadRequest("{\"success\":false,\"error\":\"key not found\"}");
		return;
    }
    m_key = std::string(key);
    
    if(m_method == "POST") {
        const char *contentType = http_request_get_content_type(m_req);
        if(!contentType) {
            m_isValid = false;
            LOG_warn("request Can not get a valid content-type");
            sendBadRequest("{\"success\":false,\"error\":\"content-type not found\"}");
            return;
        }
        m_contentType = std::string(contentType);
        m_contentLength = http_request_get_content_length(m_req);
        if(m_contentType.find("application/json") != 0) {
            m_isValid = false;
            LOG_warn("request Requires content-type 'application/json', but got '%s'", contentType);
            sendBadRequest("{\"success\":false,\"error\":\"invalid content-type\"}");
            return;
        }

        char *body = NULL;
        size_t body_size = 0;
        http_request_read_all_body(m_req, &body, &body_size);
        if(!body) {
            m_isValid = false;
            LOG_warn("request Can not get a valid json body");
            sendBadRequest("{\"success\":false,\"error\":\"body not found\"}");
            return;
        }
        m_json = std::string(body, body_size);
        free(body);
    }
    
    LOG_info("request -METHOD=%s -URL=%s -KEY=%s", m_method.c_str(), m_uri.c_str(), m_key.c_str());
}

std::string ServerRequest::getHeader(std::string const& key) {
    const char *val = http_request_get_header(m_req, key.c_str());
    if(val) {
        return std::string(val);
    }
    return std::string();
}

void ServerRequest::sendBadRequest() {
    sendBadRequest("{\"success\":false,\"error\":\"400 BadRequest\"}");
}

void ServerRequest::sendBadRequest(std::string const& reason) {
    doReply(400, reason.c_str());
}

void ServerRequest::sendAuthFailed() {
    doReply(401, "{\"success\":false,\"error\":\"401 NotAuthenticated\"}");
}

void ServerRequest::sendInternalError() {
    doReply(500, "{\"success\":false,\"error\":\"500 InternalError\"}");
}


void ServerRequest::sendOk(std::string const& body) {
    doReply(200, body);
}


void ServerRequest::doReply(int code, std::string const& body) {
    http_response_set_status(m_res, code);
    http_response_set_content_type(m_res, "application/json");
    http_response_send_response(m_res, body.c_str(), body.length());
}


void ServerRequest::setResponseHeader(const char *key, const char *value) {
    http_response_set_header(m_res, key, value);
}


void ServerRequest::beginResponse() {
    http_response_set_status(m_res, 200);
    http_response_send_head(m_res);
}

void ServerRequest::sendBytes(const char *data, size_t dataLen) {
    http_response_send_some(m_res, data, dataLen);
}

void ServerRequest::endResponse() {
    http_response_send_end(m_res);
}


void ServerRequest::readBytes(char **data, size_t *dataLen) {
    http_request_read_some(m_req, data, dataLen);
}

bool ServerRequest::verifyAuth(std::shared_ptr<SignatureFilter> const& filter) {
    if(m_signature.length() != 128) {
        return false;
    }
    if(filter->find(m_signature)) {
        return false;
    }
    if(m_publicKeyHex.length() != 128 && m_publicKeyHex.length() != 130) {
        return false;
    }
    if(m_publicKeyHex.length() == 130 && (m_publicKeyHex[0] != '0' || m_publicKeyHex[1] != '4')) {
        return false;
    } else {
        m_publicKeyHex = m_publicKeyHex.substr(2);
    }

    EcPublicKey *key = common::getPublicKeyFromHex(m_publicKeyHex);
    if(!key) {
        return false;
    }
    EcSignature *sig = common::getSignatureFromHex(m_signature);
    if(!sig) {
        free(key);
        return false;
    }
    std::string m;
    if(m_type == FILE_REQUEST) {
        m = m_uri + m_filename;
    } else {
        m = m_uri + m_key;
    }
    bool ret = sm2p256v1_verify(key, (uint8_t *)m.c_str(), m.length(), sig) == 1;
    if(ret) {
        filter->save(m_signature);
    }
    free(key);
    free(sig);

    return ret;
}





