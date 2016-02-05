//
//  url.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/16.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "url.h"
#include "util.h"

namespace nwr {
    UrlParts::UrlParts():
    scheme(),
    hostname(),
    port(),
    path(){}
    
    UrlParts ParseUrl(const std::string & url) {
        UrlParts ret = UrlParts();
        auto scheme_after = Split(url, "://", 2);
        if (scheme_after.size() == 1) {
            return ret;
        }
        ret.scheme = scheme_after[0];
        auto hostname_port_after = Split(scheme_after[1], "/", 2);
        auto hostname_port = Split(hostname_port_after[0], ":", 2);
        ret.hostname = hostname_port[0];
        if (hostname_port.size() == 2) {
            ret.port = OptionalSome(atoi(hostname_port[1].c_str()));
        }
        
        if (hostname_port_after.size() == 1) {
            hostname_port_after.push_back("/");
        } else {
            hostname_port_after[1] = "/" + hostname_port_after[1];
        }
        
        if (hostname_port_after.size() == 2) {
            auto path_fragment = Split(hostname_port_after[1], "#", 2);
            if (path_fragment.size() == 2) {
                ret.fragment = path_fragment[1];
            }
            auto path_query = Split(path_fragment[0], "?", 2);
            ret.path = path_query[0];
            if (path_query.size() == 2) {
                ret.query = QueryStringDecode(path_query[1]);
            }
        }
        return ret;
    }
    
    //  http://stackoverflow.com/questions/154536/encode-decode-urls-in-c
    
    std::string UrlEncode(const std::string & string) {
        std::ostringstream ret;

        int i = 0;
        while (true) {
            if (i >= string.length()) {
                break;
            }
            
            const char ch = string[i];
            if (isalnum(ch) || ch == '-' || ch == '_' || ch == '.' || ch == '~') {
                ret << ch;
                i += 1;
                continue;
            }
            
            ret << Format("%02X", (int)ch);
            i += 1;
        }
        
        return ret.str();
    }
    std::string UrlDecode(const std::string & string) {
        std::ostringstream ret;
        
        int i = 0;
        while (true) {
            if (i >= string.length()) {
                break;
            }
            
            const char ch = string[i];
            if (ch == '%') {
                if (i + 2 >= string.length()) {
                    break;
                }
                int hex;
                sscanf(string.substr(i + 1, 2).c_str(), "%x", &hex);
                ret << static_cast<char>(hex);
                i += 3;
            } else if (ch == '+') {
                ret << ' ';
                i += 1;
            } else {
                ret << ch;
                i += 1;
            }
        }
        
        return ret.str();
    }
    
    std::string QueryStringEncode(const QueryStringParams & params) {
        std::vector<std::string> entries;
        
        for (auto param_entry : params) {
            auto key = UrlEncode(param_entry.first);
            auto value = UrlEncode(param_entry.second);
            entries.push_back(Format("%s=%s", key.c_str(), value.c_str()));
        }
        
        return Join(entries, "&");
    }
    
    QueryStringParams QueryStringDecode(const std::string & string) {
        QueryStringParams ret;
        
        std::vector<std::string> entries = Split(string, "&");
        for (auto entry : entries) {
            std::vector<std::string> key_value = Split(entry, "=", 2);
            if (key_value.size() == 1) {
                key_value.push_back("");
            }
            auto key = UrlDecode(key_value[0]);
            auto value = UrlDecode(key_value[1]);
            
            ret[key] = value;
        }
        
        return ret;
    }
    
}