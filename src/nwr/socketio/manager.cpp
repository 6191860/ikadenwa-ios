//
//  socket_manager.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/24.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "manager.h"

#include "on.h"
#include "parser.h"
#include "socket.h"

namespace nwr {
namespace sio {
    
    std::shared_ptr<Manager> Manager::Create(
        const std::string & uri, const eio::Socket::ConstructorParams & params)
    {
        auto thiz = std::shared_ptr<Manager>(new Manager());
        thiz->Init(uri, params);
        return thiz;
    }
 
    Manager::Manager():
    open_emitter_(std::make_shared<decltype(open_emitter_)::element_type>()),
    close_emitter_(std::make_shared<decltype(close_emitter_)::element_type>()),
    packet_emitter_(std::make_shared<decltype(packet_emitter_)::element_type>())
    {
    }
    
//    TODO: recheck
    void Manager::Init(
        const std::string & uri, const eio::Socket::ConstructorParams & params)
    {
        eio::Socket::ConstructorParams p = params;
        if (!p.path) {
            p.path = OptionalSome(std::string("/socket.io"));
        }
        
        params_ = p;
        
        set_reconnection(p.reconnection);
        reconnection_ = false; // TODO: disabled now
        
        set_reconnection_attempts(p.reconnection_attempts);
        set_reconnection_delay(p.reconnection_delay);
        set_reconnection_delay_max(p.reconnection_delay_max);
        set_randomization_factor(p.randomization_factor);
        backoff_ = 0;
        set_timeout(p.timeout);
        ready_state_ = ReadyState::Closed;
        uri_ = uri;
        last_ping_ = Optional<std::chrono::system_clock::time_point>();
        encoding_ = false;
        encoder_ = std::make_shared<Encoder>();
        decoder_ = std::make_shared<Decoder>();
        auto_connect_ = p.auto_connect;
        if (auto_connect_) {
            Open([](Optional<Error> e){});
        }
    }
    
    void Manager::EachNsp(const std::function<void(const std::shared_ptr<Socket> &)
                                > & proc)
    {
        auto nsps = nsps_;
        for(auto iter : nsps) {
            proc(iter.second);
        }
    }
    
    void Manager::EachNsp(const std::function<void(
                                                         const std::string &,
                                                         const std::shared_ptr<Socket> &)
                                > & proc)
    {
        auto nsps = nsps_;
        for(auto iter : nsps) {
            proc(iter.first, iter.second);
        }
    }
    
    void Manager::UpdateSocketIds() {
        EachNsp([this](const std::shared_ptr<Socket> & socket){
            socket->set_id(engine_->id());
        });        
    }
    
    bool Manager::reconnection() {
        return reconnection_;
    }
    void Manager::set_reconnection(bool value) {
        reconnection_ = value;
    }
    int Manager::reconnection_attempts() {
        return reconnection_attempts_;
    }
    void Manager::set_reconnection_attempts(int value) {
        reconnection_attempts_ = value;
    }
    TimeDuration Manager::reconnection_delay() {
        return reconnection_delay_;
    }
    void Manager::set_reconnection_delay(const TimeDuration & value) {
        reconnection_delay_ = value;
    }
    TimeDuration Manager::reconnection_delay_max() {
        return reconnection_delay_max_;
    }
    void Manager::set_reconnection_delay_max(const TimeDuration & value) {
        reconnection_delay_max_ = value;
    }
    double Manager::randomization_factor() {
        return randomization_factor_;
    }
    void Manager::set_randomization_factor(double value) {
        randomization_factor_ = value;
    }
    TimeDuration Manager::timeout() {
        return timeout_;
    }
    void Manager::set_timeout(const TimeDuration & value) {
        timeout_ = value;
    }
    
    void Manager::MaybeReconnectOnOpen() {
        // Only try to reconnect if it's the first time we're connecting
        if (!reconnecting_ && reconnection_ /* TODO: && this.backoff.attempts === 0 */) {
            // keeps reconnection from firing twice for the same reconnection loop
            Reconnect();
        }
    }

    void Manager::EmitAll() {
//        this.emit.apply(this, arguments);
//        for (var nsp in this.nsps) {
//            if (has.call(this.nsps, nsp)) {
//                this.nsps[nsp].emit.apply(this.nsps[nsp], arguments);
//            }
//        }
    }
    
