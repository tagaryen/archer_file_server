#include "FileService.h"

#include <stdio.h>
#include <sys/file.h>
#include <sstream>  
#include <regex>  

using namespace fs::service;


#define KEY_START    1
#define VAL_START    2
#define CHUNKED_LEN  3
#define CHUNKED_VAL  4

static std::string fileChunked = "chunked";
static const int fileHexMap[] = {127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 127, 127, 127, 127, 127, 127, 127, 10, 11, 12, 13, 14, 15, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 10, 11, 12, 13, 14, 15, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127};
static std::string multipartHead = "multipart/form-data; boundary=";

struct MultiPartPos
{
    size_t start;
    size_t end;
};


static MultiPartPos matchContentTypeAndGetOffset(std::string const& tmp, std::string const& bound, std::shared_ptr<fs::database::FileInfo> const& fileInfo) {
    MultiPartPos pos;
    pos.start = 0;
    pos.end = tmp.length();
    size_t off;
    if(tmp.find(bound, 0) == 0) {
        off = tmp.find("\r\n\r\n", bound.length()+2);
        if(off == std::string::npos) {
            pos.start = off;
            LOG_error("Upload file multipart parse error, '%s'", tmp);
            return pos;
        }
        std::string part = tmp.substr(bound.length() + 2, off);
        std::string type = "application/none";
        std::smatch match; 
        std::regex type_regex(R"(Content-Type:\s*([^\r\n]+))", std::regex_constants::icase);  
        if (std::regex_search(part, match, type_regex)) {  
            type = match[1].str();  
        } 
        fileInfo->setContentType(type);
        pos.start = off + 4;
    }
    off = tmp.find(bound + "--", 0);
    if(off != std::string::npos) {
        pos.end = off - 2;
    }
    return pos;
}

static int bytesToInt(char *data, int from, int to) {
    int ret = 0;
    for(int i = from; i < to; i++) {
        int v = fileHexMap[data[i]];
        if(v > 0) {
            ret = ret * 16 + v - 1;
        }
    }
    return ret;
}

std::shared_ptr<fs::database::FileInfo> FileService::getFileInfo(std::string const & filename, std::string const& publicKeyHex) {
    std::string name;
    LOG_trace("Get fileinfo path = %s", filename.c_str());
    if(publicKeyHex.empty()) {
        name = filename;
    } else {
        name = filename + publicKeyHex;
    }
    Hash32 hash;
    sm3((uint8_t *)name.c_str(), name.length(), &hash);
    std::string key = fs::common::getHexFromUint8s(hash.h, 32);
    LOG_trace("Get fileinfo key = %s", key.c_str());
    return m_fileDB->getFileInfo(key);
}


void FileService::handleFileListMessage(std::shared_ptr<server::ServerRequest> const& request, int pageNum) {
    std::shared_ptr<fs::database::Page> page = m_fileDB->listFile(request->getPublicKeyHex(), pageNum);
    std::string resultJson = "{\"total\":" + std::to_string(page->getTotal());
    resultJson += ",\"pageNum\":" + std::to_string(page->getPageNum());
    resultJson += ",\"files\":[";
    std::string size;
    for(auto f: page->getFile()) {
        size = std::to_string(f.getSize());
        resultJson += "{\"filename\":\"" + f.getRealName() + "\",\"time\":\""+f.getUpdateTime()+"\",\"size\":"+size+"},";
    }
    if(page->getFile().size() > 0) {
        resultJson = resultJson.substr(0, resultJson.length() - 1);
    }
    resultJson += "]}";
    request->sendOk("{\"success\":true, \"data\":" + resultJson + "}");
}

