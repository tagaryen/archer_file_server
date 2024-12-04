#include "FileViewOpenApi.h"
#include <libservice/FileService.h>

using namespace fs::api;


void FileViewOpenApi::handleRequest(std::shared_ptr<fs::server::ServerRequest> request) {
    std::string filename = request->getFilename();
    
    LOG_trace("Handler %s handle", m_uri.c_str());
    std::shared_ptr<fs::database::FileInfo> fileInfo = 
        m_service->getFileInfo(filename, request->getPublicKeyHex());
    if(!fileInfo || !fileInfo->isValid()) {
        request->sendOk("{\"success\":false,\"error\":\"file not found\"}");
        return ;
    }

    m_service->handleFileViewMessage(request, fileInfo);
}