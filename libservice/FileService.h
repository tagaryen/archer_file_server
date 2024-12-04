#pragma once

#include <libcommon/Common.h>
#include <libcommon/GlobalConfig.h>
#include <libserver/ServerRequest.h>
#include <libdatabase/DataBase.h>

namespace fs 
{
namespace service 
{
class FileService
{
public:

    FileService(std::shared_ptr<database::DataBase> const& fileDB) {
        m_fileDB = fileDB;
    }

    ~FileService() {};

    std::shared_ptr<fs::database::FileInfo> getFileInfo(std::string const & filename, std::string const& signatire);

    void handleFileUpoadMessage(std::shared_ptr<server::ServerRequest> const& request, std::shared_ptr<database::FileInfo> const& message);

    void handleFileDownoadMessage(std::shared_ptr<server::ServerRequest> const& request, std::shared_ptr<database::FileInfo> const& message);

    void handleFileViewMessage(std::shared_ptr<server::ServerRequest> const& request, std::shared_ptr<database::FileInfo> const& message);

private:
    void handleFileOuputMessage(std::shared_ptr<fs::server::ServerRequest> const& request, std::shared_ptr<fs::database::FileInfo> const& message);

    std::shared_ptr<database::DataBase> m_fileDB;
};
}
}