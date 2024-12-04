#pragma once

#include <libcommon/Common.h>
#include <libcommon/GlobalConfig.h>
#include <libdatabase/DataBase.h>

#include <map>
#include <mutex>
#include <thread>
#include <vector>

#include "archer_net.h"


#ifndef RPC_VERSION
#define RPC_VERSION                   1
#endif

namespace fs 
{
namespace handler 
{

enum MsgType {GET_TYPE, SAVE_TYPE, RESPONSE_OK_TYPE, RESPONSE_FAIL_TYPE};

class ChannelMessageCache
{
public:
    ChannelMessageCache(Channel *channel) {
        m_channel = channel;
        m_lock = fair_lock_new();
        m_data.reserve(1024 * 1024);
    }
    ~ChannelMessageCache() {
        fair_lock_destroy(m_lock);
    }

    void appendDataMessage(char *data, size_t data_len);

    std::vector<char> processUnwrapData();

private:
    Channel                *m_channel;
    FairLock               *m_lock;
    
    std::vector<char>       m_data;
    size_t                  m_cap = 1024 * 1024;
    size_t                  m_len = 0;
};



class RPCHandler
{
public:

    RPCHandler(std::shared_ptr<database::DataBase> const& database);
    ~RPCHandler() {}

    void handle(Channel *channel, char *data, size_t data_len);

    void handleMessage(Channel *channel, char *data, size_t data_len);

    void close(Channel *channel);

private:

    void handleSaveJson(Channel *channel, std::vector<char> const& seq, std::vector<char> const& data, size_t off);

    void handleGetJson(Channel *channel, std::vector<char> const& seq, std::vector<char> const& data, size_t off);
    
    void doResponse(Channel *channel, MsgType msgType, std::vector<char> const& seq, std::string const& name, std::string const& content);

    
    void handleSaveJson(Channel *channel, const char *seq, const char *data, size_t data_len);

    void handleGetJson(Channel *channel, const char *seq, const char *data, size_t data_len);
    
    void doResponse(Channel *channel, MsgType msgType, const char *seq, std::string const& name, std::string const& content);

    std::shared_ptr<ChannelMessageCache> getMessageCache(Channel *channel);
    
    void delMessageCache(Channel *channel);

    std::mutex                                                m_messageMutex;
    std::mutex                                                m_fileMutex;
    std::map<uint64_t, std::shared_ptr<ChannelMessageCache>>  m_messageCache;

    
    std::shared_ptr<database::DataBase>                       m_database;
};
}
}

