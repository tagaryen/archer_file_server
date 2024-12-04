#include "GlobalConfig.h"

using namespace fs::common;

// static GlobalConfig *globalConfigInstance = static_cast<GlobalConfig *>(std::malloc(sizeof(GlobalConfig)));


static GlobalConfig globalConfigInstance = GlobalConfig();

GlobalConfig* GlobalConfig::instance() {
    return &globalConfigInstance;
}

void GlobalConfig::parseConfig(std::string const & configPath) {
    m_curPath = fs::common::getCurrentPath();
    std::string abConfigPath = configPath;
    if(!fs::common::isAbsolutePath(configPath)) {
        abConfigPath = m_curPath + "/" + configPath;
    }
    if(!fs::common::fileExists(abConfigPath)) {
        console_warn("Can not read %s, file not exists, using default configs", abConfigPath.c_str());
        m_logPath = "logs";
        m_logLevel = LOG_LEVEL_INFO;
        m_filePath = "/opt/archer-file/static/";
        m_httpServerAddress = "127.0.0.1";
        m_httpServerPort = 9617;
        m_httpThreadNum = 0;
        m_rpcServerAddress = "0.0.0.0";
        m_rpcServerPort = 9611;
        m_rpcThreadNum = 0;
        m_dbPath = "/opt/archer-file/database/";
        m_dbReaders = 4;
        m_dbMemory = 1024 * 1024 * 128;
        

        if(!fs::common::fileExists(m_filePath)) {
            fs::common::createDirectories(m_filePath);
        }
        if(!fs::common::fileExists(m_dbPath)) {
            fs::common::createDirectories(m_dbPath);
        }
        m_logger = logger_new_with_path_level(m_logPath.c_str(), m_logLevel);
        
        console_out("Log path = %s", m_logPath.c_str());
        console_out("Log level = INFO");
        console_out("File path = %s", m_filePath.c_str());
        console_out("Database path = %s", m_dbPath.c_str());
        console_out("Database readers = %d", m_dbReaders);
        console_out("Database memory size = %u", m_dbMemory);
        console_out("HTTP Server host = %s", m_httpServerAddress.c_str());
        console_out("HTTP Server port = %d", m_httpServerPort);
        console_out("HTTP Server use multithreads = %s", (m_httpThreadNum > 0 ? "true":"false"));
        console_out("RPC Server host = %s", m_rpcServerAddress.c_str());
        console_out("RPC Server port = %d", m_rpcServerPort);
        console_out("RPC Server use multithreads = %s", (m_rpcThreadNum > 0 ? "true":"false"));

        return ;
    }
    
    console_out("Using config file %s", configPath.c_str());

    std::ifstream file(configPath);
    if(!file.is_open()) {
        console_err("Can not open file %s", configPath.c_str());
        exit(0);
    }
    m_root = Json::Value(Json::stringValue);
    try {
        file >> m_root;
    } catch(std::exception& ex) {
        console_err("Parse json file %s failed, due to %s", configPath.c_str(), ex.what());
        file.close();
        exit(0);
    }
    file.close();

    console_out("Parse log configs");
    std::string level;
    if(m_root.isMember("log")) {
        if(m_root["log"].isMember("path")) {
            m_logPath = std::string(m_root["log"]["path"].asCString());
        } else {
            m_logPath = "logs";
        }

        if(m_root["log"].isMember("level")) {
            level = std::string(m_root["log"]["level"].asCString());
            if(level == "TRACE") {
                m_logLevel = LOG_LEVEL_TRACE;
            } else if(level == "DEBUG") {
                m_logLevel = LOG_LEVEL_DEBUG;
            } else if(level == "INFO") {
                m_logLevel = LOG_LEVEL_INFO;
            } else if(level == "WARN") {
                m_logLevel = LOG_LEVEL_WARN;
            } else if(level == "ERROR") {
                m_logLevel = LOG_LEVEL_ERROR;
            } else if(level == "FATAL") {
                m_logLevel = LOG_LEVEL_FATAL;
            } else {
                level = "INFO";
                m_logLevel = LOG_LEVEL_INFO;
            }
        } else {
            m_logLevel = LOG_LEVEL_INFO;
        }
    } else {
        m_logPath = "logs";
        m_logLevel = LOG_LEVEL_INFO;
    }
    
    m_logger = logger_new_with_path_level(m_logPath.c_str(), m_logLevel);
    
    console_out("Log path = %s", m_logPath.c_str());
    console_out("Log level = %s", level.c_str());

    console_out("Parse file configs");
    if(m_root.isMember("file") && m_root["file"].isMember("path")) {
        m_filePath = std::string(m_root["file"]["path"].asCString());
        if(!fs::common::isAbsolutePath(m_filePath)) {
            if(m_filePath[0] == '/' || m_filePath[0] == '\\') {
                m_filePath = m_curPath + m_filePath;
            } else {
                m_filePath = m_curPath + "/" + m_filePath;
            }
            if(m_filePath[m_filePath.length()-1] != '/' && m_filePath[m_filePath.length()-1] != '\\') {
                m_filePath += '/';
            }
        }
    } else {
        m_filePath = m_curPath + "/data/";
    }
    console_out("File path = %s", m_filePath.c_str());


    console_out("Parse database configs");
    if(m_root.isMember("database") && m_root["database"].isMember("path")) {
        m_dbPath = std::string(m_root["database"]["path"].asCString());
        if(!fs::common::isAbsolutePath(m_dbPath)) {
            if(m_dbPath[0] == '/' || m_dbPath[0] == '\\') {
                m_dbPath = m_curPath + m_dbPath;
            } else {
                m_dbPath = m_curPath + "/" + m_dbPath;
            }
            if(m_dbPath[m_dbPath.length()-1] != '/' && m_dbPath[m_dbPath.length()-1] != '\\') {
                m_dbPath += '/';
            }
        }
    } else {
        m_dbPath = m_curPath + "/database/";
    }
    if(m_root.isMember("database") && m_root["database"].isMember("readers")) {
        int readers = m_root["database"]["readers"].asInt();
        if(readers < 0 && readers > 1024) {
            m_dbReaders = 4;
        } else {
            m_dbReaders = readers;
        }
    } else {
        m_dbReaders = 4;
    }
    if(m_root.isMember("database") && m_root["database"].isMember("memory")) {
        m_dbMemory = m_root["database"]["memory"].asUInt();
    } else {
        m_dbMemory = 1024 * 1024 * 128;
    }

    console_out("Database path = %s", m_dbPath.c_str());
    console_out("Database readers = %d", m_dbReaders);
    console_out("Database memory size = %u", m_dbMemory);

    console_out("Parse http server configs");
    if(m_root.isMember("http") && m_root["http"].isMember("host")) {
        m_httpServerAddress = std::string(m_root["http"]["host"].asCString());
    } else {
        m_httpServerAddress = "127.0.0.1";
    }
    if(m_root.isMember("http") && m_root["http"].isMember("port")) {
        uint32_t port = m_root["http"]["port"].asUInt();
        if(port > 65535) {
            m_httpServerPort = 9607;
        } else {
            m_httpServerPort = port;
        }
    } else {
        m_httpServerPort = 9607;
    }
    bool useHTTPMultithreads = false;
    if(m_root.isMember("http") && m_root["http"].isMember("multithread")) {
        useHTTPMultithreads = m_root["http"]["multithread"].asBool();
    }
    if(useHTTPMultithreads && m_root.isMember("http") && m_root["http"].isMember("threadNum")) {
        uint32_t threadNum = m_root["http"]["threadNum"].asUInt();
        if(threadNum < 1024) {
            m_httpThreadNum = threadNum;
        } else {
            m_httpThreadNum = 0;
        }
    } else {
        m_httpThreadNum = 0;
    }
    console_out("HTTP Server host = %s", m_httpServerAddress.c_str());
    console_out("HTTP Server port = %d", m_httpServerPort);
    console_out("HTTP Server use multithreads = %s", (m_httpThreadNum > 0 ? "true":"false"));


    console_out("Parse rpc server configs");
    if(m_root.isMember("rpc") && m_root["rpc"].isMember("host")) {
        m_rpcServerAddress = std::string(m_root["rpc"]["host"].asCString());
    } else {
        m_rpcServerAddress = "127.0.0.1";
    }
    if(m_root.isMember("rpc") && m_root["rpc"].isMember("port")) {
        uint32_t port = m_root["rpc"]["port"].asUInt();
        if(port > 65535) {
            m_rpcServerPort = 9607;
        } else {
            m_rpcServerPort = port;
        }
    } else {
        m_rpcServerPort = 9607;
    }
    bool useRPCMultithreads = false;
    if(m_root.isMember("rpc") && m_root["rpc"].isMember("multithread")) {
        useRPCMultithreads = m_root["rpc"]["multithread"].asBool();
    }
    if(useRPCMultithreads && m_root.isMember("rpc") && m_root["rpc"].isMember("threadNum")) {
        uint32_t threadNum = m_root["rpc"]["threadNum"].asUInt();
        if(threadNum < 1024) {
            m_rpcThreadNum = threadNum;
        } else {
            m_rpcThreadNum = 0;
        }
    } else {
        m_rpcThreadNum = 0;
    }
    console_out("RPC Server host = %s", m_rpcServerAddress.c_str());
    console_out("RPC Server port = %d", m_rpcServerPort);
    console_out("RPC Server use multithreads = %s", (m_rpcThreadNum > 0 ? "true":"false"));

    if(!fs::common::fileExists(m_filePath)) {
        fs::common::createDirectories(m_filePath);
    }
    if(!fs::common::fileExists(m_dbPath)) {
        fs::common::createDirectories(m_dbPath);
    }
}

Json::Value const& GlobalConfig::fetchConfig(std::string const & key) {
    return m_root[key];
}

