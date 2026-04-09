#include "DataBase.h"
#include <regex>

using namespace fs::database;

const std::string publicFileKey = "0000000000000000000000000000000000000000000000000000000000000000";

static std::string FileInfoIntToString(size_t n) {
    char chars[] = {((char) ((n >> 24) & 0xff)), ((char) ((n >> 16) & 0xff)), ((char) ((n >> 8) & 0xff)), ((char) (n & 0xff))};
    return fs::common::getHexFromUint8s((uint8_t *) chars, 4);
}

static size_t FileInfoStringToInt(std::string const& str) {
    uint8_t chars[4];
    fs::common::getUint8sFromHex(str, chars);
    return (((size_t)chars[0]) << 24) | (((size_t)chars[1]) << 16) | (((size_t)chars[2]) << 8) | ((size_t)chars[3]);
}

static char* FileInfoGetPrintStr(const char *data, size_t data_len) {
    char *printStr = (char *)malloc(data_len + 1);
    memcpy(printStr, data, data_len);
    printStr[data_len] = '\0';
    return printStr;
}

FileInfo::FileInfo(std::string const& realName, std::string const& publicKeyHex, std::string const& updateTime, std::string const& contentType) {
    m_valid = true;
    std::string name;
    if(publicKeyHex.empty()) {
        name = realName;
    } else {
        name = realName + publicKeyHex;
    }
    Hash32 hash;
    sm3((uint8_t *)name.c_str(), name.length(), &hash);
    m_name = common::getHexFromUint8s(hash.h, 32);
    m_realName = realName;
    m_publicKeyHex = publicKeyHex;
    m_updateTime = updateTime;
    m_contentType = contentType;
    
    LOG_trace("Fileinfo save name = %s", m_name.c_str());

    std::string nameLen = FileInfoIntToString(m_realName.length());
    m_encode = m_name + (m_publicKeyHex.empty()?'0':'1') + m_publicKeyHex + m_updateTime + nameLen + m_realName + m_contentType;

    LOG_trace("Fileinfo encoded = %s", m_encode.c_str());
}

void FileInfo::setContentType(std::string const& contentType) {
    m_contentType = contentType;
    
    std::string nameLen = FileInfoIntToString(m_realName.length());
    m_encode = m_name + (m_publicKeyHex.empty()?'0':'1') + m_publicKeyHex + m_updateTime + nameLen + m_realName + m_contentType;

    LOG_trace("Set contentType encoded = %s", m_encode.c_str());
};

void FileInfo::decode(const char *data, size_t data_len) {
    LOG_trace("Fileinfo raw size = %llu", data_len);
    if(data_len < 65) {
        char *printStr = FileInfoGetPrintStr(data, data_len);
        LOG_error("fileInfo Can not parse database value to FileInfo, Value = %s", printStr);
        free(printStr);
        return ;
    }

    bool includePublicKey = data[64] == '1';
    size_t basicLen = 64 + (includePublicKey?128:0) + TIME_FORMAT_LEN + 8 + 10;

    if(data_len < basicLen) {
        char *printStr = FileInfoGetPrintStr(data, data_len);
        LOG_error("fileInfo Can not parse file database value to FileInfo, Value = %s", printStr);
        free(printStr);
        return ;
    }

    m_encode = std::string(data, data_len);
    m_name = m_encode.substr(0, 64);
    LOG_trace("Fileinfo m_encode = %s, m_name=%s", m_encode, m_name);
    size_t off = 65;
    if(includePublicKey) {
        m_publicKeyHex = m_encode.substr(off, 128);
        off += 128;
    }
    m_updateTime = m_encode.substr(off, TIME_FORMAT_LEN);
    off += TIME_FORMAT_LEN;

    std::string nameLenStr = m_encode.substr(off, 8);
    off+= 8;

    size_t nameLen =  FileInfoStringToInt(nameLenStr);
    m_realName = m_encode.substr(off, nameLen);
    off += nameLen;

    m_contentType = m_encode.substr(off);

    m_valid = true;

}

