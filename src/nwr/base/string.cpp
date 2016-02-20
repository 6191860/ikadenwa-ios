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
        int pos = 0;
        while (true) {
            if (ret.size() + 1 == max_count) {
                break;
            }
            int next_pos = IndexOf(str, delim, pos);
            if (next_pos == -1) {
                break;
            }
            ret.push_back(str.substr(pos, next_pos));
            pos = next_pos + static_cast<int>(delim.length());
        }
        ret.push_back(str.substr(pos, str.length()));
        return ret;
    }
    std::vector<std::string> Split(const std::string & str, const std::regex & delim) {
        return Split(str, delim, -1);
    }
    std::vector<std::string> Split(const std::string & str, const std::regex & delim, int max_count) {
        std::vector<std::string> ret;
        if (str.length() == 0 || max_count == 0) {
            return ret;
        }
        int pos = 0;
        while (true) {
            if (ret.size() + 1 == max_count) {
                break;
            }
            
            std::smatch mret;
            std::regex_search(str.cbegin() + pos, str.cend(),
                              mret, delim,
                              std::regex_constants::format_first_only);
            if (mret.size() == 0) {
                break;
            }
            
            int part_pos = pos + static_cast<int>(mret.position(0));
            int part_len = static_cast<int>(mret.length(0));
            ret.push_back(str.substr(pos, part_pos));
            pos = part_pos + static_cast<int>(part_len);
        }
        ret.push_back(str.substr(pos, str.length()));
        return ret;
    }
    
    std::string Join(const std::vector<std::string> & parts) {
        return Join(parts, "");
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
    
    int IndexOf(const std::string & str, const std::string & needle) {
        return IndexOf(str, needle, 0);
    }
    int IndexOf(const std::string & str, const std::string & needle, int pos) {
        auto p = str.find(needle, pos);
        if (p == std::string::npos) { return -1; }
        return static_cast<int>(p);
    }
    
    std::string Replace(const std::string & arg_str, const std::regex & regex,
                        const std::function<std::string(const std::string &)> & func)
    {
        std::string str = arg_str;
        int pos = 0;
        while (true) {
            std::smatch mret;
            std::regex_search(str.cbegin() + pos, str.cend(),
                              mret, regex,
                              std::regex_constants::format_first_only);
            if (mret.size() == 0) { return str; }
            
            int part_pos = pos + static_cast<int>(mret.position(0));
            int part_len = static_cast<int>(mret.length(0));
            std::string part = str.substr(part_pos, part_len);
            std::string rep = func(part);
            str.replace(part_pos, part_len, rep);
            pos = part_pos + static_cast<int>(rep.length());
        }
    }
    
    std::string GetRandomString(int len) {
        std::string chars("1234567890"
                          "abcdefghijklmnopqrstuvwxyz"
                          "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        std::default_random_engine random;
        std::uniform_int_distribution<int> distribution(0, static_cast<int>(chars.length()));
        std::stringstream ss;
        for (int i = 0; i < len; i++) {
            int dice = distribution(random);
            ss << chars[dice];
        }
        return ss.str();
    }
}

