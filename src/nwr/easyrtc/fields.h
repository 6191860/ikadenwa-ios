//
//  fields.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/21.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <map>
#include <string>
#include <nwr/base/any.h>

namespace nwr {
namespace ert {
    struct Fields {
        void Clear();
        std::map<std::string, std::map<std::string, Any>> rooms;
        std::map<std::string, Any> application;
        std::map<std::string, Any> connection;
    };
}
}