DataBase::DataBase(std::string const& dbPath, unsigned int readerNum, size_t maxMemorySize){
    if(doError(mdb_env_create(&m_env))) {
        console_err("Can not create lmdb file database environment. Exit(0)\n");
        LOG_error("Can not create lmdb file database environment. Exit(0)\n");
        exit(0);
    }
    if(doError(mdb_env_set_maxdbs(m_env, 4))) {
        console_err("Can not set lmdb file database max db nums. Exit(0)");
        LOG_error("Can not set lmdb file database max db nums. Exit(0)");
        exit(0);
    }
    if(doError(mdb_env_set_maxreaders(m_env, readerNum))) {
        console_err("Can not set lmdb file database readers. Exit(0)");
        LOG_error("Can not set lmdb file database readers. Exit(0)");
        exit(0);
    }
    if(doError(mdb_env_set_mapsize(m_env, maxMemorySize))) {
        console_err("Can not set lmdb file database max memory size. Exit(0)");
        LOG_error("Can not set lmdb file database max memory size. Exit(0)");
        exit(0);
    }

    fs::common::createDirectories(dbPath);
    if(doError(mdb_env_open(m_env, dbPath.c_str(), 0, 0664))) {
        console_err("Can not open lmdb file database dir '%s'. Exit(0)", dbPath.c_str());
        LOG_error("Can not open lmdb file database dir '%s'. Exit(0)", dbPath.c_str());
        exit(0);
    }

    MDB_txn *txn;
    if(doError(mdb_txn_begin(m_env, NULL, 0, &txn))) {
        console_err("Begin init file transaction failed. Exit(0)");
        LOG_error("Begin init file transaction failed. Exit(0)");
        exit(0);
    }

    if(doError(mdb_dbi_open(txn, "fileinfo", MDB_CREATE, &m_dbi))) {
        console_err("Open file database failed. Exit(0)");
        LOG_error("Open file database failed. Exit(0)");
        mdb_txn_abort(txn);
        exit(0);
    }
    mdb_txn_commit(txn);

    console_out("Create lmdb file database success");
    LOG_info("Create lmdb file database success");
}


std::string DataBase::findKv(std::string const& key) {
    auto it = m_kvRLU.find(key);
    if(it != m_kvRLU.end()) {
        return it->second;
    }
    return "";
}

bool DataBase::saveKv(std::string const& key, std::string const& val) {
    std::lock_guard<std::mutex> lock(m_kvMutex);
    auto it = m_kvRLU.find(key);
    if(it != m_kvRLU.end()) {
        if(it->second == val) {
            return false;
        }
        it->second = val;
        return true;
    }
    if(m_kvRLU.size() > 64) {
        it = m_kvRLU.begin();
        m_kvRLU.erase(it->first);
    }
    m_kvRLU[key] = val;
    return true;
}


std::vector<std::string> DataBase::getFileList(std::string const& key) {
    int rc = 0;

    // MDB_dbi dbi;
    MDB_val dbKey, dbValue;
    MDB_cursor *cursor = NULL;
    MDB_txn *txn;

    if((rc = mdb_txn_begin(m_env, NULL, 0, &txn))) {
        LOG_error("database Begin read transaction failed, due to %s", mdb_strerror(rc));
        return std::vector<std::string>();
    }

    LOG_trace("Database get key: %s, key.length:%llu", key.c_str(), key.length());

    dbKey.mv_size = key.length();
    dbKey.mv_data = (void *)(key.c_str());

    dbValue.mv_size = 0;
    LOG_trace("database Begin read transaction Key = %s", key.c_str());
    if((rc = mdb_get(txn, m_dbi, &dbKey, &dbValue))) {
        LOG_error("database Readding key %s failed, due to %s", key.c_str(), mdb_strerror(rc));
        mdb_txn_abort(txn);
        return std::vector<std::string>();
    }
    std::string val((char *)dbValue.mv_data, dbValue.mv_size);
    if((rc = mdb_txn_commit(txn))) {
        LOG_error("database read Commit transaction failed, due to %s", mdb_strerror(rc));
        mdb_txn_abort(txn);
        return std::vector<std::string>();
    }
    // LOG_trace("database Begin getList val = %s", val.c_str());
    std::regex reg("@`");
    return std::vector<std::string>(std::sregex_token_iterator(val.begin(), val.end(), reg, -1), std::sregex_token_iterator());
}

//time = yyyy-MM-dd HH:mm:ss  len = 19
int DataBase::saveFileList(std::string const& key, std::vector<std::string> const& datas) {

    int rc = 0;
    // MDB_dbi dbi;
    MDB_val dbKey, dbValue;
    MDB_txn *txn;

    std::string value = "";
    for(std::string const& s: datas) {
        value += s + "@`";
    }
    
    LOG_trace("Begin to save file list to database with key = '%s', key.length = %llu", key.c_str(), key.length());

    if((rc = mdb_txn_begin(m_env, NULL, 0, &txn))) {
        LOG_error("database Begin write transaction failed, due to %s", mdb_strerror(rc));
        return rc;
    }

    LOG_trace("database Begin write transaction key = %s, value = %s", 
            key.c_str(), value.c_str());

    dbKey.mv_size = key.length();
    dbKey.mv_data = (void *)key.c_str();
    dbValue.mv_size = value.length();
    dbValue.mv_data = (void *)value.c_str();

    if((rc = mdb_put(txn, m_dbi, &dbKey, &dbValue, 0))) {
        LOG_error("database Writting data failed, due to %s", mdb_strerror(rc));
        mdb_txn_abort(txn);
        return rc;
    }
    if((rc = mdb_txn_commit(txn))) {
        LOG_error("database Commit transaction failed, due to %s", mdb_strerror(rc));
        mdb_txn_abort(txn);
        return rc;
    }

    LOG_info("Save file information to database with key = '%s' end", key.c_str());

    return rc;
}

