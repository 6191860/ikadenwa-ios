//
//  task_queue.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/18.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "task_queue.h"

#include "ios_task_queue.h"

namespace nwr {
    std::shared_ptr<TaskQueue> TaskQueue::system_current_queue() {
        return IosTaskQueue::current_queue();
    }
}
