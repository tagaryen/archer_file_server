#include "JsonService.h"

#include <stdio.h>
#include <sys/file.h>

using namespace fs::service;

void JsonService::getJsonInfo(std::shared_ptr<fs::server::ServerRequest> request) {
    std::string key = request->getKey();
    LOG_trace("Get key = %s", key.c_str());
    std::string json = m_jsonDB->getJsonInfo(key);
    if(json.empty()) {
        request->sendOk("");
        return ;
    }
    request->sendOk(json);
}

void JsonService::saveJsonInfo(std::shared_ptr<fs::server::ServerRequest> request) {
    std::string json = request->getJson();
    std::string key = request->getKey();
    LOG_trace("Save key = %s", key.c_str());

    Json::Value val = Json::Value(Json::stringValue);
    Json::Reader reader;
    if(reader.parse(json, val, false)) {
        m_jsonDB->saveJsonInfo(key, json);
        request->sendOk("{\"success\":true,\"key\":\""+key+"\"}");
        return ;
    }
    request->sendOk("{\"success\":false,\"error\":\"json parse failed\"}");
}

