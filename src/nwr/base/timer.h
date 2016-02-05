//
//  timer.h
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/18.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <chrono>
#include <thread>
#include <memory>

#include "task.h"
#include "none.h"
#include "time.h"
#include "emitter.h"

namespace nwr {
    class Timer;
    
    using TimerPtr = std::shared_ptr<Timer>;
    
    class TaskQueue;
    class Looper;
    
    class Timer: public std::enable_shared_from_this<Timer> {
    public:
        static TimerPtr Create(const TimeDuration & delay,
                               const Task & task);
        static TimerPtr Create(const TimeDuration & delay,
                               const TimeDuration & interval,
                               const Task & task);
        virtual ~Timer();
        int repeat_count() { return repeat_count_; }
        EmitterPtr<None> timer_emitter() { return timer_emitter_; }
        EmitterPtr<None> end_emitter() { return end_emitter_; }
        void Cancel();
    private:
        Timer();
        void Init(const TimeDuration & delay,
                  const TimeDuration & interval,
                  const Task & task);
        void ThreadMain();
        void OnTimer();
        std::shared_ptr<TaskQueue> queue_;
        TimeDuration delay_;
        TimeDuration interval_;
        
        int repeat_count_;
        TimerPtr cycle_;
        
        EmitterPtr<None> timer_emitter_;
        EmitterPtr<None> end_emitter_;
        
        std::shared_ptr<Looper> looper_;
        bool cancelled_;
    };
}
