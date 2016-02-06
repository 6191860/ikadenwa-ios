//
//  socket_manager.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/24.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "socket_manager.h"

#include "socket.h"

namespace nwr {
namespace sio {
    
    std::shared_ptr<SocketManager> SocketManager::Create(
        const std::string & uri, const eio::Socket::ConstructorParams & params)
    {
        auto thiz = std::shared_ptr<SocketManager>(new SocketManager());
        thiz->Init(uri, params);
        return thiz;
    }
 
    SocketManager::SocketManager() {
    }
    
//    TODO: recheck
    void SocketManager::Init(
        const std::string & uri, const eio::Socket::ConstructorParams & params)
    {
        eio::Socket::ConstructorParams p = params;
        if (!p.path) {
            p.path = OptionalSome(std::string("/socket.io"));
        }
        
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
        last_ping_ = Optional<std::chrono::system_clock::time_point>();
        encoding_ = false;
        packet_buffer_ = 0;
        encoder_ = std::make_shared<Encoder>();
        decoder_ = std::make_shared<Decoder>();
        auto_connect_ = p.auto_connect;
        if (auto_connect_) {
            Open([](Optional<Error> e){});
        }
    }
    
    void SocketManager::EachNsp(const std::function<void(const std::shared_ptr<Socket> &)
                                > & proc)
    {
        auto nsps = nsps_;
        for(auto iter : nsps) {
            proc(iter.second);
        }
    }
    
    void SocketManager::EachNsp(const std::function<void(
                                                         const std::string &,
                                                         const std::shared_ptr<Socket> &)
                                > & proc)
    {
        auto nsps = nsps_;
        for(auto iter : nsps) {
            proc(iter.first, iter.second);
        }
    }
    
    void SocketManager::UpdateSocketIds() {
        EachNsp([this](const std::shared_ptr<Socket> & socket){
            socket->set_id(engine_->id());
        });        
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
    
    void SocketManager::MaybeReconnectOnOpen() {
        // Only try to reconnect if it's the first time we're connecting
        if (!reconnecting_ && reconnection_ /* TODO: && this.backoff.attempts === 0 */) {
            // keeps reconnection from firing twice for the same reconnection loop
            Reconnect();
        }
    }

    void SocketManager::EmitAll() {
//        this.emit.apply(this, arguments);
//        for (var nsp in this.nsps) {
//            if (has.call(this.nsps, nsp)) {
//                this.nsps[nsp].emit.apply(this.nsps[nsp], arguments);
//            }
//        }
    }
    
    void SocketManager::Open(const std::function<void(const Optional<Error> &)> & callback) {
        printf("[%s] ready_state = %d\n", __PRETTY_FUNCTION__ ,(int)ready_state_);
        if (ready_state_ == ReadyState::Open || ready_state_ == ReadyState::Opening) {
            return;
        }
        
        printf("[%s] uri=%s\n", __PRETTY_FUNCTION__, uri_.c_str());
        
        engine_ = eio::Socket::Create(uri_, params_);
        
        auto socket = engine_;
        ready_state_ = ReadyState::Opening;
        skip_reconnect_ = false;
        
        auto thiz = shared_from_this();
        
        // emit `open`
        OnToken open_sub = On<None>(socket->open_emitter(), [thiz, callback](None _) {
            thiz->OnOpen();
            if (callback) { callback(Optional<Error>()); }
        });
        
        // emit `connect_error`
        OnToken error_sub = On<Error>(socket->error_emitter(), [thiz, callback](const Error & error) {
            printf("[%s] connect_error\n", __PRETTY_FUNCTION__);
            
            thiz->Cleanup();
            thiz->ready_state_ = ReadyState::Closed;
            
            thiz->EachNsp([thiz, error](const std::shared_ptr<Socket> & socket){
                socket->connect_error_emitter()->Emit(error);
            });
            
            if (callback) {
                auto err = Error("connection error", "",
                                 std::make_shared<Error>(error));
                callback(OptionalSome(err));
            } else {
                // Only do this if there is no fn to handle the error
                thiz->MaybeReconnectOnOpen();
            }
        });
        
        // emit `connect_timeout`
        if (true /* TODO: timeout_enabled */) {
            auto timeout = timeout_;
            printf("[%s] connect attempt will timeout after %f\n", __PRETTY_FUNCTION__, timeout.count());
            
            // set timer
            auto timer = Timer::Create(timeout, [thiz, timeout, open_sub, socket]() {
                printf("[%s] connect attempt timed out after %f\n", __PRETTY_FUNCTION__, timeout.count());
                
                open_sub.Destroy();
                socket->Close();
                
                socket->error_emitter()->Emit(Error("timeout", ""));
                
                thiz->EachNsp([thiz, timeout](const std::shared_ptr<Socket> & socket){
                    socket->connect_timeout_emitter()->Emit(timeout);
                });
            });
            
            subs_.push_back(OnToken([timer]{
                timer->Cancel();
            }));
        }
        
        subs_.push_back(open_sub);
        subs_.push_back(error_sub);
    }
    
    void SocketManager::OnOpen() {
        printf("[%s] open\n", __PRETTY_FUNCTION__);
        
        // clear old subs
        Cleanup();
        
        // mark as open
        ready_state_ = ReadyState::Open;
        
        open_emitter_->Emit(None());
        
        // add new subs
        auto socket = engine_;
        
        auto thiz = shared_from_this();
        
        subs_.push_back(On<eio::PacketData>(socket->message_emitter(),
                                            [thiz](const eio::PacketData & data) { thiz->OnData(data); }));
        subs_.push_back(On<None>(socket->ping_emitter(),
                                 [thiz](None _) { thiz->OnPing(); }));
        subs_.push_back(On<None>(socket->pong_emitter(),
                                 [thiz](None _) { thiz->OnPong(); }));
        subs_.push_back(On<Error>(socket->error_emitter(),
                                  [thiz](const Error & error) { thiz->OnError(error); }));
        subs_.push_back(On<None>(socket->close_emitter(),
                                 [thiz](None _) { thiz->OnClose(); }));
        subs_.push_back(On<Packet>(decoder_->decoded_emitter(),
                                   [thiz](const Packet & packet) { thiz->OnDecoded(packet); }));        
    }
    
    void SocketManager::OnPing() {
        last_ping_ = OptionalSome(std::chrono::system_clock::now());
        EachNsp([](const std::shared_ptr<Socket> & socket){
            socket->ping_emitter()->Emit(None());
        });
    }
    void SocketManager::OnPong() {
        auto duration = std::chrono::duration_cast<TimeDuration>(std::chrono::system_clock::now() - last_ping_.value());
        EachNsp([duration](const std::shared_ptr<Socket> & socket){
            socket->pong_emitter()->Emit(duration);
        });
    }
    
    void SocketManager::OnData(const eio::PacketData & data) {
        
    }
    

    void SocketManager::OnError(const Error & error) {
        
    }
    void SocketManager::OnClose() {
        
    }
    void SocketManager::OnDecoded(const Packet & packet) {
        
    }
    
    void SocketManager::Reconnect() {
        
    }
    
    void SocketManager::Cleanup() {
        
    }
    
}
}