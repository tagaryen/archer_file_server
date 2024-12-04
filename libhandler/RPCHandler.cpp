#include "RPCHandler.h"
#include <sys/file.h>

using namespace fs::handler;

void ChannelMessageCache::appendDataMessage(char *data, size_t data_len) {
    if(data_len <= 0) {
        return ;
    }
    fair_lock_acquire(m_lock);
    size_t len = m_data.size(), cap = m_data.capacity();
    if(cap - len < data_len) {
        m_data.reserve(cap + data_len);
    }
    std::copy(data, data + data_len, std::back_inserter(m_data));
    fair_lock_release(m_lock);
}

// 0~3 byte length, 4 byte version
std::vector<char> ChannelMessageCache::processUnwrapData() {
    std::vector<char> ret;
    if(m_data.empty()) {
        LOG_trace("RPC MessageCache empty data");
        return ret;
    }
    bool ok = true;
    fair_lock_acquire(m_lock);
    size_t off = 0, len = m_data.size();
    if(m_len == 0) {
        int32_t s0 = m_data[0], s1 = m_data[1], s2 = m_data[2], s3 = m_data[3];
        s0 = s0 < 0 ? s0 + 256 : s0;
        s1 = s1 < 0 ? s1 + 256 : s1;
        s2 = s2 < 0 ? s2 + 256 : s2;
        s3 = s3 < 0 ? s3 + 256 : s3;
        m_len = (s0 << 24) | (s1 << 16) | (s2 << 8) | s3;
        if(RPC_VERSION != m_data[4]) {
            LOG_warn("Invalid rpc message version %d", m_data[4]);
            ok = false;
            m_data.clear();
            m_len = 0;
        }
        off = 5;
    }
    LOG_trace("Parse m_len: %lu", m_len);
    if(ok) {
        if(m_len <= len - off) {
            ret.reserve(m_len);
            std::copy(m_data.begin() + off, m_data.begin() + (m_len + off), std::back_inserter(ret));
            if(len > m_len + off) {
                std::copy(m_data.begin() + (m_len + off), m_data.end(), m_data.begin());
                m_data.resize(len - m_len - off);
            } else {
                m_data.clear();
            }
            m_len = 0;
        } else if(off > 0) {
            std::copy(m_data.begin() + off, m_data.end(), m_data.begin());
            m_data.resize(len - off);
        }
    }
    fair_lock_release(m_lock);
    return ret;
}

RPCHandler::RPCHandler(std::shared_ptr<database::DataBase> const& database) {
    m_database = database;
}

std::shared_ptr<ChannelMessageCache> RPCHandler::getMessageCache(Channel *channel) {
    uint64_t channelUint = (uint64_t) channel;
    std::lock_guard<std::mutex> lock(m_messageMutex);
    auto it = m_messageCache.find(channelUint);
    if(it == m_messageCache.end()) {
        m_messageCache.insert(std::make_pair(channelUint, std::make_shared<ChannelMessageCache>(channel)));
    } else {
        return it->second;
    }
    LOG_trace("RPC Handler get msgCache: %llu", channelUint);
    return m_messageCache[channelUint];
}

void RPCHandler::delMessageCache(Channel *channel) {
    uint64_t channelUint = (uint64_t) channel;
    std::lock_guard<std::mutex> lock(m_messageMutex);
    m_messageCache.erase(channelUint);
}

void RPCHandler::close(Channel *channel) {
    delMessageCache(channel);
}

// 1byte 0 == get or 1 == save or 2 == response ok or 3 == response failed
void RPCHandler::handle(Channel *channel, char *data, size_t data_len) {
    
    LOG_trace("RPC Handler incomming data, len: %d", data_len);
    
    std::shared_ptr<ChannelMessageCache> msgCache = getMessageCache(channel);
    msgCache->appendDataMessage(data, data_len);
    std::vector<char> msgData = msgCache->processUnwrapData();
    
    LOG_trace("RPC Handler unwrapped data.len: %d", msgData.size());
    while(msgData.size() > 33) {
        std::vector<char> seq(msgData.begin(), msgData.begin() + 32);
        char type = msgData[32];
        if(type == 0) { // get
            handleGetJson(channel, seq, msgData, 33);
        } else if(type == 1) { // save
            handleSaveJson(channel, seq, msgData, 33);
        } else {
            LOG_warn("Invalid data option, nor get or save, type = %d", type);
        }
        msgData = msgCache->processUnwrapData();
    }
}
    
void RPCHandler::handleSaveJson(Channel *channel, std::vector<char> const& seq, std::vector<char> const& data, size_t off) {
    const char *chars = data.data();
    size_t len = data.size();

    size_t name_len = (((size_t)data[off++]) << 8) | ((size_t)data[off++]);
    std::string name = std::string(chars + off, name_len);
    off += name_len;
    std::string value = std::string(chars + off, len - off);

    LOG_trace("RPC RPCHandler save JSON, value: %s", value.c_str());
    if(m_database->saveJsonInfo(name, value)) {
        LOG_info("RPC RPCHandler save JSON fail, key:'%s'", name.c_str());
        doResponse(channel, RESPONSE_FAIL_TYPE, seq, name, "save json data to database failed");
    } else {
        LOG_info("RPC RPCHandler save JSON success, key:'%s'", name.c_str());
        doResponse(channel, RESPONSE_OK_TYPE, seq, name, "");
    }
}

