//
//  socket.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/21.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "socket.h"

#include "url.h"
#include "data.h"
#include "timer.h"
#include "transport.h"

namespace nwr {
namespace eio {
        
    Socket::ConstructorParams::ConstructorParams():
    agent(),
    timestamp_param("t"),
    timestamp_requests(false),
    
    reconnection(true),
    reconnection_attempts(-1),
    reconnection_delay(TimeDuration(1.0)),
    reconnection_delay_max(TimeDuration(5.0)),
    randomization_factor(0.5),
    timeout(TimeDuration(20.0)),
    auto_connect(true),
    
    force_new(false),
    multiplex(true)
    {}
    
    std::shared_ptr<Socket> Socket::Create(const std::string & uri, const ConstructorParams & params) {
        auto thiz = std::shared_ptr<Socket>(new Socket());
        thiz->Init(uri, params);
        return thiz;
    }
    
    Socket::Socket():
    transport_(nullptr),
    open_emitter_(std::make_shared<decltype(open_emitter_)::element_type>()),
    packet_emitter_(std::make_shared<decltype(packet_emitter_)::element_type>()),
    heartbeat_emitter_(std::make_shared<decltype(heartbeat_emitter_)::element_type>()),
    pong_emitter_(std::make_shared<decltype(pong_emitter_)::element_type>()),
    message_emitter_(std::make_shared<decltype(message_emitter_)::element_type>()),
    handshake_emitter_(std::make_shared<decltype(handshake_emitter_)::element_type>()),
    ping_emitter_(std::make_shared<decltype(ping_emitter_)::element_type>()),
    drain_emitter_(std::make_shared<decltype(drain_emitter_)::element_type>()),
    flush_emitter_(std::make_shared<decltype(flush_emitter_)::element_type>()),
    packet_create_emitter_(std::make_shared<decltype(packet_create_emitter_)::element_type>()),
    error_emitter_(std::make_shared<decltype(error_emitter_)::element_type>()),
    close_emitter_(std::make_shared<decltype(close_emitter_)::element_type>())
    {
    }
    
    void Socket::Init(const std::string &uri, const ConstructorParams &params) {
        on_heartbeat_ptr_ = EventListenerMake<Optional<TimeDuration>>(
            std::bind(&Socket::OnHeartbeat, this, std::placeholders::_1));
        
        auto new_params = params;
        auto url_parts = ParseUrl(uri);
        
        hostname_ = url_parts.hostname;
        secure_ = url_parts.scheme == "https" || url_parts.scheme == "wss";
        if (url_parts.port) {
            port_ = *url_parts.port;
        } else {
            // if no port is specified manually, use the protocol default
            port_ = secure_ ? 443 : 80;
        }
        query_ = url_parts.query;
        
        agent_ = params.agent;
        path_ = PathAppendSlash(params.path || std::string("/engine.io"));
        
        timestamp_param_ = params.timestamp_param;
        timestamp_requests_ = params.timestamp_requests;
        
        ready_state_ = ReadyState::None;
        
        Open();
    }
    
    Socket::~Socket() {
        if (ping_timeout_timer_) { ping_timeout_timer_->Cancel(); }
        if (ping_interval_timer_) { ping_interval_timer_->Cancel(); }
        if (transport_) { transport_->Close(); }
    }
    
    int Socket::protocol() {
        return parser_protocol();
    }
    
    std::shared_ptr<Transport> Socket::CreateTransport(const std::string & name) {
        auto query = query_;
        
        // append engine.io protocol identifier
        query["EIO"] = Format("%d", parser_protocol());
        
        // transport name
        query["transport"] = name;

        // session id if we already have one
        if (id_ != "") {
            query["sid"] = id_;
        }
        
        Transport::ConstructorParams p;
        
        p.path = path_;
        p.hostname = hostname_;
        p.port = port_;
        p.secure = secure_;
        p.query = query;
        p.timestamp_param = timestamp_param_;
        p.timestamp_requests = timestamp_requests_;
        p.origin = origin_;
        p.agent = agent_;
        
        return Transport::Create(name, p);
    }
    
    void Socket::Open() {
        printf("%s\n", __PRETTY_FUNCTION__);
        std::string transport_name = "websocket";
        ready_state_ = ReadyState::Opening;
        
        auto transport = CreateTransport(transport_name);
        
        transport->Open();

        set_transport(transport);
    }
    
