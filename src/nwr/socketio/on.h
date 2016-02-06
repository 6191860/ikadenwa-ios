//
//  on.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/24.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <nwr/base/emitter.h>

namespace nwr {
namespace sio {
    
    class OnToken {
    public:
        OnToken();
        OnToken(const std::function<void()> & destroy_func);
        void Destroy() const;
    private:
        std::function<void()> destroy_func_;
    };
    
    template <typename Event> OnToken On(const EmitterPtr<Event> & emitter,
                                         const EventListener<Event> & listener)
    {
        EventListenerPtr<Event> heap_listener = std::make_shared<EventListener<Event>>(listener);
        
        emitter->On(heap_listener);
        
        auto token = OnToken([emitter, heap_listener] {
            emitter->Off(heap_listener);
        });
        return token;
    }
    
}
}