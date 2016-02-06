//
//  url.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/24.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "url.h"

#include "env.h"
#include "string.h"

namespace nwr {
namespace sio {
 
    Url UrlMake(const std::string & uri, int loc) {
        Url ret;
        
        auto url_part = ParseUrl(uri);
        
        if (!url_part.port) {
            if (url_part.scheme == "http" || url_part.scheme == "ws") {
                ret.port = 80;
            } else if (url_part.scheme == "https" || url_part.scheme == "wss") {
                ret.port = 443;
            }
        }
        
        if (url_part.path.length() == 0) {
            ret.path = "/";
        } else {
            ret.path = url_part.path;
        }

        bool ipv6 = url_part.hostname.find(":") != std::string::npos;
        
        std::string host = ipv6 ? "[" + url_part.hostname + "]" : url_part.hostname;

        // define unique id
        ret.id = Format("%s://%s:%d", url_part.scheme.c_str(), host.c_str(), ret.port);
        
        // define href
        ret.href = Format("%s://%s:%d", url_part.scheme.c_str(), host.c_str(), ret.port);

        ret.protocol = url_part.scheme;
        ret.hostname = url_part.hostname;
        ret.query = url_part.query;
        ret.fragment = url_part.fragment;
        
        return ret;
    }


}
}