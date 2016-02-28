//
//  task_queue.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/18.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <functional>
#include <memory>

#include "task.h"

namespace nwr {
    class TaskQueue: public std::enable_shared_from_this<TaskQueue> {
    public:
        virtual ~TaskQueue() {}
        virtual void PostTask(const Task & task) = 0;
        
        static std::shared_ptr<TaskQueue> current_queue();
    };
}
