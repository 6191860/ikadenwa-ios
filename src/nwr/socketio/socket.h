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
#include <functional>

#include <nwr/base/emitter.h>
#include <nwr/base/none.h>
#include <nwr/base/error.h>
#include <nwr/base/time.h>

namespace nwr {
namespace sio {
    
    class OnToken;
    class Packet;
    class Manager;
    
    class Socket: public std::enable_shared_from_this<Socket> {
    public:
        static std::shared_ptr<Socket> Create(Manager * io, const std::string & nsp);
    private:
        Socket();
        void Init(Manager * io, const std::string & nsp);
    public:
        EmitterPtr<None> connect_emitter() { return connect_emitter_; }
        EmitterPtr<Error> connect_error_emitter() { return connect_error_emitter_; }
        EmitterPtr<TimeDuration> connect_timeout_emitter() { return connect_timeout_emitter_; }
        EmitterPtr<None> connecting_emitter() { return connecting_emitter_; }
        EmitterPtr<None> disconnect_emitter() { return disconnect_emitter_; }
        EmitterPtr<Error> error_emitter() { return error_emitter_; }
        EmitterPtr<None> reconnect_emitter() { return reconnect_emitter_; }
        EmitterPtr<None> reconnect_attempt_emitter() { return reconnect_attempt_emitter_; }
        EmitterPtr<None> reconnect_failed_emitter() { return reconnect_failed_emitter_; }
        EmitterPtr<None> reconnect_error_emitter() { return reconnect_error_emitter_; }
        EmitterPtr<None> reconnecting_emitter() { return reconnecting_emitter_; }
        EmitterPtr<None> ping_emitter() { return ping_emitter_; }
        EmitterPtr<TimeDuration> pong_emitter() { return pong_emitter_; }
        EmitterPtr<Packet> packet_emitter() { return packet_emitter_; }
        
        std::string id() { return id_; }
        void set_id(const std::string & value) { id_ = value; }
    private:
        void SubEvents();
        void Open();
//        void Send();
        void Emit(const std::string & ev, const Packet & packet);
        void OnOpen();
        void OnPacket(const Packet & packet);
        void OnClose();
        
        
        EmitterPtr<None> connect_emitter_;
        EmitterPtr<Error> connect_error_emitter_;
        EmitterPtr<TimeDuration> connect_timeout_emitter_;
        EmitterPtr<None> connecting_emitter_;
        EmitterPtr<None> disconnect_emitter_;
        EmitterPtr<Error> error_emitter_;
        EmitterPtr<None> reconnect_emitter_;
        EmitterPtr<None> reconnect_attempt_emitter_;
        EmitterPtr<None> reconnect_failed_emitter_;
        EmitterPtr<None> reconnect_error_emitter_;
        EmitterPtr<None> reconnecting_emitter_;
        EmitterPtr<None> ping_emitter_;
        EmitterPtr<TimeDuration> pong_emitter_;
        EmitterPtr<Packet> packet_emitter_;
        
        std::string id_;
        Manager * io_;
        std::string nsp_;
        int ids_;
        int acks_;
        int receive_buffer_;
        int send_buffer_;
        bool connected_;
        bool disconnected_;
        std::vector<OnToken> subs_;
    };
    
    
}
}
