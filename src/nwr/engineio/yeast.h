//
//  yeast.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/21.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

//  https://github.com/unshiftio/yeast

#pragma once

#include <string>
#include <vector>
#include <map>

#include <nwr/base/time.h>

namespace nwr {
namespace eio {
    
    class YeastContext {
    public:
        YeastContext();
        std::string Encode(uint64_t num);
        uint64_t Decode(const std::string & str);
        std::string Yeast();
    private:
        std::vector<std::string> alphabet_;
        int length_;
        std::map<std::string, int> map_;
        uint64_t seed_;
        std::string prev_;        
    };
    
    std::string YeastEncode(uint64_t num);
    uint64_t YeastDecode(const std::string & str);
    std::string Yeast();
    
}
}