void RPCHandler::handleGetJson(Channel *channel, std::vector<char> const& seq, std::vector<char> const& data, size_t off) {
    const char *chars = data.data();
    size_t len = data.size();
    std::string name = std::string(chars + off, len - off);
    std::string value = m_database->getJsonInfo(name);

    LOG_trace("RPC RPCHandler get JSON, value: %s", value.c_str());
    if(value.empty()) {
        LOG_info("RPC RPCHandler get JSON fail, key:'%s'", name.c_str());
        doResponse(channel, RESPONSE_FAIL_TYPE, seq, name, "key not found");
    } else {
        LOG_info("RPC RPCHandler get JSON success, key:'%s'", name.c_str());
        doResponse(channel, RESPONSE_OK_TYPE, seq, name, value);
    }
}


void RPCHandler::doResponse(Channel *channel, MsgType msgType, std::vector<char> const& seq, std::string const& name, std::string const& content) {
    size_t nameLen = name.length(), errLen = content.length();
    size_t len = 32 + 1 + 2 + nameLen + errLen;
    char *ret = (char *) malloc(len + 4);
    ret[0] = ((char)((len >> 24) & 0xff));
    ret[1] = ((char)((len >> 16) & 0xff));
    ret[2] = ((char)((len >> 8) & 0xff));
    ret[3] = ((char)(len & 0xff));
    ret[4] = ((char) RPC_VERSION);
    size_t off = 5;
    std::copy(seq.begin(), seq.end(), ret + off);
    off += 32;
    ret[off++] = (char) msgType;
    ret[off++] = (char) ((nameLen >> 8) & 0xff);
    ret[off++] = (char) (nameLen & 0xff);
    memcpy(ret + off, name.c_str(), nameLen);
    off += nameLen;
    if(errLen > 0) {
        memcpy(ret + off, content.c_str(), errLen);
    }
    channel_write(channel, ret, len + 4);
    free(ret);
}




void RPCHandler::handleMessage(Channel *channel, char *data, size_t data_len) {
    LOG_trace("RPC Handler incomming message, len: %d", data_len);
    if(RPC_VERSION != data[0]) {
        LOG_warn("Invalid rpc message version %d", data[0]);
        return ;
    }
    size_t seqOff = 1, off = 33;
    if(data_len > 34) {
        char type = data[off++];
        if(type == 0) { // get
            handleGetJson(channel, data + seqOff, data + off, data_len - off);
        } else if(type == 1) { // save
            handleSaveJson(channel, data + seqOff, data + off, data_len - off);
        } else {
            LOG_warn("Invalid data option, nor get or save, type = %d", type);
        }
    }
}

void RPCHandler::handleSaveJson(Channel *channel, const char *seq, const char *data, size_t len) {
    if(len < 4) {
        return ;
    }
    size_t off = 0;
    int32_t s0 = data[off++], s1 = data[off++];
    s0 = s0 < 0 ? s0 + 256 : s0;
    s1 = s1 < 0 ? s1 + 256 : s1;
    size_t name_len = (((size_t)s0) << 8) | ((size_t)s1);
    if(name_len == 0 || off + name_len >= len) {
        return ;
    }
    std::string name = std::string(data + off, name_len);
    off += name_len;
    std::string value = std::string(data + off, len - off);
    LOG_trace("RPC RPCHandler save JSON, value: %s", value.c_str());
    if(m_database->saveJsonInfo(name, value)) {
        LOG_info("RPC RPCHandler save JSON fail, key:'%s'", name.c_str());
        doResponse(channel, RESPONSE_FAIL_TYPE, seq, name, "save json data to database failed");
    } else {
        LOG_info("RPC RPCHandler save JSON success, key:'%s'", name.c_str());
        doResponse(channel, RESPONSE_OK_TYPE, seq, name, "");
    }
}

void RPCHandler::handleGetJson(Channel *channel, const char *seq, const char *data, size_t len) {
    if(len < 1) {
        return ;
    }
    std::string name = std::string(data, len);
    std::string value = m_database->getJsonInfo(name);

    LOG_trace("RPC RPCHandler get JSON, value: %s", value.c_str());
    if(value.empty()) {
        LOG_info("RPC RPCHandler get JSON fail, key:'%s'", name.c_str());
        doResponse(channel, RESPONSE_FAIL_TYPE, seq, name, "key not found");
    } else {
        LOG_info("RPC RPCHandler get JSON success, key:'%s'", name.c_str());
        doResponse(channel, RESPONSE_OK_TYPE, seq, name, value);
    }
}

void RPCHandler::doResponse(Channel *channel, MsgType msgType, const char *seq, std::string const& name, std::string const& content) {
    size_t nameLen = name.length(), errLen = content.length();
    size_t len = 1 + 32 + 1 + 2 + nameLen + errLen;
    char *ret = (char *) malloc(len + 4);
    ret[0] = ((char)((len >> 24) & 0xff));
    ret[1] = ((char)((len >> 16) & 0xff));
    ret[2] = ((char)((len >> 8) & 0xff));
    ret[3] = ((char)(len & 0xff));
    ret[4] = ((char) RPC_VERSION);
    size_t off = 5;
    memcpy(ret+off, seq, 32);
    off += 32;
    ret[off++] = (char) msgType;
    ret[off++] = (char) ((nameLen >> 8) & 0xff);
    ret[off++] = (char) (nameLen & 0xff);
    memcpy(ret + off, name.c_str(), nameLen);
    off += nameLen;
    if(errLen > 0) {
        memcpy(ret + off, content.c_str(), errLen);
    }
    channel_write(channel, ret, len + 4);
    free(ret);
}


