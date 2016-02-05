//
//  transport.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/20.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

//  https://github.com/socketio/engine.io-client

#pragma once

#include <string>
#include <vector>
#include <memory>

#include <nwr/base/url.h>
#include <nwr/base/emitter.h>
#include <nwr/base/none.h>
#include <nwr/base/error.h>
#include <nwr/base/websocket.h>

#include "parser.h"

namespace nwr {
namespace eio {
    
    class Transport {
    public:
        struct ConstructorParams {
            ConstructorParams();
            
            std::string path;
            std::string hostname;
            int port;
            bool secure;
            QueryStringParams query;
            std::string timestamp_param;
            bool timestamp_requests;
            std::string origin;
            std::string agent;
        };
        enum class ReadyState {
            None,
            Closed,
            Opening,
            Open
        };
    protected:
        Transport(const ConstructorParams & params);
    public:
        static std::shared_ptr<Transport> Create(const std::string & name, const ConstructorParams & params);
        virtual ~Transport();
        
        EmitterPtr<Error> error_emitter() { return error_emitter_; }
        EmitterPtr<None> open_emitter() { return open_emitter_; }
        EmitterPtr<Packet> packet_emitter() { return packet_emitter_; }
        EmitterPtr<None> close_emitter() { return close_emitter_; }
        EmitterPtr<None> flush_emitter() { return flush_emitter_; }
        EmitterPtr<None> drain_emitter() { return drain_emitter_; }
        
        virtual std::string name() = 0;
        QueryStringParams & query_ref() { return query_; }
        bool writable() { return writable_; }
        
        virtual void OnError(const std::string & msg);
        virtual void Open();
        virtual void Close();

        virtual void RemoveAllListeners();
        
        virtual void Send(const std::vector<Packet> & packets);
    protected:
        virtual void OnOpen();
        virtual void OnData(const Websocket::Message & data);
        virtual void OnPacket(const Packet & packet);
        virtual void OnClose();
        
        virtual void DoOpen() = 0;
        virtual void DoClose() = 0;
        virtual void Write(const std::vector<Packet> & packets) = 0;

        std::string path_;
        std::string hostname_;
        int port_;
        bool secure_;
        QueryStringParams query_;
        std::string timestamp_param_;
        bool timestamp_requests_;
        ReadyState ready_state_;
        std::string origin_;
        std::string agent_;
        
        bool writable_;
        
        EmitterPtr<Error> error_emitter_;
        EmitterPtr<None> open_emitter_;
        EmitterPtr<Packet> packet_emitter_;
        EmitterPtr<None> close_emitter_;
        EmitterPtr<None> flush_emitter_;
        EmitterPtr<None> drain_emitter_;
    };
    
}
}
