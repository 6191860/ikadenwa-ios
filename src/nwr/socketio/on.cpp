//
//  on.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/24.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "on.h"

namespace nwr {
namespace sio {
    OnToken::OnToken(){}
    
    OnToken::OnToken(const std::function<void()> & destroy_func):
    destroy_func_(destroy_func)
    {}
    
    void OnToken::Destroy() const{
        FuncCall(destroy_func_);
    }
}
}