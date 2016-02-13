//
//  socket.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/24.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "socket.h"

#include "on.h"
#include "packet.h"
#include "manager.h"
#include "parser.h"
#include "binary.h"

namespace nwr {
namespace sio {
    std::vector<std::string> Socket::events_ = {
        "connect",
        "connect_error",
        "connect_timeout",
        "connecting",
        "disconnect",
        "error",
        "reconnect",
        "reconnect_attempt",
        "reconnect_failed",
        "reconnect_error",
        "reconnecting",
        "ping",
        "pong"
    };
    
    std::shared_ptr<Socket> Socket::Create(Manager * io, const std::string & nsp) {
        auto thiz = std::shared_ptr<Socket>(new Socket());
        thiz->Init(io, nsp);
        return thiz;
    }
    Socket::Socket():
    emitter_(std::make_shared<decltype(emitter_)::element_type>())
    {
    }
    void Socket::Init(Manager * io, const std::string & nsp) {
        io_ = io;
        nsp_ = nsp;
        
        ids_ = 0;
        connected_ = false;
        disconnected_ = true;
        if (io->auto_connect()) {
            Open();
        }
    }
    
    void Socket::SubEvents() {
        if (subs_.size() > 0) { return; }
        
        auto thiz = shared_from_this();
        subs_ = std::vector<OnToken> {
            On<None>(io_->open_emitter(),
                     [thiz](None _) { thiz->OnOpen(); }),
            On<Packet>(io_->packet_emitter(),
                       [thiz](const Packet & packet) { thiz->OnPacket(packet); }),
            On<None>(io_->close_emitter(),
                     [thiz](None _) { thiz->OnClose(); })
        };
    }
    
    void Socket::Open() {
        if (connected_) { return; }

        SubEvents();
        io_->Open(nullptr); // ensure open
        if (io_->ready_state() == Manager::ReadyState::Open) {
            OnOpen();
        }
        
        emitter_->Emit("connecting", {});
    }
    
    void Socket::Send(const std::vector<Any> & args, const AckFunc & ack_callback) {
        Emit("message", args, ack_callback);
    }
    
    void Socket::Emit(const std::string & event, const std::vector<Any> & args) {
        Emit(event, args, nullptr);
    }
    
    void Socket::Emit(const std::string & event, const std::vector<Any> & arg_args, const AckFunc & ack_callback) {
        if (IndexOf(events_, event) != -1) {
            emitter_->Emit(event, arg_args);
            return;
        }
        
        std::vector<Any> args = arg_args;
        
        auto parser_type = PacketType::Event; // default
        for (auto & arg : args) {
            if (HasBinary(arg)) {
                parser_type = PacketType::BinaryEvent;  // binary
            }
        }
        args.insert(args.begin(), Any(event));
        
        Packet packet;
        packet.type = parser_type;
        packet.data = Any(args);
        
        // event ack callback
        if (ack_callback) {
            printf("[%s] emitting packet with ack id %d\n", __PRETTY_FUNCTION__, ids_);
            acks_[ids_] = ack_callback;
            packet.id = Some(ids_);
            ids_ += 1;
        }
        
        if (connected_) {
            SendPacket(packet);
        } else {
            send_buffer_.push_back(packet);
        }
    }
    
    void Socket::SendPacket(Packet packet) {
        packet.nsp = Some(nsp_);
        io_->WritePacket(packet);
    }
    
    void Socket::OnOpen() {
        printf("[%s] transport is open - connecting\n", __PRETTY_FUNCTION__);
        
        // write connect packet if necessary
        if (nsp_ != "/") {
            Packet packet;
            packet.type = PacketType::Connect;
            SendPacket(packet);
        }
    }
    
    void Socket::OnClose() {
        printf("[%s] close\n", __PRETTY_FUNCTION__);
        
        connected_ = false;
        disconnected_ = true;
        
        id_ = "";
        
        emitter_->Emit("disconnect", {});
    }
    
