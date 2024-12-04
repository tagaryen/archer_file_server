#include "Initializer.h"


void fs::initializer::startRPCServer(std::shared_ptr<fs::database::DataBase> const& database) {
    fs::common::GlobalConfig *config = fs::common::GlobalConfig::instance();
    
    std::string address = config->fetchRPCServerAddress();
    uint16_t port = config->fetchRPCServerPort();
    uint16_t threadNum = config->fetchRPCThreadNum();

    fs::server::RPCServer server;
    server.useMultiThreads(threadNum);

    std::shared_ptr<fs::handler::RPCHandler> handler = std::make_shared<fs::handler::RPCHandler>(database);
    server.setHandler(handler);

    server.listen(address, port);
}

void fs::initializer::startHTTPServer(std::shared_ptr<fs::database::DataBase> const& database) {
    fs::common::GlobalConfig *config = fs::common::GlobalConfig::instance();

    std::string address = config->fetchHttpServerAddress();
    uint16_t port = config->fetchHTTPServerPort();
    uint16_t threadNum = config->fetchHTTPThreadNum();

    fs::server::FileServer server;
    server.useMultiThreads(threadNum);

    std::shared_ptr<fs::service::FileService> fileService = std::make_shared<fs::service::FileService>(database);
    std::shared_ptr<fs::service::JsonService> jsonService = std::make_shared<fs::service::JsonService>(database);
    
    std::shared_ptr<fs::api::FileDownloadApi>     downloadHandler = std::make_shared<fs::api::FileDownloadApi>(fileService);
    std::shared_ptr<fs::api::FileDownloadOpenApi> downloadOpenHandler = std::make_shared<fs::api::FileDownloadOpenApi>(fileService);
    std::shared_ptr<fs::api::FileUploadApi>       uploadHandler = std::make_shared<fs::api::FileUploadApi>(fileService);
    std::shared_ptr<fs::api::FileUploadOpenApi>   uploadOpenHandler = std::make_shared<fs::api::FileUploadOpenApi>(fileService);
    std::shared_ptr<fs::api::FileViewApi>         viewHandler = std::make_shared<fs::api::FileViewApi>(fileService);
    std::shared_ptr<fs::api::FileViewOpenApi>     viewOpenHandler = std::make_shared<fs::api::FileViewOpenApi>(fileService);
    
    std::shared_ptr<fs::api::JsonGetApi>          jsonGetHandler = std::make_shared<fs::api::JsonGetApi>(jsonService);
    std::shared_ptr<fs::api::JsonGetOpenApi>      jsonGetOpenHandler = std::make_shared<fs::api::JsonGetOpenApi>(jsonService);
    std::shared_ptr<fs::api::JsonSaveApi>         jsonSaveHandler = std::make_shared<fs::api::JsonSaveApi>(jsonService);
    std::shared_ptr<fs::api::JsonSaveOpenApi>     jsonSaveOpenHandler = std::make_shared<fs::api::JsonSaveOpenApi>(jsonService);

    server.addHandler(downloadHandler);
    server.addHandler(downloadOpenHandler);
    server.addHandler(uploadHandler);
    server.addHandler(uploadOpenHandler);
    server.addHandler(viewHandler);
    server.addHandler(viewOpenHandler);
    
    server.addHandler(jsonGetHandler);
    server.addHandler(jsonGetOpenHandler);
    server.addHandler(jsonSaveHandler);
    server.addHandler(jsonSaveOpenHandler);

    server.listen(address, port);
}

void fs::initializer::startAll() {
    std::string configPath = "config.json";
    fs::common::GlobalConfig *config = fs::common::GlobalConfig::instance();
    config->parseConfig(configPath);

    std::string databasePath = config->fetchDatabasePath();
    uint16_t readers = config->fetchDatabaseReaders();
    uint32_t memory = config->fetchDatabaseMemory();
    std::shared_ptr<fs::database::DataBase> database = std::make_shared<fs::database::DataBase>(databasePath, readers, memory);

    std::thread t(startRPCServer, database);
    // t.join();

    startHTTPServer(database);
}
