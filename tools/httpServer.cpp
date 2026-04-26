#include "archer_net.h"

#include <string>
#include <sys/file.h>
#include <sstream>  
#include <regex>  

#include "MultipartParser.h"

static std::string multipartHead = "multipart/form-data; boundary=";

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

    FILE *file = fopen("./tmp.png", "wb");
    if(!file) {
        printf("Can not open file tmp.png\n");
        http_response_err(res);
        return ;
    }

    MultipartParser parser(contentType.substr(multipartHead.length()));
    parser.setFileDataCallback([file](struct MultipartParser::Multipart const& part, const char* data, size_t len) {
        fwrite(data, sizeof(char), len, file);
    });

    size_t totalLen = 0;
    char buf[128 * 1024];
    int bufLen = 0;
    
    while (true) {
        bufLen = http_request_read_some(req, buf, 128 * 1024);
        if(bufLen <= 0) {
            break;
        }
        parser.feed(buf, bufLen);
        bufLen = 0;
	}
    fclose(file);
    const char *text = "{\"success\":true}";
    http_response_send_response(res, text, strlen(text));
}

// g++ httpServer.cpp MultipartParser.cpp -I../include/ -L./ -larcher_net-win64 -o http.exe
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