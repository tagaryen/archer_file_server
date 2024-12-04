#pragma once

#include <functional>
#include <cstdlib>
#include <ctime>
#include <queue>
#include <string>
#include <memory>
#include <vector>

#include "archer_alg.h"

#pragma warning(push)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma warning(pop)
#pragma GCC diagnostic pop

#define FS_API_VERSION                    "v1"


#define FS_ICON_API                       "/favicon.ico"

#define FS_OPEN_API                       "/fs/open-api/"
#define FS_OPEN_API_FILE_UPLOAD           "/fs/open-api/v1/upload"
#define FS_OPEN_API_FILE_VIEW             "/fs/open-api/v1/view"
#define FS_OPEN_API_FILE_DOWNLOAD         "/fs/open-api/v1/download"


#define FS_API                            "/fs/api/"
#define FS_API_FILE_UPLOAD                "/fs/api/v1/upload"
#define FS_API_FILE_VIEW                  "/fs/api/v1/view"
#define FS_API_FILE_DOWNLOAD              "/fs/api/v1/download"


#define FS_OPEN_API_JSON                  "/fs/open-api/v1/json"
#define FS_API_JSON                       "/fs/api/v1/json"

#define MAX_FILE_NAME_LEN                 512    
#define TIME_FORMAT_LEN                   20                    

#ifndef HTTP_AUTHFAILED
#define HTTP_AUTHFAILED                   401
#endif

namespace fs 
{
namespace common 
{
bool isAbsolutePath(std::string const& path);
void createDirectories(std::string const& path);
std::string getCurrentPath();
bool fileExists(const std::string& filename);
std::string getNowTime();
int getUint8sFromHex(std::string const& hex, uint8_t* bytes);
std::string getHexFromUint8s(const uint8_t* bytes, size_t bytes_len);
EcPublicKey * getPublicKeyFromHex(std::string const& hex);
EcSignature * getSignatureFromHex(std::string const& hex);
}
}

