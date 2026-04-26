#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <functional>
#include <algorithm>
#include <cctype>

/**
 * MultipartFormDataParser - 流式解析 multipart/form-data 格式
 * 
 * 使用方式：
 * 1. 构造时传入 boundary 字符串（HTTP 头 Content-Type 中给出的 boundary）
 * 2. 设置回调：on_part_begin, on_part_data, on_part_end, on_end
 * 3. 调用 feed(data, len) 逐块喂入数据
 * 
 * 回调解释：
 * - on_part_begin(headers): 每个 part 开始时调用，headers 是一个 map<string,string>，
 *   包含 Content-Disposition 和 Content-Type 等头部信息。
 * - on_part_data(data, len): 该 part 的数据块（可能多次调用）
 * - on_part_end(): 当前 part 结束
 * - on_end(): 解析完成，所有 parts 都已处理
 */
class MultipartFormDataParser {
public:

    struct Multipart {
        bool isFile;
        std::string name;
        std::string contentType;
        std::string filename;
        std::vector<char> data;
    };
    
    // using Headers = std::vector<std::pair<std::string, std::string>>;
    // using PartBeginCallback = std::function<void(const Headers&)>;
    using PartDataCallback = std::function<void(const char* data, size_t len)>;
    using PartEndCallback = std::function<void()>;
    using EndCallback = std::function<void()>;

    MultipartFormDataParser(const std::string& boundary)
        : m_boundary(boundary)
        , m_boundaryLine("--" + boundary + "\r\n")
        , m_boundaryEnd("--" + boundary + "--\r\n")
        , m_state(State::BOUNDARY)
        , m_boundaryPos(0)
        , m_hasLastBoundary(false) {}

    // 设置回调
    void set_on_part_begin(PartBeginCallback cb) { on_part_begin_ = std::move(cb); }
    void set_on_part_data(PartDataCallback cb) { on_part_data_ = std::move(cb); }
    void set_on_part_end(PartEndCallback cb) { on_part_end_ = std::move(cb); }
    void set_on_end(EndCallback cb) { on_end_ = std::move(cb); }

    // 喂入数据块，返回 true 表示成功，false 表示格式错误
    bool feed(const char* data, size_t len) {
        if (m_state == State::DONE) return false; // 已经结束

        m_buffer.append(data, len);

        while (true) {
            switch (m_state) {
            case State::BOUNDARY:
                // 尝试找到下一个边界标记
                if (!findBoundary()) {
                    // 剩余数据不足以构成完整的 boundary，等待更多数据
                    // 但需要保留可能的前缀部分
                    return true;
                }
                // 找到 boundary 后，跳过 m_boundaryLine 或 m_boundaryEnd
                m_buffer.erase(0, m_boundaryMatchLen);
                if (m_boundaryIsEnd) {
                    // 遇到结束边界，解析完成
                    m_state = State::DONE;
                    return true;
                }
                // 普通边界，开始新的 part
                m_state = State::HEADERS;
                clearCurrentPart();
                break;

            case State::HEADERS:
                if (!parseHeaders()) {
                    // 头部不完整，需要更多数据
                    return true;
                }
                // 头部解析完成，通知用户
                if (on_part_begin_) on_part_begin_(m_headers);
                m_state = State::DATA;
                break;

            case State::DATA:
                if (!processData()) {
                    return true; // 等待更多数据
                }
                // 当前 part 数据结束，通知结束
                if (on_part_end_) on_part_end_();
                m_state = State::BOUNDARY;
                break;

            case State::DONE:
                return true;
            }
        }
        return true;
    }

    // 通知解析器没有更多数据（用来检查是否正常结束）
    bool finish() {
        // 如果解析还没完成且缓冲区有待处理数据，尝试解析
        if (m_state != State::DONE && !m_buffer.empty()) {
            // 简单的结束检查：应该已经遇到结束边界
            feed(nullptr, 0); // 触发最后的查找
        }
        return m_state == State::DONE;
    }

private:
    enum class State {
        BOUNDARY,   // 寻找边界标记
        HEADERS,    // 解析头部（直到空行）
        DATA,       // 读取 part 数据直到下一个边界
        DONE        // 解析完成（已遇到结束边界）
    };

