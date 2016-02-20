//
//  any_func_forward.h
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/21.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <memory>
#include <vector>
#include <functional>

namespace nwr {
    class Any;
    class AnyFunc;
    
    using AnyFuncPtr = std::shared_ptr<AnyFunc>;
    template <typename F> class AnyFuncImpl;
    template <typename F> AnyFuncPtr AnyFuncMake(const F & func);
    
    class AnyFunc : public std::enable_shared_from_this<AnyFunc> {
    public:
        virtual Any Call(const std::vector<Any> & args) const = 0;
        std::function<Any(const std::vector<Any> &)> AsFunction() const;
    protected:
        AnyFunc();
    };
}