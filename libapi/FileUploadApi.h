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

class FileUploadApi : public fs::server::HttpHandler
{
public:
    FileUploadApi(std::shared_ptr<fs::service::FileService> const& fileService) {
        m_service = fileService;
    }
    ~FileUploadApi() {}

    void handleRequest(std::shared_ptr<fs::server::ServerRequest> request) override;

    std::string const& getMappingUri() override {
        return m_uri;
    };
    
    std::string const& getMappingMethod() override {
        return m_method;
    }
    
    bool needAuth() override {return true;}

private:
    std::string   m_uri = FS_API_FILE_UPLOAD;
    std::string   m_method = "POST";
    std::shared_ptr<fs::service::FileService> m_service;
};
}
}