//
//  any_func.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/21.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "any_func.h"

namespace nwr {
    std::function<Any(const std::vector<Any> &)> AnyFunc::AsFunction() const {
        auto thiz = shared_from_this();
        return [thiz](const std::vector<Any> & args){
            return thiz->Call(args);
        };
    }
    
    AnyFunc::AnyFunc() {}
}