//
//  util.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/16.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "string.h"

#include "env.h"

namespace nwr {
    std::string Format(const char * format, ...) {
        va_list ap;
        va_start(ap, format);
        std::string ret = FormatV(format, ap);
        va_end(ap);
        return ret;
    }
    std::string FormatV(const char * format, va_list ap) {
        va_list ap2;
        va_copy(ap2, ap);
        const int len = vsnprintf(nullptr, 0, format, ap);
        if (len == -1) {
            Fatal("vsnprintf(null) failed");
        }
        const int buf_len = len + 1;
        char * buf = new char[buf_len];
        const int len2 = vsnprintf(buf, buf_len, format, ap2);
        va_end(ap2);
        if (len2 == -1) {
            Fatal("vsnprintf failed");
        }
        std::string ret(buf, len);
        delete [] buf;
        return ret;
    }
    
    std::vector<std::string> Split(const std::string & str, const std::string & delim) {
        return Split(str, delim, -1);
    }
    std::vector<std::string> Split(const std::string & str, const std::string & delim, int max_count) {
        std::vector<std::string> ret;
        if (str.length() == 0 || max_count == 0) {
            return ret;
        }
        std::string::size_type pos = 0;
        while (true) {
            if (ret.size() + 1 == max_count) {
                break;
            }
            std::string::size_type next_pos = str.find(delim, pos);
            if (next_pos == std::string::npos) {
                break;
            }
            ret.push_back(str.substr(pos, next_pos));
            pos = next_pos + delim.length();
        }
        ret.push_back(str.substr(pos, str.length()));
        return ret;
    }
    std::string Join(const std::vector<std::string> & parts, const std::string & glue) {
        std::string str;
        for (int i = 0; i < parts.size(); i++) {
            if (i != 0) {
                str.append(glue);
            }
            str.append(parts[i]);
        }
        return str;
    }
    
    const uint8_t * AsDataPointer(const std::string & string) {
        return reinterpret_cast<const uint8_t *>(string.c_str());
    }    
    Data ToData(const std::string & string) {
        const uint8_t * p = AsDataPointer(string);
        return Data(p, p + string.length());
    }

    bool IsDigit(const std::string & str) {
        for (int i = 0; i < str.length(); i++) {
            if (!std::isdigit(str[i])) { return false; }
        }
        return true;
    }
}

