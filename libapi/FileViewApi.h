
#pragma once

#include <libcommon/Common.h>
#include <libcommon/GlobalConfig.h>
#include <libserver/ServerRequest.h>
#include <libhandler/HttpHandler.h>
#include <libservice/FileService.h>

namespace fs 
{
namespace api 
{
class FileViewApi : public fs::server::HttpHandler
{
public:
    FileViewApi(std::shared_ptr<fs::service::FileService> const& fileService) {
        m_service = fileService;
    }
    ~FileViewApi() {}

    void handleRequest(std::shared_ptr<fs::server::ServerRequest> request) override;

    std::string const& getMappingUri() override {
        return m_uri;
    };
    
    std::string const& getMappingMethod() override {
        return m_method;
    }
    
    bool needAuth() override {return true;}

private:
    std::string   m_uri = FS_API_FILE_VIEW;
    std::string   m_method = "GET";
    std::shared_ptr<fs::service::FileService> m_service;
};
}
}