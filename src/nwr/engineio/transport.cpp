//
//  transport.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/20.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "transport.h"

#include "websocket_transport.h"

namespace nwr {
namespace eio {
    
    Transport::ConstructorParams::ConstructorParams():
    path(),
    hostname(),
    port(-1),
    secure(false),
    query(),
    timestamp_param(),
    timestamp_requests(),
    origin(),
    agent()
    {}
    
    Transport::Transport(const ConstructorParams & params):
    error_emitter_(std::make_shared<Emitter<Error>>()),
    open_emitter_(std::make_shared<Emitter<None>>()),
    packet_emitter_(std::make_shared<Emitter<Packet>>()),
    close_emitter_(std::make_shared<Emitter<None>>()),
    flush_emitter_(std::make_shared<Emitter<None>>()),
    drain_emitter_(std::make_shared<Emitter<None>>())
    {
        path_ = params.path;
        hostname_ = params.hostname;
        port_ = params.port;
        secure_ = params.secure;
        query_ = params.query;
        timestamp_param_ = params.timestamp_param;
        timestamp_requests_ = params.timestamp_requests;
        ready_state_ = ReadyState::None;
        origin_ = params.origin;
        agent_ = params.agent;
        
        writable_ = false;
    }
    
    std::shared_ptr<Transport> Transport::Create(const std::string & name,
                                                 const ConstructorParams & params)
    {
        if (name == "websocket") {
            return std::make_shared<WebsocketTransport>(params);
        } else {
            Fatal(Format("invalid transport name: %s", name.c_str()));
        }
    }
    
    Transport::~Transport() {
        if (ready_state_ != ReadyState::Closed || ready_state_ != ReadyState::None) {
            Fatal("not closed");
        }
    }
    
    void Transport::OnError(const std::string &msg) {
        auto err = Error("TransportError", msg);
        error_emitter_->Emit(err);
    }
    
    void Transport::Open() {
        if (ready_state_ == ReadyState::Closed || ready_state_ == ReadyState::None) {
            ready_state_ = ReadyState::Opening;
            DoOpen();
        }
    }
    
    void Transport::RemoveAllListeners() {
        error_emitter_->RemoveAllListeners();
        open_emitter_->RemoveAllListeners();
        packet_emitter_->RemoveAllListeners();
        close_emitter_->RemoveAllListeners();
        flush_emitter_->RemoveAllListeners();
        drain_emitter_->RemoveAllListeners();
    }
    
    void Transport::Close() {
        if (ready_state_ == ReadyState::Opening || ready_state_ == ReadyState::Open) {
            DoClose();
            OnClose();
        }
    }
    
    void Transport::Send(const std::vector<Packet> &packets) {
        if (ready_state_ == ReadyState::Open) {
            Write(packets);
        } else {
            Fatal("Transport not open");
        }
    }
    
    void Transport::OnOpen() {
        ready_state_ = ReadyState::Open;
        writable_ = true;
        open_emitter_->Emit(None());
    }
    
    void Transport::OnData(const Websocket::Message &data) {
        auto packet = DecodePacket(data);
        OnPacket(packet);
    }
    
    void Transport::OnPacket(const Packet &packet) {
        packet_emitter_->Emit(packet);
    }
    
    void Transport::OnClose() {
        ready_state_ = ReadyState::Closed;
        close_emitter_->Emit(None());
    }
    
}
}