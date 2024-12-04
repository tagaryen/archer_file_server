#include "FileUploadOpenApi.h"
#include <libservice/FileService.h>

using namespace fs::api;


void FileUploadOpenApi::handleRequest(std::shared_ptr<fs::server::ServerRequest> request) {
    std::string filename = request->getFilename();
    std::string updateTime = common::getNowTime();
    std::string contentType = request->getContentType();

    std::shared_ptr<fs::database::FileInfo> fileMessage = 
            std::make_shared<fs::database::FileInfo>(filename, "", updateTime, contentType);
    
    m_service->handleFileUpoadMessage(request, fileMessage);
}