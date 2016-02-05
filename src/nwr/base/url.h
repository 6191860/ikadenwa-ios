//
//  url.h
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/16.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <cstdlib>
#include <string>
#include <sstream>
#include <cctype>
#include <map>

#include "optional.h"

namespace nwr {
    using QueryStringParams = std::map<std::string, std::string>;
    
    struct UrlParts {
        UrlParts();
        std::string scheme;
        std::string hostname;
        Optional<int> port;
        std::string path;
        QueryStringParams query;
        std::string fragment;
    };
    
    UrlParts ParseUrl(const std::string & url);
    
    std::string UrlEncode(const std::string & string);
    std::string UelDecode(const std::string & string);
    
    std::string QueryStringEncode(const QueryStringParams & params);
    QueryStringParams QueryStringDecode(const std::string & string);
}