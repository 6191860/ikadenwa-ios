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

namespace nwr {
    std::shared_ptr<Json::Value> JsonParse(const std::string & str);
    std::shared_ptr<Json::Value> JsonParse(const Data & data);
    std::string JsonFormat(const Json::Value & json);
}
