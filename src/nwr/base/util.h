//
//  util.h
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/16.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <string>
#include <vector>
#include <cstdlib>

#include "data.h"

namespace nwr {
    
    std::string Format(const char * format, ...);
    std::string FormatV(const char * foramt, va_list ap);
    
    [[noreturn]] void Fatal(const std::string & message);
    
    std::vector<std::string> Split(const std::string & str, const std::string & delim);
    std::vector<std::string> Split(const std::string & str, const std::string & delim, int max_count);
    
    std::string Join(const std::vector<std::string> & parts, const std::string & glue);
    
    const uint8_t * AsDataPointer(const std::string & string);
    Data ToData(const std::string & string);
}
