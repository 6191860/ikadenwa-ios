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
    std::shared_ptr<Json::Value> JsonParse(const std::string & str);
    std::shared_ptr<Json::Value> JsonParse(const Data & data);
    std::shared_ptr<Json::Value> JsonParse(const uint8_t * data, int size);
    std::string JsonFormat(const Json::Value & json);
}
