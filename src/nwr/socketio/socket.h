//
//  socket.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/24.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <string>
#include <memory>

#include <nwr/base/emitter.h>
#include <nwr/base/none.h>
#include <nwr/base/error.h>
#include <nwr/base/time.h>


namespace nwr {
namespace sio {
    
    class Socket: public std::enable_shared_from_this<Socket> {
    public:
        static std::shared_ptr<Socket> Create(int io, int nsp);
    private:
        Socket();
        void Init(int io, int nsp);
    public:
        EmitterPtr<None> connect_emitter() { return connect_emitter_; }
        EmitterPtr<Error> connect_error_emitter() { return connect_error_emitter_; }
        EmitterPtr<TimeDuration> connect_timeout_emitter() { return connect_timeout_emitter_; }
        EmitterPtr<None> connecting_emitter() { return connecting_emitter_; }
        EmitterPtr<None> disconnect_emitter() { return disconnect_emitter_; }
        EmitterPtr<None> error_emitter() { return error_emitter_; }
        EmitterPtr<None> reconnect_emitter() { return reconnect_emitter_; }
        EmitterPtr<None> reconnect_attempt_emitter() { return reconnect_attempt_emitter_; }
        EmitterPtr<None> reconnect_failed_emitter() { return reconnect_failed_emitter_; }
        EmitterPtr<None> reconnect_error_emitter() { return reconnect_error_emitter_; }
        EmitterPtr<None> reconnecting_emitter() { return reconnecting_emitter_; }
        EmitterPtr<None> ping_emitter() { return ping_emitter_; }
        EmitterPtr<TimeDuration> pong_emitter() { return pong_emitter_; }
        
        std::string id() { return id_; }
        void set_id(const std::string & value) { id_ = value; }
    private:
        EmitterPtr<None> connect_emitter_;
        EmitterPtr<Error> connect_error_emitter_;
        EmitterPtr<TimeDuration> connect_timeout_emitter_;
        EmitterPtr<None> connecting_emitter_;
        EmitterPtr<None> disconnect_emitter_;
        EmitterPtr<None> error_emitter_;
        EmitterPtr<None> reconnect_emitter_;
        EmitterPtr<None> reconnect_attempt_emitter_;
        EmitterPtr<None> reconnect_failed_emitter_;
        EmitterPtr<None> reconnect_error_emitter_;
        EmitterPtr<None> reconnecting_emitter_;
        EmitterPtr<None> ping_emitter_;
        EmitterPtr<TimeDuration> pong_emitter_;
        
        std::string id_;
    };
    
    
}
}