    void Manager::Open(const std::function<void(const Optional<Error> &)> & callback) {
        printf("[%s] ready_state = %d\n", __PRETTY_FUNCTION__ ,(int)ready_state_);
        if (ready_state_ == ReadyState::Open || ready_state_ == ReadyState::Opening) {
            return;
        }
        
        printf("[%s] uri=%s\n", __PRETTY_FUNCTION__, uri_.c_str());
        
        engine_ = eio::Socket::Create(uri_, params_);
        
        auto socket = engine_;
        ready_state_ = ReadyState::Opening;
        skip_reconnect_ = true; // TODO: reconnecting is disabled now
        
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
    
    void Manager::OnOpen() {
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
    
    void Manager::OnPing() {
        last_ping_ = OptionalSome(std::chrono::system_clock::now());
        EachNsp([](const std::shared_ptr<Socket> & socket){
            socket->ping_emitter()->Emit(None());
        });
    }
    
    void Manager::OnPong() {
        auto duration = std::chrono::duration_cast<TimeDuration>(std::chrono::system_clock::now() - last_ping_.value());
        EachNsp([duration](const std::shared_ptr<Socket> & socket){
            socket->pong_emitter()->Emit(duration);
        });
    }
    
    void Manager::OnData(const eio::PacketData & data) {
        decoder_->Add(data);
    }
    
    void Manager::OnDecoded(const Packet & packet) {
        packet_emitter_->Emit(packet);
    }

    void Manager::OnError(const Error & error) {
        printf("[%s] %s\n", __PRETTY_FUNCTION__, error.Dump().c_str());
        EachNsp([error](const std::shared_ptr<Socket> & socket) {
            socket->error_emitter()->Emit(error);
        });
    }
    
    std::shared_ptr<Socket> Manager::GetSocket(const std::string & nsp) {
        auto socket_ptr = std::make_shared<std::shared_ptr<Socket>>();
        
        auto thiz = shared_from_this();
        auto on_connecting = EventListenerMake<None>([thiz, socket_ptr] (None _) {
            if (std::find(thiz->connecting_.begin(), thiz->connecting_.end(),
                          *socket_ptr) == thiz->connecting_.end())
            {
                thiz->connecting_.push_back(*socket_ptr);
            }
        });
        
        std::shared_ptr<Socket> socket;
        *socket_ptr = socket = nsps_[nsp];
        
        if (!socket) {
            *socket_ptr = socket = Socket::Create(this, nsp);
            
            nsps_[nsp] = socket;
            
            socket->connecting_emitter()->On(on_connecting);

            socket->connect_emitter()->On([thiz, socket](None _){
                socket->set_id(thiz->engine_->id());
            });
            
            if (auto_connect_) {
                // manually call here since connecting evnet is fired before listening
                (*on_connecting)(None());
            }
        }
        

        return socket;
    }
    
    void Manager::Destroy(const std::shared_ptr<Socket> & socket) {
        auto index = std::find(connecting_.begin(), connecting_.end(), socket);
        if (index != connecting_.end()) {
            connecting_.erase(index);
        }
        if (connecting_.size() > 0) {
            return;
        }
        
        Close();
    }
    
    void Manager::WritePacket(const Packet & packet) {
        printf("[%s]\n", __PRETTY_FUNCTION__);

        if (!encoding_) {
            // encode, then write to engine with result
            encoding_ = true;
            auto encoded_packets = encoder_->Encode(packet);
 
            for (int i = 0; i < encoded_packets.size(); i++) {
                engine_->Send(encoded_packets[i]);
            }
            
            encoding_ = false;
            ProcessPacketQueue();
        } else { // add packet to the queue
            packet_buffer_.push_back(packet);
        }
    }
    
    void Manager::ProcessPacketQueue() {
        if (packet_buffer_.size() > 0 && !encoding_) {
            auto pack = packet_buffer_.front();
            packet_buffer_.erase(packet_buffer_.begin());
            WritePacket(pack);
        }
    }

    void Manager::Cleanup() {
        printf("[%s]\n", __PRETTY_FUNCTION__);

        for (auto sub : subs_) {
            sub.Destroy();
        }
        subs_.clear();
        
        packet_buffer_.clear();
        encoding_ = false;
        last_ping_ = Optional<std::chrono::system_clock::time_point>();

        decoder_->Destroy();
    }
    
    void Manager::Close() {
        printf("[%s]\n", __PRETTY_FUNCTION__);

        skip_reconnect_ = true;
        reconnecting_ = false;
        
        if (ready_state_ == ReadyState::Opening) {
            // `onclose` will not fire because
            // an open event never happened
            Cleanup();
        }
        
        ready_state_ = ReadyState::Closed;
        
        if (engine_) {
            engine_->Close();
        }
    }
    
    void Manager::OnClose() {
        printf("[%s]\n", __PRETTY_FUNCTION__);

        Cleanup();
//        this.backoff.reset();
        ready_state_ = ReadyState::Closed;
        
        close_emitter_->Emit(None());
        
        if (reconnection_ && !skip_reconnect_) {
            Reconnect();
        }
    }

    
    void Manager::Reconnect() {
        Fatal("not implemented");
    }

}
}