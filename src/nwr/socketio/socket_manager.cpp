//
//  socket_manager.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/24.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "socket_manager.h"

namespace nwr {
namespace sio {
    
    SocketManager::ConstructorParams::ConstructorParams():
    reconnection(true),
    reconnection_attempts(-1),
    reconnection_delay(TimeDuration(1.0)),
    reconnection_delay_max(TimeDuration(5.0)),
    randomization_factor(0.5),
    timeout(TimeDuration(20.0)),
    auto_connect(true)
    {}
 
    SocketManager::SocketManager(const std::string & uri, const ConstructorParams & params) {
        ConstructorParams p = params;
        if (params.path.length() == 0) {
            p.path = "/socket.io";
        }
        
        nsps_ = 0;
        subs_ = 0;
        params_ = p;
        
        set_reconnection(p.reconnection);
        set_reconnection_attempts(p.reconnection_attempts);
        set_reconnection_delay(p.reconnection_delay);
        set_reconnection_delay_max(p.reconnection_delay_max);
        set_randomization_factor(p.randomization_factor);
        backoff_ = 0;
        set_timeout(p.timeout);
        ready_state_ = ReadyState::Closed;
        uri_ = uri;
        connecting_ = 0;
        last_ping_ = 0;
        encoding_ = false;
        packet_buffer_ = 0;
        encoder_ = 0;
        decoder_ = 0;
        auto_connect_ = p.auto_connect;
        if (auto_connect_) {
            Open();
        }
    }
    
    bool SocketManager::reconnection() {
        return reconnection_;
    }
    void SocketManager::set_reconnection(bool value) {
        reconnection_ = value;
    }
    int SocketManager::reconnection_attempts() {
        return reconnection_attempts_;
    }
    void SocketManager::set_reconnection_attempts(int value) {
        reconnection_attempts_ = value;
    }
    TimeDuration SocketManager::reconnection_delay() {
        return reconnection_delay_;
    }
    void SocketManager::set_reconnection_delay(const TimeDuration & value) {
        reconnection_delay_ = value;
    }
    TimeDuration SocketManager::reconnection_delay_max() {
        return reconnection_delay_max_;
    }
    void SocketManager::set_reconnection_delay_max(const TimeDuration & value) {
        reconnection_delay_max_ = value;
    }
    double SocketManager::randomization_factor() {
        return randomization_factor_;
    }
    void SocketManager::set_randomization_factor(double value) {
        randomization_factor_ = value;
    }
    TimeDuration SocketManager::timeout() {
        return timeout_;
    }
    void SocketManager::set_timeout(const TimeDuration & value) {
        timeout_ = value;
    }
    void SocketManager::EmitAll() {
//        this.emit.apply(this, arguments);
//        for (var nsp in this.nsps) {
//            if (has.call(this.nsps, nsp)) {
//                this.nsps[nsp].emit.apply(this.nsps[nsp], arguments);
//            }
//        }
    }
    
    void SocketManager::Open() {
        
    }
}
}