    void Socket::OnPacket(const Packet & packet) {
        if (packet.nsp != Some(nsp_)) { return; }
        
        printf("[%s] type=%d\n", __PRETTY_FUNCTION__, packet.type);
        
        switch (packet.type) {
            case PacketType::Connect:
                OnConnect();
                break;

            case PacketType::Event:
                OnEvent(packet);
                break;
                
            case PacketType::BinaryEvent:
                OnEvent(packet);
                break;
                
            case PacketType::Ack:
                OnAck(packet);
                break;
                
            case PacketType::BinaryAck:
                OnAck(packet);
                break;
                
            case PacketType::Disconnect:
                OnDisconnect();
                break;
                
            case PacketType::Error: {
                emitter_->Emit("error", { packet.data });
                break;
            }
        }
    }
    
    void Socket::OnEvent(const Packet & packet) {
        std::vector<Any> args = packet.data.AsArray() || std::vector<Any>();
        printf("[%s] emitting event %s\n", __PRETTY_FUNCTION__,
               packet.data.ToJsonString().c_str());
        
        if (args.size() <= 1) {
            printf("no event name\n");
            return;
        }
        
        auto event = args[0].AsString();
        if (!event) {
            printf("event name not string\n");
            return;
        }
        args.erase(args.begin());
        
        AckFunc ack;
        if (packet.id) {
            printf("[%s] attaching ack callback to event\n", __PRETTY_FUNCTION__);
            ack = Ack(packet.id.value());
            Fatal("need to investigate this flow");
        }
        
        if (connected_) {
            emitter_->Emit(event.value(), args);
        } else {
            receive_buffer_.push_back(EmitParams { event.value(), args });
        }
    }
    
    Socket::AckFunc Socket::Ack(int id) {
        auto thiz = shared_from_this();
        
        auto sent_ptr = std::make_shared<bool>(false);
        
        return [thiz, sent_ptr, id](const Any & args) {
            // prevent double callbacks
            if (*sent_ptr) { return; }
            *sent_ptr = true;
            
            printf("[%s] sending ack\n", __PRETTY_FUNCTION__);
            
            auto type = HasBinary(args) ? PacketType::BinaryAck : PacketType::Ack;
            Packet packet;
            packet.type = type;
            packet.id = Some(id);
            packet.data = args;

            thiz->SendPacket(packet);
        };
    }
    
    void Socket::OnAck(const Packet & packet) {
        if (packet.id) {
            auto packet_id = packet.id.value();
            AckFunc ack = acks_[packet_id];
            printf("[%s] calling ack %d\n", __PRETTY_FUNCTION__, packet_id);
            if (ack) {
                ack(packet.data);
            }
            acks_.erase(packet_id);
        }
    }
    
    void Socket::OnConnect() {
        connected_ = true;
        disconnected_ = false;
        emitter_->Emit("connect", {});
        EmitBuffered();
    }
    
    void Socket::EmitBuffered() {
        int i;
        for (i = 0; i < receive_buffer_.size(); i++) {
            auto params = receive_buffer_[i];
            emitter_->Emit(params.event, params.args);
        }
        receive_buffer_.clear();
        
        for (i = 0; i < send_buffer_.size(); i++) {
            SendPacket(send_buffer_[i]);
        }
        send_buffer_.clear();
    }
    
    void Socket::OnDisconnect() {
        printf("[%s] server disconnect (%s)\n", __PRETTY_FUNCTION__, nsp_.c_str());
        Destroy();
        OnClose();
    }
    
    void Socket::Destroy() {
        // clean subscriptions to avoid reconnections
        for (int i = 0; i < subs_.size(); i++) {
            subs_[i].Destroy();
        }
        subs_.clear();
        
        io_->Destroy(shared_from_this());
    }
    
    void Socket::Close() {
        if (connected_) {
            printf("[%s] performing disconnect (%s)\n", __PRETTY_FUNCTION__, nsp_.c_str());
            Packet packet;
            packet.type = PacketType::Disconnect;
            SendPacket(packet);
        }
        
        // remove socket from pool
        Destroy();
        
        if (connected_) {
            // fire events
            OnClose();
        }
    }
}
}
