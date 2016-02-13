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
#include <cctype>
#include <regex>
#include <functional>

#include "data.h"

namespace nwr {
    
    std::string Format(const char * format, ...);
    std::string FormatV(const char * foramt, va_list ap);
    
    std::vector<std::string> Split(const std::string & str, const std::string & delim);
    std::vector<std::string> Split(const std::string & str, const std::string & delim, int max_count);
    
    std::string Join(const std::vector<std::string> & parts, const std::string & glue);
    
    const uint8_t * AsDataPointer(const std::string & string);
    Data ToData(const std::string & string);
    
    bool IsDigit(const std::string & str);
    
    int IndexOf(const std::string & str, const std::string & needle);
    int IndexOf(const std::string & str, const std::string & needle, int pos);
    
    std::string Replace(const std::string & str, const std::regex & regex,
                        const std::function<std::string(const std::string &)> & func);
}
