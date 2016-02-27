//
//  post_target.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/18.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <functional>
#include <memory>
#include <nwr/base/env.h>
#include <nwr/base/task_queue.h>

#include "closable.h"

namespace nwr {
namespace jsrtc {
    template <typename T>
    class PostTarget : public std::enable_shared_from_this<T>, public ClosableImpl {
    public:
        PostTarget()
        {}
        
        virtual ~PostTarget() {}
        
        void Post(const std::function<void(T &)> & proc) {
            auto thiz = this->shared_from_this();
            this->queue()->PostTask([thiz, proc](){
                if (thiz->closed()) { return; }
                proc(*thiz);
            });
        }
        
        virtual void OnClose() = 0;
    private:
    };
}
}
