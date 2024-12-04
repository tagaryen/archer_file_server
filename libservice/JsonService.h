#pragma once

#include <libcommon/Common.h>
// #include <libcommon/Log.h>
#include <libcommon/GlobalConfig.h>
#include <libserver/ServerRequest.h>
#include <libdatabase/DataBase.h>

namespace fs 
{
namespace service 
{
class JsonService
{
public:

    JsonService(std::shared_ptr<database::DataBase> const& jsonDB) {
        m_jsonDB = jsonDB;
    }

    ~JsonService() {};

    void getJsonInfo(std::shared_ptr<fs::server::ServerRequest> request);

    void saveJsonInfo(std::shared_ptr<fs::server::ServerRequest> request);

private:

    std::shared_ptr<database::DataBase> m_jsonDB;
};
}
}