    std::string m_boundary;
    std::string m_boundaryLine;   // "--" + boundary + "\r\n"
    std::string m_boundaryEnd;    // "--" + boundary + "--\r\n"
    State m_state;
    std::string m_buffer;          // 未处理的数据缓冲区
    size_t m_boundaryPos;         // 当前正在匹配边界的索引
    size_t m_boundaryMatchLen;   // 匹配到的边界长度（boundary_line_ 或 boundary_end_）
    bool m_boundaryIsEnd;        // 是否匹配到结束边界
    std::vector<Multipart> m_parts;
    Multipart m_currentPart;      // 当前 part 的临时变量
    std::string m_headerLine;     // 临时存储一行头部
    bool m_headerLineStart;      // 标记是否在行首
    bool m_hasLastBoundary;

    void clearCurrentPart() {
        m_currentPart.name.clear();
        m_currentPart.filename.clear();
        m_currentPart.isFile = false;
        m_currentPart.contentType.clear();
        m_currentPart.data.clear();
    }
    // 在 m_buffer 中查找边界标记（boundary_line_ 或 boundary_end_）
    // 返回 true 如果找到，false 如果未找到（需要更多数据）
    bool findBoundary() {
        // 边界标记长度至少 2 个字符（"--"）加上 boundary 本身
        const size_t min_match_len = 2 + m_boundary.size() + 2; // "--" + boundary + "\r\n" 至少
        if (m_buffer.size() < min_match_len) return false;

        // 在 m_buffer 中搜索第一个 "\r\n--" 或直接边界（若是数据开头则没有前导 \r\n）
        // 规范要求边界前有一个 \r\n，但第一段数据可能直接以边界开头。
        // 简化处理：在 m_buffer 中搜索 "--" + m_boundary 的位置。
        std::string search_str = "--" + m_boundary;
        size_t pos = m_buffer.find(search_str);
        if (pos == std::string::npos) {
            // 未找到，但可能部分匹配跨 buffer 边界？
            // 保留 m_buffer 末尾足够长的部分（search_str 长度-1）用于下次匹配
            if (m_buffer.size() > search_str.size()) {
                m_buffer.erase(0, m_buffer.size() - (search_str.size() - 1));
            }
            return false;
        }

        // 检查边界后跟 "\r\n" 还是 "--\r\n"
        size_t check_pos = pos + search_str.size();
        if (check_pos >= m_buffer.size()) return false; // 需要更多数据判断
        if (m_buffer.compare(check_pos, 2, "\r\n") == 0) {
            m_boundaryMatchLen = search_str.size() + 2; // 包含 "\r\n"
            m_boundaryIsEnd = false;
        } else if (m_buffer.compare(check_pos, 4, "--\r\n") == 0) {
            m_boundaryMatchLen = search_str.size() + 4;
            m_boundaryIsEnd = true;
        } else {
            // 不是有效的边界，继续向后搜索
            m_buffer.erase(0, pos + 1);
            return findBoundary();
        }

        // 找到边界后，确保它前面是行首（即前一个是 \r\n 或 pos==0 且是开头）
        // （第一块数据可能没有前导 \r\n，这是允许的）
        if (pos > 0) {
            if (m_buffer[pos-1] != '\n') {
                // 不是独立行，忽略此匹配，继续搜索
                m_buffer.erase(0, pos + 1);
                return findBoundary();
            }
        }

        // 移除匹配到的边界之前的无用数据（注意保留边界之前的缓冲区？实际上边界之前的全是上一个 part 的数据，
        // 但在 STATE_DATA 状态下我们已经处理了上一个 part 直到边界前的所有数据，所以这里不应该还有数据）
        // 为了安全，可以删除 pos 之前的内容。
        if (pos > 0) {
            m_buffer.erase(0, pos);
            // 重新计算匹配位置（因为 m_buffer 变了）
            return findBoundary();
        }
        return true;
    }

