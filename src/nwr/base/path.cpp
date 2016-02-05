//
//  path.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/21.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "path.h"

namespace nwr {
    std::string PathAppendSlash(const std::string & path) {
        if (path.length() == 0) { return "/"; }
        
        if (path[path.length()-1] == '/') {
            return path;
        }
        
        return path + "/";
    }
}