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
#include <nwr/base/task_queue.h>

namespace nwr {
namespace jsrtc {
    template <typename T>
    class PostTarget : public std::enable_shared_from_this<T> {
    public:
        PostTarget():
        queue_(TaskQueue::system_current_queue()),
        closed_(false)
        {}
        
        virtual ~PostTarget() {}
        
        void Post(const std::function<void(T &)> & proc) {
            auto thiz = this->shared_from_this();
            this->queue_->PostTask([thiz, proc](){
                auto target_thiz = std::static_pointer_cast<PostTarget<T>>(thiz);
                if (target_thiz->closed_) { return; }
                proc(*thiz);
            });
        }
    protected:
        void ClosePostTarget() {
            if (closed_) { return; }
            closed_ = true;
        }
    private:
        std::shared_ptr<TaskQueue> queue_;
        bool closed_;
        
    };
}
}
