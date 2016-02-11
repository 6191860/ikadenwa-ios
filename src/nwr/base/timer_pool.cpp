//
//  timer_pool.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/21.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "timer_pool.h"

#include "array.h"

namespace nwr {
    TimerPool::TimerPool() {
        
    }
    TimerPool::~TimerPool() {
        for (auto timer : timers_) {
            timer->Cancel();
        }
    }
    void TimerPool::Add(const TimerPtr & timer) {
        timers_.push_back(timer);
        
        timer->end_emitter()->Once([this, timer](None none){
            Remove(timer);
        });
    }
    void TimerPool::Remove(const TimerPtr & timer) {
        nwr::Remove(timers_, timer);
    }
    void TimerPool::SetTimeout(const TimeDuration & delay, const Task & task) {
        Add(Timer::Create(delay, task));
    }
}