void FileService::handleFileUpoadMessage(std::shared_ptr<fs::server::ServerRequest> const& request, std::shared_ptr<fs::database::FileInfo> const& fileInfo) {
    std::string filename = request->getFilename();

    std::string name = fileInfo->getName();
    std::string path = fs::common::GlobalConfig::instance()->fetchFilePath();

    LOG_info("Start saving file %s", filename.c_str());
    path += name;


    std::string contentType = request->getContentType();
    if(contentType.find(multipartHead, 0) == std::string::npos) {
        request->sendBadRequest();
        return ;
    }
    std::string bound = "--"+contentType.substr(multipartHead.length());
    std::string encoding = request->getHeader("Transfer-Encoding");
    uint32_t contentLength = request->getContentLength();

    FILE *file = fopen(path.c_str(), "wb");
    if(!file) {
        LOG_error("Can not open file %s", path.c_str());
        return ;
    }
    int fd = fileno(file);

    bool ok = true;
    size_t bufLen = 0, totalLen = 0;
    char buf[128 * 1024];

#ifdef _WIN32
    if(__lock_fhandle(fd) < 0) {
        LOG_trace("Try to lock file %s", path.c_str());
#else
    if (flock(fd, LOCK_EX) < 0) {
#endif
        LOG_error("Can not lock file %s", path.c_str());
        fclose(file);
        request->sendInternalError();
        return ;
    }
    std::string tmp;
    struct MultiPartPos pos;
    while (true) {
        bufLen = request->readBytes(buf, 128 * 1024);
        if(bufLen <= 0) {
            break;
        }
        LOG_trace("Upload file transfer-encoding=%s, content-length=%lu", encoding.c_str(), contentLength);
        if(fileChunked == encoding) {
			size_t s = 0, len = 0;
            for(size_t i = 0; i < bufLen; i++) {
                if(buf[i] == '\n') {
                    if(i == bufLen - 1) {
                        break;
                    }
                    len = bytesToInt(buf, s, i - 1);
                    if(len == 0) {
                        break;
                    } else {
                        if(len + i + 1 > bufLen) {
                            tmp = std::string(buf+s, bufLen);
                            pos = matchContentTypeAndGetOffset(tmp, bound, fileInfo);
                            if(pos.start == std::string::npos) {
                                ok = false;
                                break;
                            }
                            totalLen += pos.end - pos.start;
                            fwrite(tmp.c_str() + pos.start, sizeof(char), pos.end - pos.start, file);
                            break;
                        } else {
                            tmp = std::string(buf+(i+1), len);
                            pos = matchContentTypeAndGetOffset(tmp, bound, fileInfo);
                            if(pos.start == std::string::npos) {
                                ok = false;
                                break;
                            }
                            totalLen += pos.end - pos.start;
                            fwrite(tmp.c_str() + pos.start, sizeof(char), pos.end - pos.start, file);
                            
                        }
                        i += 1 + len;
                    }
                    s = i + 1;
                }
			}
        } else if(contentLength > 0) {
            tmp = std::string(buf, bufLen);
            pos = matchContentTypeAndGetOffset(tmp, bound, fileInfo);
            if(pos.start == std::string::npos) {
                ok = false;
                break;
            }
            totalLen += pos.end - pos.start;
            fwrite(tmp.c_str() + pos.start, sizeof(char), pos.end - pos.start, file);
        } else {
            ok = false;
            break;
        }
        bufLen = 0;
	}
#ifdef _WIN32
    _unlock_fhandle(fd);
#else
    flock(fd, LOCK_UN);
#endif
    LOG_trace("Try to unlock file %s", path.c_str());
    fclose(file);
    if(!ok) {
        request->sendBadRequest();
        return ;
    }
    fileInfo->setSize(totalLen);
    if(m_fileDB->saveFileInfo(fileInfo)) {
        remove(path.c_str());
        request->sendInternalError();
        return ;
    }
    request->sendOk("{\"success\":true, \"file\":\"" + filename + "\"}");
}


void FileService::handleFileDownoadMessage(std::shared_ptr<fs::server::ServerRequest> const& request, std::shared_ptr<fs::database::FileInfo> const& fileInfo) {
    std::string filename = fileInfo->getRealName();
    std::string attach = "attachment; filename=" + filename;
    request->setResponseHeader("Content-Type", "application/octet-stream");
    request->setResponseHeader("Content-Disposition", attach.c_str());
    
    LOG_info("filedownload Reading file %s", request->getFilename().c_str());

    handleFileOuputMessage(request, fileInfo);
}

void FileService::handleFileViewMessage(std::shared_ptr<fs::server::ServerRequest> const& request, std::shared_ptr<fs::database::FileInfo> const& fileInfo) {
    std::string contentType = fileInfo->getContentType();
    LOG_info("fileview Reading file %s", request->getFilename().c_str());

    request->setResponseHeader("Content-Type", contentType.c_str());
    handleFileOuputMessage(request, fileInfo);
}

void FileService::handleFileOuputMessage(std::shared_ptr<fs::server::ServerRequest> const& request, std::shared_ptr<fs::database::FileInfo> const& fileInfo) {
    std::string name = fileInfo->getName();
    std::string filename = fileInfo->getRealName();

    std::string path = fs::common::GlobalConfig::instance()->fetchFilePath();
    path += name;
    if(!fs::common::fileExists(path)) {
        LOG_error("fileouput Can not found %s", path.c_str());
        request->sendOk("{\"error\":\"file not found\"}");
        return ;
    }

    FILE *file = fopen(path.c_str(), "rb");
    if(!file) {
        LOG_error("Can not open file %s", path.c_str());
        return ;
    }
    
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);  

    int fd = fileno(file);

#ifdef _WIN32
    if(__lock_fhandle(fd) < 0) {
        LOG_trace("Try to lock file %s", path.c_str());
#else
    if (flock(fd, LOCK_SH) < 0) {
#endif
        LOG_error("Can not lock file %s", path.c_str());
        fclose(file);
        request->sendInternalError();
        return ;
    }
    request->setResponseHeader("Content-Length", std::to_string(fileSize).c_str());

    LOG_info("fileouput Start to transport file %s", filename.c_str());
    request->beginResponse();
    char buf[1024 * 1024];
    size_t readBytes = 0;
    while((readBytes = fread(buf, sizeof(char), 1024 * 1024, file)) > 0) {
        request->sendBytes(buf, readBytes);
    }
#ifdef _WIN32
    _unlock_fhandle(fd);
#else
    flock(fd, LOCK_UN);
#endif
    fclose(file);
    request->endResponse();
}