std::shared_ptr<Page> DataBase::listFile(std::string const& publicKeyHex, int pageNum) {
    if(pageNum <= 0) {
        pageNum = 1;
    }
    std::string fileKey = publicKeyHex;
    if(fileKey.empty()) {
        fileKey = publicFileKey;
    }
    std::vector<std::string> list = getFileList(fileKey);
    if(list.size() == 1 && list[0].empty()) {
        return std::make_shared<Page>(0, pageNum, std::vector<FileInfo>());
    } 
    std::sort(list.begin(), list.end(), [](std::string a, std::string b) {
        return fs::common::timeToLong(a.substr(0, 19)) > fs::common::timeToLong(b.substr(0, 19));
    });
    int total = list.size();
    if(total <= 0) {
        return std::make_shared<Page>(0, pageNum, std::vector<FileInfo>());
    }
    std::vector<FileInfo> files;
    
    int start = (pageNum - 1) * 10, end = pageNum * 10;
    if(start > list.size()) {
        start = total;
    }
    if(end > total) {
        end = total;
    }
    std::string sizeStr;
    for(auto it = list.begin() + start; it != list.begin() + end; it++) {
        FileInfo file((*it).substr(23), "", (*it).substr(0, 19), "");
        sizeStr = (*it).substr(19, 23);
        size_t l0 = (uint8_t)sizeStr[0];
        size_t l1 = (uint8_t)sizeStr[1];
        size_t l2 = (uint8_t)sizeStr[2];
        size_t l3 = (uint8_t)sizeStr[3];
        file.setSize((l0 << 24) | (l1 << 16) | (l2 << 8) | l3);
        files.push_back(file);
    }
    return std::make_shared<Page>(total, pageNum, files);
}

// 2025-04-05 12:23:451027README.md
// 2025-04-05 12:23:451027LICENSE
int DataBase::saveFileInfo(std::shared_ptr<FileInfo> const& fileInfo) {

    std::string fileKey = fileInfo->getPublicKeyHex();
    if(fileKey.empty()) {
        fileKey = publicFileKey;
    }

    bool saved = false;
    size_t size = fileInfo->getSize();
    char lenChar[5] = {(char)((size >> 24) & 0xff), (char)((size >> 16) & 0xff), (char)((size >> 8) & 0xff), (char)(size & 0xff), 0};
    std::string lenStr(lenChar, 4);
    std::string val = fileInfo->getUpdateTime() + lenStr + fileInfo->getRealName();
    LOG_info("Save file as list value: name=%s, size=%llu, contentType=%s", fileInfo->getRealName().c_str(), fileInfo->getSize(), fileInfo->getContentType().c_str());
    std::vector<std::string> list = getFileList(fileKey);
    for(size_t i = 0; i < list.size(); i++) {
        if(list[i].substr(23) == val.substr(23)) {
            list[i] = val;
            saved = true;
            break;
        }
    }
    int rc = 0;
    if(!saved) {
        list.push_back(val);
    }
    if(rc = saveFileList(fileKey, list)) {
        return rc;
    }

    // MDB_dbi dbi;
    MDB_val dbKey, dbValue;
    MDB_txn *txn;

    std::string key = fileInfo->getName();
    std::string value = fileInfo->encode();
    
    LOG_trace("Begin to save file information to database with key = '%s', key.length = %llu", key.c_str(), key.length());

    if((rc = mdb_txn_begin(m_env, NULL, 0, &txn))) {
        LOG_error("database Begin write transaction failed, due to %s", mdb_strerror(rc));
        return rc;
    }

    LOG_info("database Begin write transaction PublicKey = %s, Filename = %s", 
            fileInfo->getPublicKeyHex().c_str(), fileInfo->getRealName().c_str());

    dbKey.mv_size = key.length();
    dbKey.mv_data = (void *)key.c_str();
    dbValue.mv_size = value.length();
    dbValue.mv_data = (void *)value.c_str();

    if((rc = mdb_put(txn, m_dbi, &dbKey, &dbValue, 0))) {
        LOG_error("database Writting data failed, due to %s", mdb_strerror(rc));
        mdb_txn_abort(txn);
        return rc;
    }
    if((rc = mdb_txn_commit(txn))) {
        LOG_error("database Commit transaction failed, due to %s", mdb_strerror(rc));
        mdb_txn_abort(txn);
        return rc;
    }

    LOG_info("Save file information to database with key = '%s' end", key.c_str());

    return rc;
}

