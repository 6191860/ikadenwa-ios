//
//  looper.h
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/19.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include "task_queue.h"

namespace nwr {
    class Looper: public TaskQueue {
    public:
        virtual ~Looper() {}
        virtual void Quit() = 0;
        
        static std::shared_ptr<Looper> Create();
    };
}
