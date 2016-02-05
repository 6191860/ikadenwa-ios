//
//  ios_task_queue.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/18.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "ios_task_queue.h"

namespace nwr {
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
        return std::make_shared<IosTaskQueue>([NSOperationQueue currentQueue]);
    }
}