std::shared_ptr<FileInfo> DataBase::getFileInfo(std::string const & name) {
    int rc = 0;
    
    // MDB_dbi dbi;
    MDB_val dbKey, dbValue;
    MDB_cursor *cursor = NULL;
    MDB_txn *txn;

    if((rc = mdb_txn_begin(m_env, NULL, 0, &txn))) {
        LOG_error("database Begin read transaction failed, due to %s", mdb_strerror(rc));
        return std::shared_ptr<FileInfo>();
    }

    LOG_trace("Database set key: %s, key.length:%llu", name.c_str(), name.length());

    dbKey.mv_size = name.length();
    dbKey.mv_data = (void *)(name.c_str());

    dbValue.mv_size = 0;
    LOG_info("database Begin read transaction Key = %s", name.c_str());
    if((rc = mdb_get(txn, m_dbi, &dbKey, &dbValue))) {
        LOG_error("database Readding key %s failed, due to %s", name.c_str(), mdb_strerror(rc));
        mdb_txn_abort(txn);
        return std::shared_ptr<FileInfo>();
    }
    LOG_trace("Database start to decode value");
    std::shared_ptr<FileInfo> fileInfo = std::make_shared<FileInfo>();
    fileInfo->decode((char *)dbValue.mv_data, dbValue.mv_size);
    if((rc = mdb_txn_commit(txn))) {
        LOG_error("database read Commit transaction failed, due to %s", mdb_strerror(rc));
        mdb_txn_abort(txn);
        return std::shared_ptr<FileInfo>();
    }
    return fileInfo;
}



int DataBase::saveJsonInfo(std::string const& key, std::string const& value) {
    int rc = 0;

    bool changed = saveKv(key, value);
    if(!changed) {
        return rc;
    }
    
    // MDB_dbi dbi;
    MDB_val dbKey, dbValue;
    MDB_txn *txn;
    
    LOG_info("Begin to save json information to database with key = '%s'", key.c_str());

    if((rc = mdb_txn_begin(m_env, NULL, 0, &txn))) {
        LOG_error("database Begin write transaction failed, due to %s", mdb_strerror(rc));
        return rc;
    }

    LOG_info("Begin to save json information write transaction, Key = %s", key.c_str());

    dbKey.mv_size = key.length();
    dbKey.mv_data = (void *)key.c_str();
    dbValue.mv_size = value.length();
    dbValue.mv_data = (void *)value.c_str();

    if((rc = mdb_put(txn, m_dbi, &dbKey, &dbValue, 0))) {
        LOG_error("database Writting json data failed, due to %s", mdb_strerror(rc));
        mdb_txn_abort(txn);
        return rc;
    }
    if((rc = mdb_txn_commit(txn))) {
        LOG_error("database Commit transaction failed, due to %s", mdb_strerror(rc));
        mdb_txn_abort(txn);
        return rc;
    }
    return rc;
}

std::string DataBase::getJsonInfo(std::string const& key) {
    int rc = 0;
    std::string str = findKv(key);
    if(!str.empty()) {
        return str;
    }

    MDB_val dbKey, dbValue;
    MDB_cursor *cursor = NULL;
    MDB_txn *txn;

    LOG_info("Begin to get json information from database with key = '%s'", key.c_str());

    if((rc = mdb_txn_begin(m_env, NULL, 0, &txn))) {
        LOG_error("database Begin read transaction failed, due to %s", mdb_strerror(rc));
        return "";
    }

    LOG_info("Begin to get json information write transaction with key = '%s'", key.c_str());

    dbKey.mv_size = key.length();
    dbKey.mv_data = (void *)(key.c_str());

    if((rc = mdb_get(txn, m_dbi, &dbKey, &dbValue))) {
        LOG_error("database Readding key %s failed, due to %s", key.c_str(), mdb_strerror(rc));
        mdb_txn_abort(txn);
        return "";
    }
    if((rc = mdb_txn_commit(txn))) {
        LOG_error("database read Commit transaction failed, due to %s", mdb_strerror(rc));
        mdb_txn_abort(txn);
        return "";
    }
    std::string val((char *)dbValue.mv_data, dbValue.mv_size);
    saveKv(key, val);
    return val;
}


