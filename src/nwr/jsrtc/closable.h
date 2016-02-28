//
//  Closable.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/27.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <memory>
#include <nwr/base/env.h>
#include <nwr/base/task_queue.h>

namespace nwr {
namespace jsrtc {
    
    class Closable {
    public:
        virtual ~Closable(){}
        virtual bool closed() const = 0;
        virtual void OnClose() = 0;
        virtual void Close() = 0;
    };
    
    class ClosableImpl : public Closable {
    public:
        ClosableImpl();
        ClosableImpl(const std::shared_ptr<TaskQueue> & queue);
        virtual ~ClosableImpl();
        bool closed() const override;
        std::shared_ptr<TaskQueue> queue() const;
        void Close() override;
        void CheckTaskQueue();
    protected:
        void OnClose() override = 0;
    private:
        bool closed_;
        std::shared_ptr<TaskQueue> queue_;
    };
}
}
