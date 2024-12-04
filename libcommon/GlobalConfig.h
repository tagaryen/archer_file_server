#pragma once

#include <fstream>
#include <json/json.h>

#include "Common.h"
// #include "Log.h"
#include "Logger.h"


namespace fs 
{
namespace common 
{

class GlobalConfig 
{
public:

    // static GlobalConfig* instance();
    
    static GlobalConfig* instance();

    GlobalConfig() {};
    ~GlobalConfig() {};

    void parseConfig(std::string const & configPath);

    Json::Value const& fetchConfig(std::string const & key);
    
    std::string const& fetchCurPath()  {return m_curPath;}

    std::string const& fetchLogPath()  {return m_logPath;}
    
    uint16_t fetchLogLevel()  {return m_logLevel;}

    std::string const& fetchFilePath() {return m_filePath;}
    
    std::string const& fetchDatabasePath()  {return m_dbPath;}
    
    uint16_t fetchDatabaseReaders()  {return m_dbReaders;}
    
    uint32_t fetchDatabaseMemory()  {return m_dbMemory;}

    std::string const& fetchHttpServerAddress()  {return m_httpServerAddress;}
    
    uint16_t fetchHTTPServerPort()  {return m_httpServerPort;}
    
    uint16_t fetchHTTPThreadNum()  {return m_httpThreadNum;}
    
    std::string const& fetchRPCServerAddress()  {return m_rpcServerAddress;}
    
    uint16_t fetchRPCServerPort()  {return m_rpcServerPort;}
    
    uint16_t fetchRPCThreadNum()  {return m_rpcThreadNum;}

    Logger * getLogger() {
        return m_logger;
    }

private:
    std::string m_curPath;
    std::string m_logPath;
    uint16_t    m_logLevel;
    std::string m_filePath;
    std::string m_dbPath;
    uint16_t    m_dbReaders;
    uint32_t    m_dbMemory;
    std::string m_httpServerAddress;
    uint16_t    m_httpServerPort;
    uint16_t    m_httpThreadNum;
    std::string m_rpcServerAddress;
    uint16_t    m_rpcServerPort;
    uint16_t    m_rpcThreadNum;
    Json::Value m_root;
    Logger     *m_logger;
};
}
}


#define LOG_trace(...) logger_log(fs::common::GlobalConfig::instance()->getLogger(), LOG_LEVEL_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_debug(...) logger_log(fs::common::GlobalConfig::instance()->getLogger(), LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_info(...)  logger_log(fs::common::GlobalConfig::instance()->getLogger(), LOG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_warn(...)  logger_log(fs::common::GlobalConfig::instance()->getLogger(), LOG_LEVEL_WARN, __FILE__, __LINE__,  __VA_ARGS__)
#define LOG_error(...) logger_log(fs::common::GlobalConfig::instance()->getLogger(), LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_fatal(...) logger_log(fs::common::GlobalConfig::instance()->getLogger(), LOG_LEVEL_FATAL, __FILE__, __LINE__, __VA_ARGS__)
