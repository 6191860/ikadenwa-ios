//
//  util.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/26.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <nwr/base/url.h>
#include <nwr/base/map.h>

namespace nwr {
namespace sio0 {
    std::string MergeQuery(const std::string & base, const std::string & add);
}
}
