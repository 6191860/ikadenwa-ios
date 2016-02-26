//
//  socket.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/25.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "socket.h"

#include "io.h"
#include "namespace.h"
#include "transport.h"
#include "websocket_transport.h"
#include "util.h"

namespace nwr {
namespace sio0 {
    
    SocketOptions::SocketOptions()
    {}
    
    std::shared_ptr<CoreSocket> CoreSocket::Create(const SocketOptions & options) {
        auto thiz = std::shared_ptr<CoreSocket>(new CoreSocket());
        thiz->Init(options);
        return thiz;
    }
    void CoreSocket::Init(const SocketOptions & options) {
        
        emitter_ = std::make_shared<AnyEmitter>();
        
        options_ = options;
        auto & o = options_;
        
        if (!o.port) { o.port = Some(80); }
        if (!o.secure) { o.secure = Some(false); }
        if (!o.resource) { o.resource = Some(std::string("socket.io")); }
        if (!o.connect_timeout) { o.connect_timeout = Some(TimeDuration(10.0)); }
        
        if (!o.reconnect) { o.reconnect = Some(false); } // original: true
        if (!o.reconnection_delay) { o.reconnection_delay = Some(TimeDuration(0.5)); }
        if (!o.reconnection_limit) { o.reconnection_limit = Some(TimeDuration(std::numeric_limits<double>::infinity())); }
        if (!o.reopen_delay) { o.reopen_delay = Some(TimeDuration(3.0)); }
        if (!o.max_reconnection_attempts) { o.max_reconnection_attempts = Some(10); }
        if (!o.auto_connect) { o.auto_connect = Some(true); }
        if (!o.manual_flush) { o.manual_flush = Some(false); }
    
        connected_ = false;
        open_ = false;
        connecting_ = false;
        reconnecting_ = false;
        namespaces_.clear();
        buffer_.clear();
        do_buffer_ = false;
        
        if (*options_.auto_connect) {
            Connect(nullptr);
        }
    }
    
    CoreSocket::~CoreSocket() {
        
    }
    
    AnyEmitterPtr CoreSocket::emitter() const {
        return emitter_;
    }
    bool CoreSocket::connected() const {
        return connected_;
    }
    bool CoreSocket::connecting() const {
        return connecting_;
    }
    
    bool CoreSocket::reconnecting() const {
        return reconnecting_;
    }
    
    TimeDuration CoreSocket::close_timeout() const {
        return close_timeout_;
    }
    
    const SocketOptions & CoreSocket::options() const {
        return options_;
    }
    
    CoreSocket::CoreSocket()
    {}
    
    std::shared_ptr<Socket> CoreSocket::Of(const std::string & name) {
        if (!HasKey(namespaces_, name))  {
            namespaces_[name] = std::shared_ptr<Socket>(new Socket(shared_from_this(), name));

            if (name != "") {
                Packet pkt;
                pkt.type = PacketType::Connect;
                namespaces_[name]->SendPacket(pkt);
            }
        }
        
        return namespaces_[name];
    }
    
    void CoreSocket::Publish(const std::string & event, const std::vector<Any> & args) {
        emitter_->Emit(event, args);

        for (const auto & i : Keys(namespaces_)) {
            const auto & nsp = Of(i);
            nsp->emitter()->Emit(event, args);
        }
    }
    
    void CoreSocket::Handshake(const std::function<void(const std::vector<std::string> &)> & fn) {
        auto thiz = shared_from_this();
        
        auto complete_error = [thiz](const std::string & error){
            thiz->connecting_ = false;
            thiz->OnError(error);
        };
        auto complete_success = [thiz, fn](const std::string & data) {
            FuncCall(fn, Split(data, ":"));
        };
        
        auto url_parts = std::vector<std::string> {
            std::string("http") + (*options_.secure ? std::string("s") : std::string()) + std::string(":/"),
            Format("%s:%d", (*options_.host).c_str(), (*options_.port)),
            *options_.resource,
            Format("%d", Io::protocol_),
            MergeQuery(*options_.query, Format("t=%lld", GetCurrentUnixTime().count()))
        };
        auto url = Join(url_parts, "/");
        
        HttpRequest request(url, "GET", {});
        auto op = HttpOperation::Create(request);
        op->set_on_failure([complete_error](const std::string & error){
            complete_error(error);
        });
        op->set_on_success([thiz, complete_success, complete_error](const HttpResponse & response) {
            if (response.code == 200) {
                complete_success(ToString(*response.data));
            } else {
                thiz->connecting_ = false;
                thiz->OnError(ToString(*response.data));
            }
        });
    }
    
    std::shared_ptr<Transport> CoreSocket::GetTransport() {
        auto transport_rawptr = new WebsocketTransport(shared_from_this(),
                                                       session_id_);
        auto transport = std::shared_ptr<WebsocketTransport>(transport_rawptr);
        return transport;
    }
    
