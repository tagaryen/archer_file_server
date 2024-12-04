#include "FileUploadApi.h"
#include <libservice/FileService.h>

using namespace fs::api;


void FileUploadApi::handleRequest(std::shared_ptr<fs::server::ServerRequest> request) {
    std::string filename = request->getFilename();
    std::string publicKeyHex = request->getPublicKeyHex();
    std::string updateTime = common::getNowTime();
    std::string contentType = request->getContentType();

    std::shared_ptr<fs::database::FileInfo> fileMessage = 
            std::make_shared<fs::database::FileInfo>(filename, publicKeyHex, updateTime, contentType);

    m_service->handleFileUpoadMessage(request, fileMessage);
}