//
//  emitter.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/20.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

//  https://github.com/component/emitter

#pragma once

#include <vector>
#include <functional>
#include <memory>

namespace nwr {
    template <typename Event> class Emitter;
    template <typename Event> using EmitterPtr = std::shared_ptr<Emitter<Event>>;

    template <typename Event> using EventListener = std::shared_ptr<std::function<void (const Event &)>>;
    template <typename Event>
    EventListener<Event> EventListenerMake(const std::function<void (const Event &)> & func) {
        return std::make_shared<typename EventListener<Event>::element_type>(func);
    }
    
    template <typename Event> class Emitter {
    public:
        Emitter() {}
        ~Emitter() {}
        
        std::vector<EventListener<Event>> listeners() {
            return listeners_;
        }
        
        void On(const typename EventListener<Event>::element_type & listener) {
            On(EventListenerMake(listener));
        }
        
        void On(const EventListener<Event> & listener) {
            listeners_.push_back(listener);
        }
        
        void Once(const typename EventListener<Event>::element_type & listener) {
            Once(EventListenerMake(listener));
        }
        
        void Once(const EventListener<Event> & listener) {
            auto on_handler_ptr = std::make_shared<EventListener<Event>>();
            
            auto on_handler = EventListenerMake<Event>([this, on_handler_ptr, listener](const Event & event){
                this->Off(*on_handler_ptr);
                (*listener)(event);
            });
            
            *on_handler_ptr = on_handler;
                        
            this->On(on_handler);
        }
        
        void Off(const EventListener<Event> & listener) {
            std::remove_if(listeners_.begin(), listeners_.end(),
                           [listener](const EventListener<Event> & elem){
                               return listener == elem;
                           });
        }
        void RemoveAllListeners() {
            for (auto listener : listeners()) {
                Off(listener);
            }
        }
        
        void Emit(const Event & event) {
            for (auto listener : listeners()) {
                (*listener)(event);
            }
        }
    private:
        std::vector<EventListener<Event>> listeners_;
    };
}

