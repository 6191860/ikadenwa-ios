//
//  common.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/06.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "env.h"

namespace nwr {
    void Fatal(const std::string & message) {
        printf("[Fatal] %s\n", message.c_str());
        std::abort();
    }
}