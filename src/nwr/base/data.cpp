//
//  data.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/21.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "data.h"
#include "string.h"

namespace nwr {
    const char * AsCharPointer(const Data & data) {
        return reinterpret_cast<const char *>(&data[0]);
    }
    std::string ToString(const Data & data) {
        return std::string(AsCharPointer(data), data.size());
    }
    
    std::string DataFormat(const Data & data) {
        return DataFormat(&data[0], static_cast<int>(data.size()));
    }
    std::string DataFormat(const uint8_t * data, int size) {
        std::stringstream str;
        for (int i = 0; i < size; i++) {
            if (i != 0) {
                str << " ";
            }
            str << Format("%02x", data[i]);
        }
        return str.str();
    }
}
