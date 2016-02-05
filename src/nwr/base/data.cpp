//
//  data.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/21.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "data.h"

namespace nwr {
    std::string ToString(const Data & data) {
        return std::string((const char *)&data[0], data.size());
    }
}