    void Socket::set_transport(const std::shared_ptr<Transport> & transport) {
        if (transport_) {
            printf("clearing existing transport %s\n", transport_->name().c_str());
            transport_->RemoveAllListeners();
        }
        
        // set up transport
        transport_ = transport;
        
        auto thiz = shared_from_this();
        
        // set up transport listeners
        transport_->drain_emitter()->On([thiz](None _){
            thiz->OnDrain();
        });
        transport_->packet_emitter()->On([thiz](const Packet & packet){
            thiz->OnPacket(packet);
        });
        transport_->error_emitter()->On([thiz](const Error & error){
            thiz->OnError(error);
        });
        transport_->close_emitter()->On([thiz](None _){
            thiz->OnClose();
        });
    }
    
    void Socket::OnOpen() {
        printf("%s\n", __PRETTY_FUNCTION__);
        ready_state_ = ReadyState::Open;

        open_emitter_->Emit(None());
        Flush();
    }
    
    void Socket::OnPacket(const Packet & packet) {
        printf("%s\n", __PRETTY_FUNCTION__);
        if (ready_state_ == ReadyState::Opening || ready_state_ == ReadyState::Open) {
            printf("socket receive: type=%s, data=%.*s\n",
                   ToString(packet.type).c_str(),
                   packet.data.size(), packet.data.char_ptr());
            
            packet_emitter_->Emit(packet);
            
            // Socket is live - any packet counts
            heartbeat_emitter_->Emit(None());
            
            switch (packet.type) {
                case PacketType::Open: {
                    auto json = JsonParse(packet.data.ptr(), packet.data.size());
                    if (!json) { Fatal("json parse failed"); }
                    OnHandshake(*json);
                    break;
                }
                case PacketType::Pong:
                    SetPing();
                    pong_emitter_->Emit(None());
                    break;
                case PacketType::Error: {
                    auto err = Error("server error", std::string(packet.data.char_ptr(), packet.data.size()));
                    OnError(err);
                    break;
                }
                case PacketType::Message:
                    message_emitter_->Emit(packet.data);
                    break;
                default:
                    break;
            }
        } else {
            printf("packet received with socket readyState %d\n", ready_state_);
        }
    }
    
    void Socket::OnHandshake(const Json::Value & json) {
        printf("%s\n", __PRETTY_FUNCTION__);
        handshake_emitter_->Emit(json);
        
        std::string str;
        if(!rtc::GetStringFromJsonObject(json, "sid", &str)) {
            Fatal(Format("invalid json: %s", JsonFormat(json).c_str()));
        }
        id_ = str;
        transport_->query_ref()["sid"] = str;
        
        double dbl;
        if (!rtc::GetDoubleFromJsonObject(json, "pingInterval", &dbl)) {
            Fatal(Format("invalid json: %s", JsonFormat(json).c_str()));
        }
        ping_interval_ = TimeDuration(dbl / 1000.0);
        
        if (!rtc::GetDoubleFromJsonObject(json, "pingTimeout", &dbl)) {
            Fatal(Format("invalid json: %s", JsonFormat(json).c_str()));
        }
        ping_timeout_ = TimeDuration(dbl / 1000.0);
 
        OnOpen();

        // In case open handler closes socket
        if (ready_state_ == ReadyState::Closed) { return; }

        SetPing();
        
        // Prolong liveness of socket on heartbeat
        heartbeat_emitter_->Off(on_heartbeat_ptr_);
        heartbeat_emitter_->On(on_heartbeat_ptr_);
    }
    
    void Socket::OnHeartbeat(const Optional<TimeDuration> & timeout) {
        printf("%s\n", __PRETTY_FUNCTION__);
        
        if (ping_timeout_timer_) {
            ping_timeout_timer_->Cancel();
        }

        auto thiz = shared_from_this();
        ping_timeout_timer_ = Timer::Create(timeout || (ping_interval_ + ping_timeout_),
                                            [thiz]{
                                                printf("ping timeout\n");
                                                if (thiz->ready_state_ == ReadyState::Closed) { return; }
                                                thiz->OnClose();
                                            });
    }
    
    void Socket::SetPing() {
        printf("%s\n", __PRETTY_FUNCTION__);
        if (ping_interval_timer_) {
            ping_interval_timer_->Cancel();
        }
        
        auto thiz = shared_from_this();
        ping_interval_timer_ = Timer::Create(ping_interval_,
                                             [thiz]{
                                                 printf("write ping\n");
                                                 thiz->Ping();
                                                 thiz->OnHeartbeat(Some(thiz->ping_timeout_));
                                             });
    }
    
