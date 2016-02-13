//
//  logged_in_info.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/13.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <string>
#include <map>

#include <nwr/base/optional.h>
#include <nwr/base/any.h>

namespace nwr {
namespace ert {
    struct LoggedInInfo {
        Optional<std::string> username;
        std::map<std::string, Any> api_field;
    };
}
}
