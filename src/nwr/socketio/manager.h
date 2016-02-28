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

#include <nwr/base/array.h>
#include <nwr/base/time.h>
#include <nwr/base/timer.h>
#include <nwr/base/emitter.h>
#include <nwr/base/none.h>
#include <nwr/base/any.h>
#include <nwr/base/any_emitter.h>
#include <nwr/engineio/socket.h>


namespace nwr {
namespace sio {
    
    class OnToken;
    class Encoder;
    class Decoder;
    class Packet;
    class Socket;
    
    class Manager: public std::enable_shared_from_this<Manager> {
    public:
        enum class ReadyState {
            Closed,
            Opening,
            Open,
            
        };
        static std::shared_ptr<Manager> Create(const std::string & uri, const eio::Socket::ConstructorParams & params);
    private:
        Manager();
        void Init(const std::string & uri, const eio::Socket::ConstructorParams & params);
    public:
        EmitterPtr<None> open_emitter() { return open_emitter_; }
        EmitterPtr<None> close_emitter() { return close_emitter_; }
        EmitterPtr<Packet> packet_emitter() { return packet_emitter_; }
        
        std::map<std::string, std::shared_ptr<Socket>> nsps() { return nsps_; }
        ReadyState ready_state() { return ready_state_; }
        bool auto_connect() { return auto_connect_; }
    private:
        void EachNsp(const std::function<void(const std::shared_ptr<Socket> &)
                     > & proc);
        void EachNsp(const std::function<void(const std::string &,
                                              const std::shared_ptr<Socket> &)
                     > & proc);
        void EmitAll(const std::string & event, const std::vector<Any> & args);
        
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
    public:
        void Open(const std::function<void(const Optional<Error> &)> & callback);
    private:
        void OnOpen();
        void OnPing();
        void OnPong();
        void OnData(const eio::PacketData & data);
        void OnDecoded(const Packet & packet);
        void OnError(const Error & error);
    public:
        std::shared_ptr<Socket> GetSocket(const std::string & nsp);
    public:
        void Destroy(const std::shared_ptr<Socket> & socket);
    public:
        void WritePacket(const Packet & packet);
    private:
        void ProcessPacketQueue();
        void Cleanup();
        void Close();
        void OnClose();
        void Reconnect();

        EmitterPtr<None> open_emitter_;
        EmitterPtr<None> close_emitter_;
        EmitterPtr<Packet> packet_emitter_;
        
        std::map<std::string, std::shared_ptr<Socket>> nsps_;
        std::vector<OnToken> subs_;

        eio::Socket::ConstructorParams params_;
        bool reconnection_;
        int reconnection_attempts_;
        TimeDuration reconnection_delay_;
        TimeDuration reconnection_delay_max_;
        double randomization_factor_;
        int backoff_;
        TimeDuration timeout_;
        ReadyState ready_state_;
        std::string uri_;
        std::vector<std::shared_ptr<Socket>> connecting_;
        Optional<std::chrono::system_clock::time_point> last_ping_;
        bool encoding_;
        std::vector<Packet> packet_buffer_;
        std::shared_ptr<Encoder> encoder_;
        std::shared_ptr<Decoder> decoder_;
        bool auto_connect_;
        bool reconnecting_;
        std::shared_ptr<eio::Socket> engine_;
        bool skip_reconnect_;

        
    };
}
}