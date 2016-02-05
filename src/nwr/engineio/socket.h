//
//  socket.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/21.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <nwr/base/url.h>
#include <nwr/base/path.h>
#include <nwr/base/none.h>
#include <nwr/base/data.h>
#include <nwr/base/error.h>
#include <nwr/base/emitter.h>
#include <nwr/base/json.h>

#include "optional.h"
#include "parser.h"
#include "timer.h"

namespace nwr {
namespace eio {
    
    class Transport;
    
    class Socket {
    public:
        struct ConstructorParams {
            ConstructorParams();
            
            std::string origin;
            std::string agent;
            Optional<std::string> path;
            std::string timestamp_param;
            bool timestamp_requests;
        };
        enum class ReadyState {
            None,
            Opening,
            Open,
            Closed,
            Closing
        };
        Socket(const std::string & uri, const ConstructorParams & params);
        virtual ~Socket();
        
        int protocol();
        
        EmitterPtr<None> open_emitter() { return open_emitter_; };
        EmitterPtr<Packet> packet_emitter() { return packet_emitter_; };
        EmitterPtr<Optional<TimeDuration>> heartbeat_emitter() { return heartbeat_emitter_; }
        EmitterPtr<None> pong_emitter() { return pong_emitter_; }
        EmitterPtr<Data> message_emitter() { return message_emitter_; }
        EmitterPtr<Json::Value> handshake_emitter() { return handshake_emitter_; }
        EmitterPtr<None> ping_emitter() { return ping_emitter_; }
        EmitterPtr<None> drain_emitter() { return drain_emitter_; }
        EmitterPtr<None> flush_emitter() { return flush_emitter_; }
        EmitterPtr<Packet> packet_create_emitter() { return packet_create_emitter_; }
        EmitterPtr<Error> error_emitter() { return error_emitter_; }
        EmitterPtr<None> close_emitter() { return close_emitter_; }
    private:
        std::shared_ptr<Transport> CreateTransport(const std::string & name);
        
        void Open();
        
        std::shared_ptr<Transport> transport() { return transport_; }
        void set_transport(const std::shared_ptr<Transport> & transport);
    
    public:
        void OnOpen();
    private:
        void OnPacket(const Packet & packet);
        void OnHandshake(const Json::Value & json);
        void OnHeartbeat(const Optional<TimeDuration> & timeout);
        void SetPing();
        void Ping();
        void OnDrain();
        void Flush();
    public:
        void Send(const Data & data);
        void Send(const Data & data, int options, std::function<void()> callback);
    private:
        void SendPacket(PacketType type, const Data & data, int options, std::function<void()> callback);
    public:
        void Close();
    private:
        void OnError(const Error & error);
        void OnClose();
        
        
        std::string hostname_;
        bool secure_;
        int port_;
        QueryStringParams query_;
        std::string path_;
        std::string origin_;
        std::string agent_;
        std::string timestamp_param_;
        bool timestamp_requests_;
        ReadyState ready_state_;
        std::vector<Packet> write_buffer_;
        std::shared_ptr<Transport> transport_;
        std::string sid_;
        TimeDuration ping_interval_;
        TimeDuration ping_timeout_;
        TimerPtr ping_timeout_timer_;
        TimerPtr ping_interval_timer_;
        int prev_buffer_len_;
        EventListenerPtr<Optional<TimeDuration>> on_heartbeat_ptr_;
        
        EmitterPtr<None> open_emitter_;
        EmitterPtr<Packet> packet_emitter_;
        EmitterPtr<Optional<TimeDuration>> heartbeat_emitter_;
        EmitterPtr<None> pong_emitter_;
        EmitterPtr<Data> message_emitter_;
        EmitterPtr<Json::Value> handshake_emitter_;
        EmitterPtr<None> ping_emitter_;
        EmitterPtr<None> drain_emitter_;
        EmitterPtr<None> flush_emitter_;
        EmitterPtr<Packet> packet_create_emitter_;
        EmitterPtr<Error> error_emitter_;
        EmitterPtr<None> close_emitter_;
        
    };
    
}
}