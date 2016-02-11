//
//  url.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/24.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <string>

#include <nwr/base/url.h>

namespace nwr {
namespace sio {
    
    struct Url {
        std::string source;
        
        std::string protocol;
        std::string hostname;
        int port;
        std::string path;
        QueryStringParams query;
        std::string fragment;
        
        std::string id;
        std::string href;
    };
    
    Url UrlMake(const std::string & uri);
    
}
}
