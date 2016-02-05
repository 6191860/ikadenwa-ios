//
//  timer.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/18.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "timer.h"

#include "task_queue.h"
#include "looper.h"

namespace nwr {
    TimerPtr Timer::Create(const TimeDuration & delay, const Task & task) {
        return Create(delay, std::chrono::duration<double>(-1), task);
    }
    TimerPtr Timer::Create(const TimeDuration & delay,
                           const TimeDuration & interval,
                           const Task & task)
    {
        TimerPtr thiz(new Timer());
        thiz->Init(delay, interval, task);
        return thiz;
    }
    Timer::~Timer() {
        Cancel();
    }
    void Timer::Cancel() {
        if (cancelled_) {
            return;
        }
        cancelled_ = true;
    }
    Timer::Timer():
    timer_emitter_(std::make_shared<Emitter<None>>()),
    end_emitter_(std::make_shared<Emitter<None>>())
    {}
    
    void Timer::Init(const TimeDuration & delay,
                     const TimeDuration & interval,
                     const Task & task)
    {
        queue_ = TaskQueue::system_current_queue();
        delay_ = delay;
        interval_ = interval;
        repeat_count_ = 0;
        cycle_ = shared_from_this();
        looper_ = Looper::Create();
        cancelled_ = false;
        
        timer_emitter_->On([task](None none){
            task();
        });
        
        looper_->PostTask([this]{
            std::this_thread::sleep_for(this->delay_);
            queue_->PostTask(std::bind(&Timer::OnTimer, this));
        });
    }
    void Timer::OnTimer() {
        if (!cancelled_) {
            repeat_count_ += 1;
            timer_emitter_->Emit(None());
        }
        
        if (interval_ < std::chrono::seconds(0) || cancelled_) {
            looper_->Quit();
            looper_ = nullptr;
            
            end_emitter_->Emit(None());
            
            cycle_ = nullptr;
            return;
        }
        
        looper_->PostTask([this]{
            std::this_thread::sleep_for(this->interval_);
            queue_->PostTask(std::bind(&Timer::OnTimer, this));
        });
    }
}

