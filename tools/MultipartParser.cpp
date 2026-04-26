// multipart_parser.cpp
#include "MultipartParser.h"
#include <algorithm>

MultipartParser::MultipartParser(const std::string& boundary)
    : m_boundary(boundary)
{
    m_fullBoundary = "--" + m_boundary;
    m_endBoundary  = "--" + m_boundary + "--";
}

static std::string parseAttr(const std::string& str, const std::string& attr) {
    size_t pos = str.find(attr + "=\"");
    if (pos == std::string::npos) return "";
    pos += attr.length() + 2;
    size_t end = str.find('"', pos);
    if (end == std::string::npos) return "";
    return str.substr(pos, end - pos);
};

void MultipartParser::feed(const char* data, size_t len)
{
    if (m_state == PARSE_END) return;

    size_t processed = 0;
    while (processed < len) {
        if (m_state == PARSE_BOUNDARY || m_state == PARSE_HEADERS) {
            // 按行解析
            size_t line_end = 0;
            bool found = false;
            for (size_t i = processed; i < len - 1; ++i) {
                if (data[i] == '\r' && data[i+1] == '\n') {
                    line_end = i + 2; // 包含 \r\n
                    found = true;
                    break;
                }
            }
            if (found) {
                // 取出当前行 (不包含末尾的 \r\n)
                std::string line(data + processed, line_end - 2 - processed);
                processed = line_end;
                // 将之前缓冲的行拼接上
                if (!m_lineBuffer.empty()) {
                    line = m_lineBuffer + line;
                    m_lineBuffer.clear();
                }
                processLine(line);
            } else {
                // 没有完整行，暂存剩余数据
                m_lineBuffer.append(data + processed, len - processed);
                processed = len;
            }
        } else if (m_state == PARSE_BODY) {
            // 正文部分可能跨多个数据块，特殊处理
            processBodyData(data + processed, len - processed);
            processed = len; // processBodyData 内部已经推进了状态，本函数中不进一步推进
        } else {
            break;
        }
    }
}

void MultipartParser::processLine(const std::string& line)
{
    switch (m_state) {
    case PARSE_BOUNDARY:
        // 期望行就是边界行: "--boundary" 或 "--boundary--"
        if (line == m_fullBoundary) {
            // 正常边界，准备解析头部
            m_state = PARSE_HEADERS;
            // 重置当前 part 信息
            m_currentPart.filename.clear();
            m_currentPart.name.clear();
            m_currentPart.isFile = false;
            m_currentPart.data.clear();
        } else if (line == m_endBoundary) {
            m_state = PARSE_END;
        } else {
            // 非法格式
            // 可以抛出异常或置错误状态，这里简单终止
            m_state = PARSE_END;
        }
        break;

    case PARSE_HEADERS:
        if (line.empty()) {
            // 空行，头部结束，进入正文解析
            m_state = PARSE_BODY;
        } else {
            // 解析头部行，一般第一行包含 Content-Disposition: form-data; name="..."
            // 解析头部行，一般第二行包含 Content-Type: ...
            // 简单实现：忽略大小写，只提取 name 和 filename
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                std::string key = line.substr(0, colon);
                std::string value = line.substr(colon + 1);
                if (strcasecmp("content-disposition", key.c_str())== 0) {
                    // 去除前导空格
                    size_t start = value.find_first_not_of(" \t");
                    if (start != std::string::npos) value = value.substr(start);
                    else value.clear();

                    m_currentPart.name = parseAttr(value, "name");
                    m_currentPart.filename = parseAttr(value, "filename");
                    m_currentPart.isFile = !m_currentPart.filename.empty();
                    
                    printf("part value = %s, filename=%s\n", value.c_str(), m_currentPart.filename.c_str());
                    fflush(stdout);
                }
                if (strcasecmp("content-type", key.c_str()) == 0) {
                    // 去除前导空格
                    size_t start = value.find_first_not_of(" \t");
                    if (start != std::string::npos) value = value.substr(start);
                    else value.clear();

                    size_t end = value.find_last_not_of(" \t");
                    if (start != std::string::npos) value = value.substr(0, end + 1);
                    else value.clear();

                    m_currentPart.contentType = value;
                }
            }
        }
        break;

    default:
        break;
    }
}

