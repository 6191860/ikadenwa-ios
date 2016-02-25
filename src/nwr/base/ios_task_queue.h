//
//  ios_task_queue.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/18.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#import <Foundation/Foundation.h>

#include "task_queue.h"

namespace nwr {
    class IosTaskQueue: public TaskQueue {
    public:
        NSOperationQueue * inner_queue() { return operation_queue_; }
        
        IosTaskQueue(NSOperationQueue * operation_queue);
        virtual ~IosTaskQueue() {}
        void PostTask(const Task & task) override;
        static std::shared_ptr<IosTaskQueue> current_queue();
    private:
        NSOperationQueue * operation_queue_;
    };
}