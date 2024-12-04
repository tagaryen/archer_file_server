#pragma once

#include <libcommon/Common.h>
#include <libcommon/GlobalConfig.h>
#include "lmdb.h"

#include <mutex>
#include <array>

namespace fs 
{
namespace database 
{

class FileInfo 
{
public:
    FileInfo() {m_valid = false;}
    FileInfo(std::string const& realName, std::string const& publicKeyHex, std::string const& updateTime, std::string const& contentType);
    ~FileInfo(){}

    bool isValid() {return m_valid;}
    std::string const& getName() {return m_name;}
    std::string const& getRealName() {return m_realName;}
    std::string const& getPublicKeyHex() {return m_publicKeyHex;}
    std::string const& getUpdateTime() {return m_updateTime;}
    std::string const& getContentType() {return m_contentType;}

    std::string const& encode() {return m_encode;};
    void decode(const char *data, size_t data_len);

private:
    bool             m_valid;
    std::string      m_name;
    std::string      m_realName;
    std::string      m_publicKeyHex;
    std::string      m_updateTime;
    std::string      m_contentType;

    std::string      m_encode;
};

class DataBase 
{

public:
    DataBase(std::string const& dbPath, unsigned int readerNum, size_t maxMemorySize);
    
    ~DataBase() {
        mdb_env_close(m_env);
    }

    int saveFileInfo(std::shared_ptr<FileInfo> const& fileMessage);

    std::shared_ptr<FileInfo> getFileInfo(std::string const & name);

    int saveJsonInfo(std::string const& key, std::string const& val);

    std::string getJsonInfo(std::string const& key);

private:

    bool doError(int rc) {
        if(rc) {
            char *msg = mdb_strerror(rc);
            console_err("%s", msg);
            LOG_error("%s", msg);
        }
        return rc;
    }

    std::string findKv(std::string const& key);
    bool saveKv(std::string const& key, std::string const& val);

    std::map<std::string, std::string>   m_kvRLU;
    std::mutex                           m_kvMutex;

    MDB_env    *m_env;
    MDB_dbi     m_dbi;
};

}

}
