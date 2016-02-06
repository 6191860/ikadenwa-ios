//
//  socket_manager.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/24.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>

#include <nwr/base/time.h>
#include <nwr/base/timer.h>
#include <nwr/base/emitter.h>
#include <nwr/base/none.h>
#include <nwr/engineio/socket.h>

#include "parser.h"
#include "on.h"

namespace nwr {
namespace sio {
    
    //  TODO: reconnection
    
    class Socket;
    
    class SocketManager: public std::enable_shared_from_this<SocketManager> {
    public:
        enum class ReadyState {
            Closed,
            Opening,
            Open,
            
        };
        static std::shared_ptr<SocketManager> Create(const std::string & uri, const eio::Socket::ConstructorParams & params);
    private:
        SocketManager();
        void Init(const std::string & uri, const eio::Socket::ConstructorParams & params);
    private:
        void EachNsp(const std::function<void(const std::shared_ptr<Socket> &)
                     > & proc);
        void EachNsp(const std::function<void(const std::string &,
                                              const std::shared_ptr<Socket> &)
                     > & proc);
        
        void UpdateSocketIds();
        
        bool reconnection();
        void set_reconnection(bool value);
        
        int reconnection_attempts();
        void set_reconnection_attempts(int value);
        
        TimeDuration reconnection_delay();
        void set_reconnection_delay(const TimeDuration & value);
        
        TimeDuration reconnection_delay_max();
        void set_reconnection_delay_max(const TimeDuration & value);
        
        double randomization_factor();
        void set_randomization_factor(double value);
        
        TimeDuration timeout();
        void set_timeout(const TimeDuration & value);
        
        void MaybeReconnectOnOpen();

        
        void EmitAll();
        
        void Open(const std::function<void(const Optional<Error> &)> & callback);
        void OnOpen();
        void OnPing();

        
        void OnData(const eio::PacketData & data);

        void OnPong();
        void OnError(const Error & error);
        void OnClose();
        void OnDecoded(const Packet & packet);
        
        void Reconnect();
        void Cleanup();
        
        EmitterPtr<None> open_emitter_;
        
        std::map<std::string, std::shared_ptr<Socket>> nsps_;
        std::vector<OnToken> subs_;

        eio::Socket::ConstructorParams params_;
        bool reconnection_;
        int reconnection_attempts_;
        TimeDuration reconnection_delay_;
        TimeDuration reconnection_delay_max_;
        double randomization_factor_;
        int backoff_; //TODO
        TimeDuration timeout_;
        ReadyState ready_state_;
        std::string uri_;
        int connecting_;
        Optional<std::chrono::system_clock::time_point> last_ping_;
        bool encoding_;
        int packet_buffer_;
        std::shared_ptr<Encoder> encoder_;
        std::shared_ptr<Decoder> decoder_;
        bool auto_connect_;
        
        bool reconnecting_;
        std::shared_ptr<eio::Socket> engine_;
        bool skip_reconnect_;
        
    };
}
}