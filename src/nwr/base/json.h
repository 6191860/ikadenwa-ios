//
//  json.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/24.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <memory>

#include <webrtc/base/json.h>

#include "data.h"
#include "optional.h"

namespace nwr {
    Optional<Json::Value> JsonParse(const std::string & str);
    Optional<Json::Value> JsonParse(const Data & data);
    Optional<Json::Value> JsonParse(const uint8_t * data, int size);
    std::string JsonFormat(const Json::Value & json);
}