    // 解析头部，直到遇到空行（\r\n\r\n）
    // 返回 true 表示头部解析完成，false 表示需要更多数据
    bool parseHeaders() {
        // 不断读取行直到空行
        while (true) {
            // 查找行结束符
            size_t eol = m_buffer.find("\r\n");
            if (eol == std::string::npos) {
                // 行不完整
                return false;
            }
            std::string line = m_buffer.substr(0, eol);
            m_buffer.erase(0, eol + 2); // 删除行 + \r\n

            if (line.empty()) {
                // 空行，头部结束
                return true;
            }

            // 解析头部字段（格式：Name: value）
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                std::string name = line.substr(0, colon);
                std::string value = line.substr(colon + 1);
                // 去除值前导空白
                size_t pos = value.find_first_not_of(" \t");
                if (pos != std::string::npos) value.erase(0, pos);
                // 去除尾部\r?（实际上之前已去掉）
                // 保存使用小写字段名以便查找
                std::transform(name.begin(), name.end(), name.begin(), ::tolower);
                m_headers.emplace_back(name, value);
            }
        }
    }

    // 处理当前 part 的数据，直到遇到下一个边界标记
    // 返回 true 当该 part 的数据结束（即下一个边界已经找到），false 需要更多数据
    bool processData() {
        // 在 m_buffer 中搜索下一个边界标记（"--" + boundary_）
        std::string search_str = "\r\n--" + m_boundary; // 规范中边界前有 \r\n，但数据可能是第一段？
        // 如果 part 数据可能为空，边界可能紧跟在 \r\n\r\n 之后，此时缓冲区可能直接以边界开头。
        // 另外数据部分中可能出现 "\r\n--" 但不是边界（理论上不会，因为边界是唯一的）。
        // 简化：搜索 search_str，若找不到则保留可能为边界前缀的部分。
        size_t pos = m_buffer.find(search_str);
        if (pos == std::string::npos) {
            // 未找到完整边界，但可能缓冲区末尾包含部分前缀，需要保留
            // 保留长度 search_str.size()-1 以避免跨 buffer 错过边界
            if (m_buffer.size() > search_str.size() - 1) {
                // 将前面确定不是边界前缀的数据输出
                size_t keep = search_str.size() - 1;
                size_t send = m_buffer.size() - keep;
                if (send > 0 && on_part_data_) {
                    on_part_data_(m_buffer.data(), send);
                }
                m_buffer.erase(0, send);
            }
            return false; // 需要更多数据
        }

        // 找到边界，将边界之前的数据（即当前 part 的内容）输出
        if (pos > 0 && on_part_data_) {
            on_part_data_(m_buffer.data(), pos);
        }
        // 删除数据部分，保留边界部分（包括前导的 \r\n），让 STATE_BOUNDARY 去处理
        m_buffer.erase(0, pos);
        return true;
    }
};

// ---------- 示例使用 ----------
/*
#include <iostream>
#include <fstream>

void test() {
    // 模拟 multipart/form-data 请求体（实际从 socket 读入）
    std::string boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
    MultipartFormDataParser parser(boundary);

    parser.set_on_part_begin([](const MultipartFormDataParser::Headers& headers) {
        std::cout << "=== Part Begin ===" << std::endl;
        for (auto& h : headers) {
            std::cout << h.first << ": " << h.second << std::endl;
        }
    });

    parser.set_on_part_data([](const char* data, size_t len) {
        std::cout << "Data: " << std::string(data, len) << std::endl;
    });

    parser.set_on_part_end([]() {
        std::cout << "=== Part End ===" << std::endl;
    });

    parser.set_on_end([]() {
        std::cout << "=== All Parts Done ===" << std::endl;
    });

    // 模拟分块喂入数据
    std::string body = 
        "------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
        "Content-Disposition: form-data; name=\"field1\"\r\n"
        "\r\n"
        "value1\r\n"
        "------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
        "Content-Disposition: form-data; name=\"file\"; filename=\"test.txt\"\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "Hello, world!\r\n"
        "More text.\r\n"
        "------WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n";

    // 分块喂入
    size_t chunk = 10;
    for (size_t i = 0; i < body.size(); i += chunk) {
        parser.feed(body.data() + i, std::min(chunk, body.size() - i));
    }
    parser.finish();
}

int main() {
    test();
    return 0;
}
*/