    void Socket::Ping() {
        printf("%s\n", __PRETTY_FUNCTION__);
        auto thiz = shared_from_this();
        SendPacket(PacketType::Ping, PacketData(""), [thiz]{
            thiz->ping_emitter_->Emit(None());
        });
    }
    
    void Socket::OnDrain() {
        printf("%s\n", __PRETTY_FUNCTION__);
        write_buffer_.erase(write_buffer_.begin(), write_buffer_.begin() + prev_buffer_len_);
        
        // setting prevBufferLen = 0 is very important
        // for example, when upgrading, upgrade packet is sent over,
        // and a nonzero prevBufferLen could cause problems on `drain`
        prev_buffer_len_ = 0;

        if (write_buffer_.size() == 0) {
            drain_emitter_->Emit(None());
        } else {
            Flush();
        }
    }
    
    void Socket::Flush() {
        printf("%s\n", __PRETTY_FUNCTION__);
        if (ready_state_ != ReadyState::Closed &&
            transport_->writable() &&
            write_buffer_.size() > 0)
        {
            printf("flushing %d packets\n", (int)write_buffer_.size());
            
            transport_->Send(write_buffer_);

            // keep track of current length of writeBuffer
            // splice writeBuffer and callbackBuffer on `drain`
            prev_buffer_len_ = static_cast<int>(write_buffer_.size());
            flush_emitter_->Emit(None());
        }
    }

    
    void Socket::Send(const PacketData & data) {
        Send(data, nullptr);
    }
    void Socket::Send(const PacketData & data, std::function<void()> callback) {
        printf("%s\n", __PRETTY_FUNCTION__);
        SendPacket(PacketType::Message, data, callback);
    }
   
    void Socket::SendPacket(PacketType type, const PacketData & data, std::function<void()> callback) {
        printf("%s\n", __PRETTY_FUNCTION__);
        if (ready_state_ == ReadyState::Closing || ready_state_ == ReadyState::Closed) {
            return;
        }
        
        Packet packet = {
            type,
            data
        };
        
        packet_create_emitter_->Emit(packet);
        write_buffer_.push_back(packet);
        
        if (callback) {
            flush_emitter_->Once([callback](const None & _) {
                if (callback) { callback(); }
            });
        }
        
        Flush();
    }
    
    void Socket::Close() {
        printf("%s\n", __PRETTY_FUNCTION__);
        
        auto thiz = shared_from_this();
        auto close_func = std::function<void()>([thiz]{
            thiz->OnClose();
            printf("socket closing");
            thiz->transport_->Close();
        });
        
        if (ready_state_ == ReadyState::Opening || ready_state_ == ReadyState::Open) {
            ready_state_ = ReadyState::Closing;

            if (write_buffer_.size() > 0) {
                drain_emitter_->Once([close_func](const None & _){
                    close_func();
                });
            } else {
                close_func();
            }
        }
    }
    
    void Socket::OnError(const Error & error) {
        printf("socket error %s\n", error.Dump().c_str());
        error_emitter_->Emit(error);
        OnClose();
    }
    
    void Socket::OnClose() {
        if (ready_state_ == ReadyState::Opening ||
            ready_state_ == ReadyState::Open ||
            ready_state_ == ReadyState::Closing)
        {
            printf("socket close\n");
            
            // clear timers
            if (ping_interval_timer_) {
                ping_interval_timer_->Cancel();
                ping_interval_timer_ = nullptr;
            }
            if (ping_timeout_timer_) {
                ping_timeout_timer_->Cancel();
                ping_timeout_timer_ = nullptr;
            }
            
            // stop event from firing again for transport
            transport_->close_emitter()->RemoveAllListeners();
            
            // ensure transport won't stay open
            transport_->Close();
            
            // ignore further transport communication
            transport_->RemoveAllListeners();
            
            // set ready state
            ready_state_ = ReadyState::Closed;
            
            // clear session id
            id_ = "";
            
            // emit close event
            
            close_emitter_->Emit(None());
            
            // clean buffers after, so users can still
            // grab the buffers on `close` event
            write_buffer_.clear();
            prev_buffer_len_ = 0;
        }
    }
    
}
}