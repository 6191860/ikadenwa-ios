//
//  any_emitter.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/11.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <functional>
#include <initializer_list>

#include "func.h"
#include "any.h"

namespace nwr {
    class AnyEmitter;
    
    using AnyEmitterPtr = std::shared_ptr<AnyEmitter>;
    
    using AnyEventListener = Func<void (const std::vector<Any> &)>;

    AnyEventListener AnyEventListenerMake(const std::function<void (const std::vector<Any> &)> & func);
    
    AnyEventListener AnyEventListenerMake(const std::function<void ()> & func);
    AnyEventListener AnyEventListenerMake(const std::function<void (const Any &)> & func);
    AnyEventListener AnyEventListenerMake(const std::function<void (const Any &, const Any &)> & func);
    AnyEventListener AnyEventListenerMake(const std::function<void (const Any &, const Any &, const Any &)> & func);

    
    class AnyEmitter {
    public:
        virtual ~AnyEmitter();
        
        std::vector<AnyEventListener> GetListenersFor(const std::string & event);
        
        void On(const std::string & event, const AnyEventListener::element_type & listener);
        void On(const std::string & event, const AnyEventListener & listener);
        void Once(const std::string & event, const AnyEventListener::element_type & listener);
        void Once(const std::string & event, const AnyEventListener & listener);
        void Off(const std::string & event, const AnyEventListener & listener);
        void RemoveAllListenersFor(const std::string & event);
        void RemoveAllListeners();
        
        void Emit(const std::string & event, const std::vector<Any> & args);
    private:
        std::map<std::string, std::vector<AnyEventListener>> listeners_map_;
    };
}
