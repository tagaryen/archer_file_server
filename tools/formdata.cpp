#include <iostream>  
#include <string>  
#include <vector>  
#include <sstream>  
#include <regex>  
  
struct FormDataPart {  
    std::string name;  
    std::string filename; // empty if not a file  
    std::string content_type;  
    std::string data;  
};  
  
std::vector<FormDataPart> parseMultipart(const std::string& body, const std::string& boundary) {  
    std::vector<FormDataPart> parts;  
    std::string full_boundary = "--" + boundary;  
  
    size_t pos = 0;  
    while ((pos = body.find(full_boundary, pos)) != std::string::npos) {  
        size_t next = body.find(full_boundary, pos + full_boundary.length());  
        if (next == std::string::npos) break;  
  
        std::string part = body.substr(pos + full_boundary.length(), next - pos - full_boundary.length());  
        pos = next;  
  
        // Skip if it's the closing boundary (--boundary--)  
        if (part.find("--") == 0) break;  
  
        // Trim leading/trailing whitespace and newlines  
        part.erase(0, part.find_first_not_of("\r\n"));  
        part.erase(part.find_last_not_of("\r\n") + 1);  
  
        // Split headers and body  
        size_t header_end = part.find("\r\n\r\n");  
        if (header_end == std::string::npos) continue;  
  
        std::string headers = part.substr(0, header_end);  
        std::string data = part.substr(header_end + 4); // skip \r\n\r\n  
  
        FormDataPart p;  
        p.data = data;  
  
        // Parse Content-Disposition  
        std::regex disp_regex(R"(Content-Disposition:\s*form-data;\s*name=\"([^\"]+)\"(?:;\s*filename=\"([^\"]+)\")?)", std::regex_constants::icase);  
        std::smatch match;  
        if (std::regex_search(headers, match, disp_regex)) {  
            p.name = match[1].str();  
            if (match.size() > 2) {  
                p.filename = match[2].str();  
            }  
        }  
  
        // Parse Content-Type (optional)  
        std::regex type_regex(R"(Content-Type:\s*([^\r\n]+))", std::regex_constants::icase);  
        if (std::regex_search(headers, match, type_regex)) {  
            p.content_type = match[1].str();  
        }  
  
        parts.push_back(p);  
    }  
  
    return parts;  
}  


int main() {
    std::string s = "------WebKitFormBoundaryk8Ebl5hg5D3AMBRf\r\nContent-Disposition: form-data; name=\"file\"; filename=\"README.md\"\r\nContent-Type: text/markdown\r\n\r\n# tiny_file_server\r\nA tiny file server\r\n------WebKitFormBoundaryk8Ebl5hg5D3AMBRf--\n";
    std::string b = "----WebKitFormBoundaryk8Ebl5hg5D3AMBRf";
    parseMultipart(s, b);


}