    void CoreSocket::Connect(const std::function<void()> & fn) {
        auto thiz = shared_from_this();
        
        if (connecting_) {
            return;
        }
        
        connecting_ = true;

        Handshake([thiz, fn](const std::vector<std::string> & strs) {
            std::string sid = strs[0];
            std::string heartbeat = strs[1];
            std::string close = strs[2];
            std::string transports = strs[3];
            
            thiz->session_id_ = sid;
            thiz->close_timeout_ = TimeDuration(atoi(close.c_str()));
            thiz->heartbeat_timeout_ = TimeDuration(atoi(heartbeat.c_str()));
            
            thiz->SetHeartbeatTimeout();
            
            auto connect = [thiz](){
                if (thiz->transport_) {
                    thiz->transport_->ClearTimeouts();
                }
                thiz->transport_ = thiz->GetTransport();
                if (!thiz->transport_) {
                    thiz->Publish("connect_failed", {});
                    return;
                }
                
                thiz->connecting_ = true;
                thiz->Publish("connecting", { Any(thiz->transport_->name()) });
                thiz->transport_->Open();

                if (thiz->options_.connect_timeout) {
                    
                    thiz->connect_timeout_timer_ =
                    Timer::Create(*thiz->options_.connect_timeout,
                                  [thiz](){
                                      if (!thiz->connected_) {
                                          thiz->connecting_ = false;
                                      }
                                      printf("connect timeout\n");
                                  });
                }

            };
            
            connect();

            thiz->emitter_->Once("connect", AnyEventListenerMake([thiz, fn](){
                if (thiz->connect_timeout_timer_) {
                    thiz->connect_timeout_timer_->Cancel();
                    thiz->connect_timeout_timer_ = nullptr;
                }
                
                FuncCall(fn);
            }));
        });
    }
    
    void CoreSocket::SetHeartbeatTimeout() {
        auto thiz = shared_from_this();
        
        if (heartbeat_timeout_timer_) {
            heartbeat_timeout_timer_->Cancel();
            heartbeat_timeout_timer_ = nullptr;
        }

        if (transport_ && !transport_->heartbeats()) { return; }
        
        heartbeat_timeout_timer_ =
        Timer::Create(heartbeat_timeout_,
                      [thiz](){
                          thiz->transport_->OnClose();
                      });
    }
    
    void CoreSocket::SendPacket(const Packet & data) {
        if (connected_ && !do_buffer_) {
            transport_->SendPacket(data);
        } else {
            buffer_.push_back(data);
        }
    }
    
    void CoreSocket::SetBuffer(bool v) {
        do_buffer_ = v;
        
        if (!v && connected_ && buffer_.size() != 0) {
            if (!(*options_.manual_flush)) {
                FlushBuffer();
            }
        }
    }
    
    void CoreSocket::FlushBuffer() {
        transport_->SendPayload(buffer_);
        buffer_.clear();
    }
    
    void CoreSocket::Disconnect() {
        if (connected_ || connecting_) {
            if (open_) {
                Packet packet;
                packet.type = PacketType::Disconnect;
                Of("")->SendPacket(packet);
            }
            
            // handle disconnection immediately
            OnDisconnect("booted");
        }
        
        namespaces_.clear();
    }
    
    void CoreSocket::OnConnect() {
        if (!connected_) {
            connected_ = true;
            connecting_ = false;
            if (!do_buffer_) {
                // make sure to flush the buffer
                SetBuffer(false);
            }
            emitter_->Emit("connect", {});
        }
    }
    
    void CoreSocket::OnOpen() {
        open_ = true;
    }
    
    void CoreSocket::OnClose() {
        open_ = false;
        
        if (heartbeat_timeout_timer_) {
            heartbeat_timeout_timer_->Cancel();
            heartbeat_timeout_timer_ = nullptr;
        }
    }

    void CoreSocket::OnPacket(const Packet & packet) {
        Of(packet.endpoint)->OnPacket(packet);
    }
    
    void CoreSocket::OnError(const std::string & error) {
        OnError(error, "");
    }
    
    void CoreSocket::OnError(const std::string & error, const std::string & advice) {
        if (advice == "reconnect" && (connected_ || connecting_)) {
            Disconnect();
            if (*options_.reconnect) {
                #warning todo reconnect
                // Reconnect();
            }
        }
        
        Publish("error", { Any(error) });
    }
    
    void CoreSocket::OnDisconnect(const std::string & reason) {
        bool was_connected = connected_;
        bool was_connecting = connecting_;

        connected_ = false;
        connecting_ = false;
        open_ = false;

        if (was_connected || was_connecting) {
            transport_->Close();
            transport_->ClearTimeouts();
            transport_ = nullptr;
            if (was_connected) {
                Publish("disconnect", { Any(reason) });
#warning todo reconnect
//                if ('booted' != reason && this.options.reconnect && !this.reconnecting) {
//                    this.reconnect();
//                }
            }
        }
    
    }

    
}
}