#pragma once

// #include <libcommon/Log.h>
#include <libcommon/GlobalConfig.h>
#include <libapi/FileDownloadApi.h>
#include <libapi/FileDownloadOpenApi.h>
#include <libapi/FileViewApi.h>
#include <libapi/FileViewOpenApi.h>
#include <libapi/FileUploadApi.h>
#include <libapi/FileUploadOpenApi.h>
#include <libapi/JsonGetApi.h>
#include <libapi/JsonGetOpenApi.h>
#include <libapi/JsonSaveApi.h>
#include <libapi/JsonSaveOpenApi.h>
#include <libdatabase/DataBase.h>
#include <libservice/FileService.h>
#include <libservice/JsonService.h>
#include <libhandler/RPCHandler.h>
#include <libserver/FileServer.h>
#include <libserver/RPCServer.h>


namespace fs
{
namespace initializer 
{

void startRPCServer(std::shared_ptr<fs::database::DataBase> const& database);

void startHTTPServer(std::shared_ptr<fs::database::DataBase> const& database);

void startAll();

}
}