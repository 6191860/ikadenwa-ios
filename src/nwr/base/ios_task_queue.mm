//
//  ios_task_queue.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/18.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "ios_task_queue.h"

namespace nwr {
    IosTaskQueue::~IosTaskQueue() {
        std::lock_guard<std::mutex> lk(cache_mutex_);
        cache_.erase(inner_queue());
    }
    
    IosTaskQueue::IosTaskQueue(NSOperationQueue * operation_queue):
    operation_queue_(operation_queue){
    }
    void IosTaskQueue::PostTask(const Task & task) {
        Task task_copy = task;
        [operation_queue_ addOperationWithBlock:^{
            task_copy();
        }];
    }
    std::shared_ptr<IosTaskQueue> IosTaskQueue::current_queue() {
        std::lock_guard<std::mutex> lk(cache_mutex_);
        
        NSOperationQueue * inner = [NSOperationQueue currentQueue];

        std::shared_ptr<IosTaskQueue> thiz;
        
        if (HasKey(cache_, inner)) {
            thiz = cache_[inner].lock();
            if (thiz) {
                return thiz;
            }
        }
        
        thiz = std::make_shared<IosTaskQueue>(inner);
        cache_[inner] = thiz;
        return thiz;
    }
    
    std::map<NSOperationQueue*, std::weak_ptr<IosTaskQueue>> IosTaskQueue::cache_;
    std::mutex IosTaskQueue::cache_mutex_;
}