//
//  any_emitter.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/11.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "any_emitter.h"

#include "env.h"
#include "array.h"

namespace nwr {
    AnyEventListener AnyEventListenerMake(const std::function<void (const std::vector<Any> &)> & func) {
        return std::make_shared<AnyEventListener::element_type>(func);
    }
    
    AnyEventListener AnyEventListenerMake(const std::function<void ()> & func) {
        return AnyEventListenerMake([func](const std::vector<Any> & args){
            func();
        });
    }
    AnyEventListener AnyEventListenerMake(const std::function<void (const Any &)> & func) {
        return AnyEventListenerMake([func](const std::vector<Any> & args){
            Any arg0 = 0 < args.size() ? args[0] : Any();
            func(arg0);
        });
    }
    AnyEventListener AnyEventListenerMake(const std::function<void (const Any &, const Any &)> & func) {
        return AnyEventListenerMake([func](const std::vector<Any> & args){
            Any arg0 = 0 < args.size() ? args[0] : Any();
            Any arg1 = 1 < args.size() ? args[1] : Any();
            func(arg0, arg1);
        });
    }
    AnyEventListener AnyEventListenerMake(const std::function<void (const Any &, const Any &, const Any &)> & func) {
        return AnyEventListenerMake([func](const std::vector<Any> & args){
            Any arg0 = 0 < args.size() ? args[0] : Any();
            Any arg1 = 1 < args.size() ? args[1] : Any();
            Any arg2 = 2 < args.size() ? args[2] : Any();
            func(arg0, arg1, arg2);
        });
    }

    
    AnyEmitter::~AnyEmitter() {}
    
    std::vector<AnyEventListener> AnyEmitter::GetListenersFor(const std::string & event) {
        return listeners_map_[event];
    }
    
    void AnyEmitter::On(const std::string & event, const AnyEventListener::element_type & listener) {
        On(event, AnyEventListenerMake(listener));
    }
    void AnyEmitter::On(const std::string & event, const AnyEventListener & listener) {
        listeners_map_[event].push_back(listener);
    }
    void AnyEmitter::Once(const std::string & event, const AnyEventListener::element_type & listener) {
        Once(event, AnyEventListenerMake(listener));
    }
    void AnyEmitter::Once(const std::string & event, const AnyEventListener & listener) {
        auto on_handler_ptr = std::make_shared<AnyEventListener>();
        
        auto on_handler = AnyEventListenerMake([this, event, on_handler_ptr, listener]
                                               (const std::vector<Any> & args){
            this->Off(event, *on_handler_ptr);
            (*listener)(args);
        });
        
        *on_handler_ptr = on_handler;
        
        this->On(event, on_handler);
    }
    void AnyEmitter::Off(const std::string & event, const AnyEventListener & listener) {        
        Remove(listeners_map_[event], listener);
        if (listeners_map_[event].size() == 0) {
            listeners_map_.erase(event);
        }
    }
    void AnyEmitter::RemoveAllListenersFor(const std::string & event) {
        listeners_map_.erase(event);
    }
    void AnyEmitter::RemoveAllListeners() {
        listeners_map_.clear();
    }
    void AnyEmitter::Emit(const std::string & event, const std::vector<Any> & args) {
        for (auto listener : GetListenersFor(event)) {
            (*listener)(args);
        }
    }
}