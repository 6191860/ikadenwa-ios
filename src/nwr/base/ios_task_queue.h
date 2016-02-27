//
//  ios_task_queue.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/18.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <map>
#include <memory>
#include <mutex>

#import <Foundation/Foundation.h>

#include "map.h"
#include "task_queue.h"

namespace nwr {
    class IosTaskQueue: public TaskQueue {
    public:
        IosTaskQueue(NSOperationQueue * operation_queue);
        virtual ~IosTaskQueue();
        
        NSOperationQueue * inner_queue() { return operation_queue_; }
        
        void PostTask(const Task & task) override;
        static std::shared_ptr<IosTaskQueue> current_queue();
    private:
        NSOperationQueue * operation_queue_;
        
        static std::map<NSOperationQueue*, std::weak_ptr<IosTaskQueue>> cache_;
        static std::mutex cache_mutex_;
    };
}