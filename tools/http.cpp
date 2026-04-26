#include "archer_net.h"

#include <string>
#include <sys/file.h>
#include <sstream>  
#include <regex>  

#define KEY_START    1
#define VAL_START    2
#define CHUNKED_LEN  3
#define CHUNKED_VAL  4

static std::string fileChunked = "chunked";
static const int fileHexMap[] = {127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 127, 127, 127, 127, 127, 127, 127, 10, 11, 12, 13, 14, 15, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 10, 11, 12, 13, 14, 15, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127};
static std::string multipartHead = "multipart/form-data; boundary=";

struct MultiPartPos
{
    size_t start;
    size_t end;
    std::string contentType;
};


static MultiPartPos matchContentTypeAndGetOffset(std::string const& tmp, std::string const& bound) {
    MultiPartPos pos;
    pos.start = 0;
    pos.end = tmp.length();
    size_t off;
    if(tmp.find(bound, 0) == 0) {
        off = tmp.find("\r\n\r\n", bound.length()+2);
        if(off == std::string::npos) {
            pos.start = off;
            printf("Upload file multipart parse error, '%s'\n", tmp);
            return pos;
        }
        std::string part = tmp.substr(bound.length() + 2, off);
        std::string type = "application/none";
        std::smatch match; 
        std::regex type_regex(R"(Content-Type:\s*([^\r\n]+))", std::regex_constants::icase);  
        if (std::regex_search(part, match, type_regex)) {  
            type = match[1].str();  
        } 
        pos.contentType = type;
        pos.start = off + 4;
    }
    off = tmp.find(bound + "--", 0);
    if(off != std::string::npos) {
        pos.end = off - 2;
    }
    return pos;
}


static void http_response_err(HttpResponse *res) {
    const char *text = "{\"success\":false}";
    http_response_send_response(res, text, strlen(text));
}

static void http_handler(HttpRequest *req, HttpResponse *res) {
    
    std::string contentType(http_request_get_content_type(req));
    if(contentType.find(multipartHead, 0) == std::string::npos) {
        http_response_err(res);
        return ;
    }
    std::string bound = "--"+contentType.substr(multipartHead.length());
    uint32_t contentLength = http_request_get_content_length(req);
    
    std::string encoding = http_request_get_header(req, "Transfer-Encoding");
    printf("Transfer-Encoding = %s\n", encoding.c_str());
    fflush(stdout);

    FILE *file = fopen("./tmp.txt", "wb");
    if(!file) {
        printf("Can not open file tmp.txt\n");
        return ;
    }
    int fd = fileno(file);

    bool ok = true;
    size_t totalLen = 0;
    char buf[128 * 1024];
    int bufLen = 0;

    std::string tmp;
    struct MultiPartPos pos;
    while (true) {
        bufLen = http_request_read_some(req, buf, 128 * 1024);
        if(bufLen <= 0) {
            break;
        }
        printf("Upload file content-length=%lu", contentLength);
        tmp = std::string(buf, bufLen);
        pos = matchContentTypeAndGetOffset(tmp, bound);
        if(pos.start == std::string::npos) {
            ok = false;
            break;
        }
        fwrite(buf + pos.start, sizeof(char), pos.end - pos.start, file);
        bufLen = 0;
	}
    fclose(file);
    if(!ok) {
        http_response_err(res);
        return ;
    }
    const char *text = "{\"success\":true}";
    http_response_send_response(res, text, strlen(text));
}

int main() {
    HttpServer *http = http_server_new();
    http_server_set_message_handler(http, http_handler);
    if(http_server_listen(http, "127.0.0.1", 9607) == 0) {
        printf("Http error: %s\n", http_server_get_errstr(http));
    }
    http_server_close(http);
    http_server_free(http);
    return 0;
}