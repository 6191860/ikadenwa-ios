//
//  Closable.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/27.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "closable.h"

namespace nwr {
namespace jsrtc {
    ClosableImpl::ClosableImpl():
    closed_(false),
    queue_(TaskQueue::current_queue())
    {}
    
    ClosableImpl::ClosableImpl(const std::shared_ptr<TaskQueue> & queue):
    closed_(false),
    queue_(queue)
    {}
    
    ClosableImpl::~ClosableImpl() {
        Close();
    }
    bool ClosableImpl::closed() const {
        return closed_;
    }
    std::shared_ptr<TaskQueue> ClosableImpl::queue() const {
        return queue_;
    }
    void ClosableImpl::Close() {
        if (!closed_) {
            CheckTaskQueue();
            OnClose();
            closed_ = true;
        }
    }
    void ClosableImpl::CheckTaskQueue() {
        if (queue_ != TaskQueue::current_queue()) {
            Fatal("invalid task queue");
        }
    }
}
}