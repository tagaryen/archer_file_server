
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
class JsonGetApi : public fs::server::HttpHandler
{
public:
    JsonGetApi(std::shared_ptr<fs::service::JsonService> const& fileService) {
        m_service = fileService;
    }
    ~JsonGetApi() {}

    void handleRequest(std::shared_ptr<fs::server::ServerRequest> request) override {
        m_service->getJsonInfo(request);
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
    std::string   m_method = "GET";
    std::shared_ptr<fs::service::JsonService> m_service;
};
}
}