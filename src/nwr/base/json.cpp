//
//  json.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/24.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "json.h"

namespace nwr {
    std::shared_ptr<Json::Value> JsonParse(const std::string & str) {
        auto ret = std::make_shared<Json::Value>();
        Json::Reader reader;
        if (!reader.parse(str, *ret)){
            return nullptr;
        }
        return ret;
    }
    std::shared_ptr<Json::Value> JsonParse(const Data & data) {
        auto ret = std::make_shared<Json::Value>();
        Json::Reader reader;
        const char * begin = reinterpret_cast<const char *>(&data[0]);
        if (!reader.parse(begin, begin + data.size(), *ret)) {
            return nullptr;
        }
        return ret;
    }
    std::string JsonFormat(const Json::Value & json) {
        return rtc::JsonValueToString(json);
    }
}