
#include <stdio.h>
#include <sys/file.h>
#include <string>
#include <vector>
#include <stdint.h>  
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cstddef>
#include <utility>


static const std::string base64_chars =  
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"  
    "abcdefghijklmnopqrstuvwxyz"  
    "0123456789+/";  
  
static inline bool is_base64(unsigned char c) {  
    return (isalnum(c) || (c == '+') || (c == '/'));  
}  
  
std::string encode(const std::vector<uint8_t>& data) {  
    std::string ret;  
    size_t i = 0;  
    uint8_t char_array_3[3];  
    uint8_t char_array_4[4];  
  
    while (data.size() > i) {  
        char_array_3[0] = i < data.size() ? data[i++] : 0;  
        char_array_3[1] = i < data.size() ? data[i++] : 0;  
        char_array_3[2] = i < data.size() ? data[i++] : 0;  
  
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;  
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);  
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);  
        char_array_4[3] = char_array_3[2] & 0x3f;  
  
        for (size_t j = 0; j < 4; ++j)  
            ret += base64_chars[char_array_4[j]];  
  
        // 处理 padding  
        if (i > data.size()) {  
            if (i - data.size() == 1) {  
                ret[ret.size() - 1] = '=';  
                ret[ret.size() - 2] = '=';  
            } else if (i - data.size() == 2) {  
                ret[ret.size() - 1] = '=';  
            }  
        }  
    }  
  
    return ret;  
}  
  
std::string encode(const std::string& data) {  
    std::vector<uint8_t> vec(data.begin(), data.end());  
    return encode(vec);  
}  
  
std::vector<uint8_t> decode(const std::string& encoded_string) {  
    size_t in_len = encoded_string.size();  
    size_t i = 0;  
    size_t j = 0;  
    size_t in_ = 0;  
    uint8_t char_array_4[4], char_array_3[3];  
    std::vector<uint8_t> ret;  
  
    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {  
        char_array_4[i++] = encoded_string[in_]; in_++;  
        if (i == 4) {  
            for (i = 0; i < 4; ++i)  
                char_array_4[i] = static_cast<uint8_t>(base64_chars.find(char_array_4[i]));  
  
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);  
            char_array_3[1] = ((char_array_4[1] & 0x0f) << 4) + ((char_array_4[2] & 0x3c) >> 2);  
            char_array_3[2] = ((char_array_4[2] & 0x03) << 6) + char_array_4[3];  
  
            for (i = 0; (i < 3); ++i)  
                ret.push_back(char_array_3[i]);  
            i = 0;  
        }  
    }  
  
    if (i) {  
        for (j = i; j < 4; ++j)  
            char_array_4[j] = 0;  
  
        for (j = 0; j < 4; ++j)  
            char_array_4[j] = static_cast<uint8_t>(base64_chars.find(char_array_4[j]));  
  
        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);  
        char_array_3[1] = ((char_array_4[1] & 0x0f) << 4) + ((char_array_4[2] & 0x3c) >> 2);  
        char_array_3[2] = ((char_array_4[2] & 0x03) << 6) + char_array_4[3];  
  
        for (j = 0; (j < i - 1); ++j)  
            ret.push_back(char_array_3[j]);  
    }  
  
    return ret;  
}  
  
// 返回 std::string 形式的解码结果（适用于文本）  
std::string decode_to_string(const std::string& encoded_string) {  
    std::vector<uint8_t> decoded = decode(encoded_string);  
    return std::string(decoded.begin(), decoded.end());  
}  



int testBase64Decode() {

    std::vector<uint8_t> base64 = decode("");


    FILE *file2 = fopen("./tmp.html", "ab");
    if(!file2) {
        printf("Can not open file tmp.html\n");
        return 0;
    }
    fwrite(base64.data(), sizeof(char), base64.size()-1, file2);

    fclose(file2);
    return 0;
}

std::string getNowTime() {
    std::time_t now = std::time(nullptr);
    // now += 8 * 3600;//时区问题
    std::tm *localTime = std::localtime(&now);
    char timestr[20];
    std::strftime(timestr, 20, "%Y-%m-%d %H:%M:%S", localTime);
    std::string nowStr(timestr, 19);
    return std::move(nowStr);
}

int main() {
    std::vector<uint8_t> src = {2,3,4,5,6,7,8,9};
    std::string base64 = encode(src);
    printf("base64 = %s\n", base64.c_str());
    
    std::vector<uint8_t> base64vec = decode(base64);

    return 0;
}