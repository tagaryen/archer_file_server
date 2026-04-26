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

    // 当前解析是否成功完成
    bool isDone() const { return m_state == PARSE_END; }

    // 当前part的文件流式回调
    void setFileDataCallback(FileDataCallback cb) { m_fileCb = std::move(cb); }

private:
    enum State {
        PARSE_BOUNDARY,   // 等待第一个边界
        PARSE_HEADERS,    // 解析当前部分的头部
        PARSE_BODY,       // 解析正文数据
        PARSE_END         // 解析完成
    };

    // 处理一个完整行（以 \r\n 结尾）
    void processLine(const std::string& line);
    // 处理正文数据（直到遇到下一个边界）
    void processBodyData(const char* data, size_t len);

    State m_state = PARSE_BOUNDARY;
    std::string m_boundary;          // 原始boundary，例如 "----WebKitFormBoundary..."
    std::string m_fullBoundary;     // "--" + m_boundary
    std::string m_endBoundary;      // "--" + m_boundary + "--"

    std::vector<Multipart> m_parts;

    // 当前 part 的临时变量
    Multipart m_currentPart;
    // 缓冲未完成的行
    std::string m_lineBuffer;

    // 当前 part 为文件时的回调
    FileDataCallback m_fileCb;
};