void MultipartParser::processBodyData(const char* data, size_t len)
{
    // 在正文中查找下一个边界
    // 边界可能以 '--' + m_boundary 形式出现，前面必有 \r\n 或者位于数据开始位置
    size_t pos = 0;
    while (pos < len) {
        // 尝试匹配完整边界 (--boundary)
        bool match = true;
        size_t i = 0;
        while (i < m_fullBoundary.size() && pos + i < len) {
            if (data[pos + i] != m_fullBoundary[i]) {
                match = false;
                break;
            }
            ++i;
        }
        if (match && pos + i <= len) {
            // 找到了一个边界
            size_t boundary_start = pos;
            // 边界前可能有 \r\n，如果边界之前不是数据块起始，应该丢弃之前的那个 \r\n
            // 这里约定边界总是出现在行首（即前面是 \r\n 或 data 起始），我们直接向前回退
            if (boundary_start > 0 && data[boundary_start-1] == '\n') {
                // 保留前面的 \n? 通常边界前是 \r\n，所以数据有效部分到 boundary_start-2
                // 但为了精确，直接处理：有效数据长度 = boundary_start - (前面是否有\r\n)
                size_t data_end = (boundary_start >= 2 && data[boundary_start-2] == '\r') ? boundary_start-2 : boundary_start-1;
                // 回调数据块
                printf("if data_end = %d, is_file=%d\n", data_end, m_currentPart.isFile);
                fflush(stdout);
                if (data_end > 0) {
                    if(m_currentPart.isFile && m_fileCb) {
                        m_fileCb(m_currentPart,  data, data_end);
                    } else {
                        m_currentPart.data.insert(m_currentPart.data.end(), data, data + data_end);
                    }
                }
                pos = boundary_start;
            }

            // 检查是否为结束边界
            bool is_end = false;
            if (pos + m_fullBoundary.size() + 2 <= len &&
                data[pos + m_fullBoundary.size()] == '-' &&
                data[pos + m_fullBoundary.size()+1] == '-') {
                is_end = true;
            }

            // 当前 part 结束, std::move 避免拷贝
            m_parts.push_back(std::move(m_currentPart));

            if (is_end) {
                m_state = PARSE_END;
                return;
            } else {
                m_state = PARSE_HEADERS;
                // 边界后面紧跟着 \r\n，跳过它们
                pos += m_fullBoundary.size();
                if (pos < len && data[pos] == '\r') pos++;
                if (pos < len && data[pos] == '\n') pos++;
                // 重新开始解析头部，但需要处理当前 pos 之前未完成的行？这里简化：返回，由 feed 继续调用
                return;
            }
        } else {
            // 没有匹配到完整边界，需要决定有多少数据可以安全地回调或缓冲
            // 为了防止跨越边界，我们只能输出直到 (len - m_fullBoundary.size() + 1) 的字符
            // 因为边界可能跨越不同数据块

            printf("else len = %d, m_fullBoundary.size()=%d isFile=%d\n", len, m_fullBoundary.size(), m_currentPart.isFile);
            fflush(stdout);
            size_t safe_len = len - m_fullBoundary.size() + 1;
            if (safe_len > pos && safe_len > 0) {
                size_t chunk_size = safe_len - pos;
                if (chunk_size > 0) {
                    if(m_currentPart.isFile && m_fileCb) {
                        m_fileCb(m_currentPart,  data + pos, chunk_size);
                    } else {
                        m_currentPart.data.insert(m_currentPart.data.end(), data + pos, data + (pos + chunk_size));
                    }
                }
                pos = safe_len;
            } else {
                // 数据太少，不足以匹配边界，等待下次 feed
                break;
            }
        }
    }
    // 如果遍历结束没有发现边界，且剩余数据大于0，仅当剩余数据量小于边界长度时才认为安全，否则应等待
    printf("current len - pos = %d, is_file=%d\n", len - pos, m_currentPart.isFile);
    fflush(stdout);
    if (pos < len && len - pos < m_fullBoundary.size()) {
        if(m_currentPart.isFile ) {
            m_fileCb(m_currentPart,  data + pos, len - pos);
        } else {
            m_currentPart.data.insert(m_currentPart.data.end(), data + pos, data + len);
        }
    }
}