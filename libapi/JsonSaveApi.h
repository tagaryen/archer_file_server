
#pragma once

#include <libcommon/Common.h>
#include <libcommon/GlobalConfig.h>
#include <libserver/ServerRequest.h>
#include <libhandler/HttpHandler.h>
#include <libservice/JsonService.h>

namespace fs 
{
namespace api 
{
class JsonSaveApi : public fs::server::HttpHandler
{
public:
    JsonSaveApi(std::shared_ptr<fs::service::JsonService> const& fileService) {
        m_service = fileService;
    }
    ~JsonSaveApi() {}

    void handleRequest(std::shared_ptr<fs::server::ServerRequest> request) override {
        m_service->saveJsonInfo(request);
    }

    std::string const& getMappingUri() override {
        return m_uri;
    };
    
    std::string const& getMappingMethod() override {
        return m_method;
    }
    
    bool needAuth() override {return false;}

private:
    std::string   m_uri = FS_API_JSON;
    std::string   m_method = "POST";
    std::shared_ptr<fs::service::JsonService> m_service;
};
}
}