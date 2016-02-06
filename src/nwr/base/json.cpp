//
//  json.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/24.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "json.h"

namespace nwr {
    Optional<Json::Value> JsonParse(const std::string & str) {
        auto ret = OptionalSome(Json::Value());
        Json::Reader reader;
        if (!reader.parse(str, ret.value())){
            return Optional<Json::Value>();
        }
        return ret;
    }
    Optional<Json::Value> JsonParse(const Data & data) {
        auto ret = OptionalSome(Json::Value());
        Json::Reader reader;
        const char * begin = AsCharPointer(data);
        if (!reader.parse(begin, begin + data.size(), ret.value())) {
            return Optional<Json::Value>();
        }
        return ret;
    }
    std::string JsonFormat(const Json::Value & json) {
        return rtc::JsonValueToString(json);
    }
}