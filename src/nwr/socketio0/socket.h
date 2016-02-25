//
//  socket.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/25.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <vector>
#include <memory>
#include <limits>
#include <map>
#include <nwr/base/optional.h>
#include <nwr/base/time.h>
#include <nwr/base/timer.h>
#include <nwr/base/func.h>
#include <nwr/base/map.h>
#include <nwr/base/http_operation.h>
#include <nwr/base/any_emitter.h>

#include "parser.h"

namespace nwr {
namespace sio0 {
    class Socket;
    class Transport;
    
    struct SocketOptions {
        SocketOptions();
        
        Optional<std::string> host;
        Optional<bool> secure;
        Optional<int> port;
        Optional<std::string> query;
//        Optional<std::string> document;
        Optional<std::string> resource;
//        transports
        Optional<TimeDuration> connect_timeout;
//        Optional<bool> try_multiple_transports;
        Optional<bool> reconnect;
        Optional<TimeDuration> reconnection_delay;
        Optional<TimeDuration> reconnection_limit;
        Optional<TimeDuration> reopen_delay;
        Optional<int> max_reconnection_attempts;
//        Optional<bool> sync_disconnect_on_unload;
        Optional<bool> auto_connect;
        // flash policy port
        Optional<bool> manual_flush;
        Optional<bool> force_new_connection;
    };
    
    class CoreSocket : public std::enable_shared_from_this<CoreSocket> {
    public:
        static std::shared_ptr<CoreSocket> Create(const SocketOptions & options);
        void Init(const SocketOptions & options);
        ~CoreSocket();
        
        AnyEmitterPtr emitter();
        bool connected();
        bool connecting();
        bool reconnecting();
        TimeDuration close_timeout();
        const SocketOptions & options();
    private:
        CoreSocket();
    public:
        std::shared_ptr<Socket> Of(const std::string & name);
    private:
        void Publish(const std::string & event, const std::vector<Any> & args);
        void Handshake(const std::function<void(const std::vector<std::string> &)> & fn);
        std::shared_ptr<Transport> GetTransport();
        void Connect(const std::function<void()> & fn);
    public:
        void SetHeartbeatTimeout();
        void SendPacket(const Packet & packet);
        void SetBuffer(bool v);
    private:
        void FlushBuffer();
    public:
        void Disconnect();

        void OnConnect();
        void OnOpen();
        void OnClose();
        void OnPacket(const Packet & packet);
        void OnError(const std::string & error);
        void OnError(const std::string & reason, const std::string & advice);
        void OnDisconnect(const std::string & reason);
        // void Reconnect();
    private:
        AnyEmitterPtr emitter_;
        SocketOptions options_;
        bool connected_;
        bool open_;
        bool connecting_;
        bool reconnecting_;
        std::map<std::string, std::shared_ptr<Socket>> namespaces_;
        std::vector<Packet> buffer_;
        bool do_buffer_;
        std::string session_id_;
        TimeDuration close_timeout_;
        TimeDuration heartbeat_timeout_;
        std::shared_ptr<Transport> transport_;
        TimerPtr connect_timeout_timer_;
        TimerPtr heartbeat_timeout_timer_;
        
#warning todo clear namespaces to break cycle
    };
}
}
