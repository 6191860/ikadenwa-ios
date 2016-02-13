//
//  on.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/24.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <nwr/base/func.h>
#include <nwr/base/emitter.h>

namespace nwr {
namespace sio {
    class OnToken;
    
    template <typename Event> OnToken On(const EmitterPtr<Event> & emitter,
                                         const typename EventListener<Event>::element_type & listener);
    template <typename Event> OnToken On(const EmitterPtr<Event> & emitter,
                                         const EventListener<Event> & listener);
    
    class OnToken {
    public:
        OnToken();
        OnToken(const std::function<void()> & destroy_func);
        void Destroy() const;
    private:
        std::function<void()> destroy_func_;
    };
    
    template <typename Event> OnToken On(const EmitterPtr<Event> & emitter,
                                         const typename EventListener<Event>::element_type & listener)
    {
        return On(emitter, EventListenerMake<Event>(listener));
    }
    template <typename Event> OnToken On(const EmitterPtr<Event> & emitter,
                                         const EventListener<Event> & listener)
    {
        emitter->On(listener);
        auto token = OnToken([emitter, listener] {
            emitter->Off(listener);
        });
        return token;
    }
    
}
}