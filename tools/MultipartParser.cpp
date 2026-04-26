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

void MultipartParser::feed(const char* data, size_t size)
{
    if (m_state == PARSE_END) return;

    m_buf += std::string(data, size);
    size_t lnEnd = 0;
    while (true) {
        if(m_state == PARSE_BOUNDARY || m_state == PARSE_HEADER) {
            lnEnd = m_buf.find("\n");
            if (lnEnd != std::string::npos) {
                std::string line = m_buf.substr(0, lnEnd + 1);
                m_buf = m_buf.substr(lnEnd + 1);
                if (m_state == PARSE_BOUNDARY) {
                    if(line == m_fullBoundary + "\r\n" || line == m_fullBoundary + "\n") {
                        m_state = PARSE_HEADER;
                        clearCurrentPart();
                    } else if(line == m_endBoundary + "\r\n" || line == m_endBoundary + "\n") {
                        m_state = PARSE_END;
                        return ;
                    } else {
                        m_state = PARSE_ERROR;
                        return ;
                    }
                } else if(m_state == PARSE_HEADER) {
                    if(line == "\r\n" || line == "\n" || line.empty()) {
                        m_state = PARSE_BODY;
                    } else {
                        processHeader(line);
                    }
                }
            } else {
                if(m_buf == m_endBoundary) {
                    m_state = PARSE_END;
                }
                return ;
            }
        } else if (m_state == PARSE_BODY) {
            if(processBodyData()) {
                return ;
            }
            m_state = PARSE_BOUNDARY;
        } else {
            return ;
        }
    }
}

void MultipartParser::processHeader(const std::string& line) {
    size_t colon = line.find(':');
    if (colon != std::string::npos) {
        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);
        if (strcasecmp("content-disposition", key.c_str())== 0) {
            size_t start = value.find_first_not_of(" ");
            if (start != std::string::npos) value = value.substr(start);

            m_currentPart.name = parseAttr(value, "name");
            m_currentPart.filename = parseAttr(value, "filename");
            m_currentPart.isFile = !m_currentPart.filename.empty();
        }

        if (strcasecmp("content-type", key.c_str()) == 0) {
            size_t start = value.find_first_not_of(" ");
            if (start != std::string::npos) value = value.substr(start);
            size_t end = value.find_last_not_of(" ");
            if (start != std::string::npos) value = value.substr(0, end + 1);
            m_currentPart.contentType = value;
        }
    }
}

bool MultipartParser::processBodyData()
{
    size_t boundaryStart = m_buf.find(m_fullBoundary), dataEnd = 0;
    if (boundaryStart != std::string::npos) {
        dataEnd = boundaryStart;
        if (m_buf[boundaryStart-1] == '\n') {
            dataEnd = m_buf[boundaryStart-2] == '\r' ? boundaryStart-2 : boundaryStart-1;
        }
        if (dataEnd > 0) {
            if(m_currentPart.isFile && m_fileCb) {
                m_fileCb(m_currentPart, m_buf.c_str(), dataEnd);
            } else {
                m_currentPart.data.insert(m_currentPart.data.end(), m_buf.c_str(), m_buf.c_str() + dataEnd);
            }
        }
        m_buf = m_buf.substr(boundaryStart);
        m_parts.push_back(std::move(m_currentPart));
        return false;
    } else {
        if (m_buf.length() > m_fullBoundary.length() - 1) {
            size_t chunkSize = m_buf.length() - m_fullBoundary.length() + 1;
            if(m_currentPart.isFile && m_fileCb) {
                m_fileCb(m_currentPart,  m_buf.c_str(), chunkSize);
            } else {
                m_currentPart.data.insert(m_currentPart.data.end(), m_buf.c_str(), m_buf.c_str() + chunkSize);
            }
            m_buf = m_buf.substr(chunkSize);
        }
        return true;
    }
}