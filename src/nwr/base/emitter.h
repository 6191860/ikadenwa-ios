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
    
    template <typename Event> using EventListener = std::function<void(const Event &)>;
    template <typename Event> using EventListenerPtr = std::shared_ptr<EventListener<Event>>;

    template <typename Event> class Emitter {
    private:
        using Listener = EventListener<Event>;
        using ListenerPtr = EventListenerPtr<Event>;
        using ListenerPtrPtr = std::shared_ptr<ListenerPtr>;
    public:
        Emitter() {}
        ~Emitter() {}
        
        std::vector<EventListenerPtr<Event>> listeners() {
            return listeners_;
        }
        
        void On(const EventListener<Event> & listener) {
            On(std::make_shared<EventListener<Event>>(listener));
        }
        void On(const EventListenerPtr<Event> & listener) {
            listeners_.push_back(listener);
        }
        
        void Once(const EventListener<Event> & listener) {
            Once(std::make_shared<EventListener<Event>>(listener));
        }
        void Once(const EventListenerPtr<Event> & listener) {
            ListenerPtrPtr on_handler_ptr = std::make_shared<ListenerPtr>(nullptr);
            
            ListenerPtr on_handler =
            std::make_shared<Listener>([this, on_handler_ptr, listener](const Event & event){
                this->Off(*on_handler_ptr);
                (*listener)(event);
            });
            
            *on_handler_ptr = on_handler;
                        
            this->On(on_handler);
        }
        
        void Off(const EventListenerPtr<Event> & listener) {
            std::remove_if(listeners_.begin(), listeners_.end(),
                           [listener](const EventListenerPtr<Event> & elem){
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
        std::vector<EventListenerPtr<Event>> listeners_;
    };
}

