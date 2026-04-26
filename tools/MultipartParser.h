#pragma once

#include <functional>
#include <string>
#include <vector>
#include <cstring>
#include <cstddef>


class MultipartParser {

public:

    struct Multipart {
        bool isFile;
        std::string name;
        std::string contentType;
        std::string filename;
        std::vector<char> data;
    };

    using FileDataCallback = std::function<void(struct Multipart const& part, const char* data, size_t len)>;

    MultipartParser(const std::string& boundary);
    ~MultipartParser() = default;

    // 禁止拷贝
    MultipartParser(const MultipartParser&) = delete;
    MultipartParser& operator=(const MultipartParser&) = delete;

    // 喂入数据块
    void feed(const char* data, size_t len);
    bool isDone() const { return m_state == PARSE_END; }
    bool isError() const { return m_state == PARSE_ERROR; }

    // 当前part的文件流式回调
    void setFileDataCallback(FileDataCallback cb) { m_fileCb = std::move(cb); }

private:
    enum State {
        PARSE_BOUNDARY,   // 等待第一个边界
        PARSE_HEADER,    // 解析当前部分的头部
        PARSE_BODY,       // 解析正文数据
        PARSE_END,         // 解析完成
        PARSE_ERROR         // 解析完成
    };

    void processHeader(const std::string& line);
    bool processBodyData();
    
    void clearCurrentPart() {
        m_currentPart.name.clear();
        m_currentPart.filename.clear();
        m_currentPart.isFile = false;
        m_currentPart.contentType.clear();
        m_currentPart.data.clear();
    }


    State m_state = PARSE_BOUNDARY;
    std::string m_boundary;          // 原始boundary，例如 "----WebKitFormBoundary..."
    std::string m_fullBoundary;     // "--" + m_boundary
    std::string m_endBoundary;      // "--" + m_boundary + "--"

    std::vector<Multipart> m_parts;
    Multipart m_currentPart;

    std::string m_buf;

    FileDataCallback m_fileCb;
};
