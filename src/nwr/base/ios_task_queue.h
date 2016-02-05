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
        IosTaskQueue(NSOperationQueue * operation_queue);
        virtual ~IosTaskQueue() {}
        virtual void PostTask(const Task & task);
        static std::shared_ptr<IosTaskQueue> current_queue();
    private:
        NSOperationQueue * operation_queue_;
    };
}