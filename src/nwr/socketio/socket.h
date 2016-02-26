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
#include <vector>
#include <map>
#include <functional>

#include <nwr/base/emitter.h>
#include <nwr/base/none.h>
#include <nwr/base/error.h>
#include <nwr/base/func.h>
#include <nwr/base/time.h>
#include <nwr/base/any.h>
#include <nwr/base/any_func.h>
#include <nwr/base/any_emitter.h>

namespace nwr {
namespace sio {
    class OnToken;
    class Packet;
    class Manager;
    
    class Socket: public std::enable_shared_from_this<Socket> {
    private:
        static std::vector<std::string> events_;
    public:
        static std::shared_ptr<Socket> Create(Manager * io, const std::string & nsp);
        AnyEmitterPtr emitter() { return emitter_; }
    private:
        struct EmitParams {
            std::string event;
            std::vector<Any> args;
        };
        
        Socket();
        void Init(Manager * io, const std::string & nsp);
    public:
        std::string id() const { return id_; }
        void set_id(const std::string & value) { id_ = value; }
        
        bool connected() const { return connected_; }
    private:
        void SubEvents();
        void Open();
        void Send(const std::vector<Any> & args);
    public:
        void Emit(const std::string & event, const std::vector<Any> & args);
    private:
        void SendPacket(Packet packet);
        void OnOpen();
        void OnClose();
        void OnPacket(const Packet & packet);
        void OnEvent(const Packet & packet);
        AnyFuncPtr MakeAck(int id);
        void OnAck(const Packet & packet);
        void OnConnect();
        void EmitBuffered();
        void OnDisconnect();
        void Destroy();
    public:
        void Close();
    private:
        AnyEmitterPtr emitter_;
        
        std::string id_;
        Manager * io_;
        std::string nsp_;
        int ids_;
        std::map<int, AnyFuncPtr> acks_;
        std::vector<EmitParams> receive_buffer_;
        std::vector<Packet> send_buffer_;
        bool connected_;
        bool disconnected_;
        std::vector<OnToken> subs_;
    };
    
    
}
}
