#pragma once

#include "archer_alg.h"
#include "archer_net.h"


#include "SignatureFilter.h"
#include <libcommon/Common.h>
#include <libcommon/GlobalConfig.h>

namespace fs
{
namespace server
{
enum RequestType {
    FILE_REQUEST,
    JSON_REQUEST
};
class ServerRequest
{
public:
    ServerRequest(HttpRequest *req, HttpResponse *res, RequestType type);
    ~ServerRequest(){};

    void sendBadRequest();
    void sendBadRequest(std::string const& reason);
    void sendAuthFailed();
    void sendInternalError();
    void sendOk(std::string const& body);
    void beginResponse();
    void sendBytes(const char *data, size_t dataLen);
    void endResponse();
    void readBytes(char **data, size_t *dataLen);
    bool verifyAuth(std::shared_ptr<SignatureFilter> const& filter);

    bool isValid() {return m_isValid;}
    std::string const& getMethod() {return m_method;}
    std::string const& getUri() {return m_uri;}
    uint32_t getContentLength() {return m_contentLength;}
    std::string const& getContentType() {return m_contentType;}
    std::string const& getFilename() {return m_filename;}
    std::string const& getKey() {return m_key;}
    std::string const& getJson() {return m_json;}
    std::string const& getPublicKeyHex() {return m_publicKeyHex;}
    std::string const& getSignature() {return m_signature;}
    std::string getHeader(std::string const& key);
    void setResponseHeader(const char *key, const char *value);
    
    int getRequestType() {return m_type;}

    

private:
    void doReply(int code, std::string const& body);

    void parseFileRequest();

    void parseJsonRequest();

private:
    bool                            m_isValid;
    std::string                     m_method;
    std::string                     m_uri;
    uint32_t                        m_contentLength;
    std::string                     m_contentType;

    std::string                     m_filename;

    std::string                     m_key;
    std::string                     m_json;

    std::string                     m_publicKeyHex;
    std::string                     m_signature;

    RequestType                     m_type;

    HttpRequest                    *m_req;
    HttpResponse                   *m_res;
};

}
}