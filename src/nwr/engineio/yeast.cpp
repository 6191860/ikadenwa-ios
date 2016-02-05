//
//  yeast.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/21.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "yeast.h"

namespace nwr {
namespace eio {
    
    
    YeastContext::YeastContext() {
        std::string str = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-_";
        for (int i = 0; i < str.length(); i++) {
            alphabet_.push_back(std::string(&str[i], 1));
        }
        length_ = 64;
        seed_ = 0;
        prev_ = "";
        for (int i = 0; i < length_; i++) {
            map_[alphabet_[i]] = i;
        }
    }
    std::string YeastContext::Encode(uint64_t num) {
        std::string ret;
        
        do {
            ret = alphabet_[num % length_] + ret;
            num = num / length_;
        } while (num > 0);
        
        return ret;
    }
    uint64_t YeastContext::Decode(const std::string & str) {
        uint64_t ret = 0;
        
        for (int i = 0; i < str.length(); i++){
            ret = ret * length_ + map_[str.substr(i, 1)];
        }
        
        return ret;
    }
    std::string YeastContext::Yeast() {
        std::string now = Encode(GetCurrentUnixTime().count());
        if (now != prev_) {
            seed_ = 0;
            prev_ = now;
            return now;
        }
        
        now = now + "." + Encode(seed_);
        seed_ += 1;
        
        return now;
    }
    
    static YeastContext shared_;
    
    std::string YeastEncode(uint64_t num) {
        return shared_.Encode(num);
    }
    
    uint64_t YeastDecode(const std::string & str) {
        return shared_.Decode(str);
    }
    
    std::string Yeast() {
        return shared_.Yeast();
    }
}
}