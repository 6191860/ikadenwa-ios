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
        return JsonParse(reinterpret_cast<const uint8_t *>(&str[0]),
                         static_cast<int>(str.length()));
    }
    Optional<Json::Value> JsonParse(const Data & data) {
        return JsonParse(&data[0], static_cast<int>(data.size()));
    }
    Optional<Json::Value> JsonParse(const uint8_t * data, int size) {
        auto ret = Some(Json::Value());
        Json::Reader reader;
        const char * p = reinterpret_cast<const char *>(data);
        if (!reader.parse(p, p + size, *ret)) {
            return None();
        }
        return ret;
    }
    std::string JsonFormat(const Json::Value & json) {
        return rtc::JsonValueToString(json);
    }
}