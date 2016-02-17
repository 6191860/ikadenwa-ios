//
//  func.h
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/13.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <memory>
#include <functional>

#include "type_helper.h"

namespace nwr {    
    template <typename F> using Func = std::shared_ptr<std::function<F>>;
    
    template <typename F>
    std::shared_ptr<typename lambda_to_function<F>::type>
    FuncMake(const F & f) {
        return std::make_shared<typename lambda_to_function<F>::type>(f);
    }
    
    template <typename F, typename ...Args>
    void FuncCall(const std::function<F> & func, Args ... args) {
        if (func) {
            func(args...);
        }
    }
    
    template <typename F, typename ...Args>
    void FuncCall(const Func<F> & func, Args ... args) {
        if (func && *func) {
            (*func)(args...);
        }
    }
    
}
