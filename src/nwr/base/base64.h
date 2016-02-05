//
//  base64.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/20.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include "data.h"

namespace nwr {
    void Base64Encode(const Data & data, Data & dest_str);
    int Base64CalcDecodedSize(const Data & str);
    void Base64Decode(const Data & str, Data & dest_data);
}

