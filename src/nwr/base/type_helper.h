//
//  type_helper.h
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/17.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <functional>

namespace nwr {
    template <typename T>
    struct functor_func_type { using type = void; };
    
    template<typename R, typename K, typename ...Args>
    struct functor_func_type<R(K::*)(Args...) const> {
        using type = std::function<R(Args...)>;
    };
    
    template <typename T>
    struct functor_return_type { using type = void; };
    
    template <typename R, typename K, typename ...Args>
    struct functor_return_type<R(K::*)(Args...) const> {
        using type = R;
    };
    
    template <typename LM>
    struct lambda_to_function {
        using type = typename functor_func_type<decltype(&LM::operator())>::type;
    };

}
