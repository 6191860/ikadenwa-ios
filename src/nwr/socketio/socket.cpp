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

namespace nwr {
namespace sio {
    std::shared_ptr<Socket> Socket::Create(Manager * io, const std::string & nsp) {
        auto thiz = std::shared_ptr<Socket>(new Socket());
        thiz->Init(io, nsp);
        return thiz;
    }
    Socket::Socket():
    connect_emitter_(std::make_shared<decltype(connect_emitter_)::element_type>()),
    connect_error_emitter_(std::make_shared<decltype(connect_error_emitter_)::element_type>()),
    connect_timeout_emitter_(std::make_shared<decltype(connect_timeout_emitter_)::element_type>()),
    connecting_emitter_(std::make_shared<decltype(connecting_emitter_)::element_type>()),
    disconnect_emitter_(std::make_shared<decltype(disconnect_emitter_)::element_type>()),
    error_emitter_(std::make_shared<decltype(error_emitter_)::element_type>()),
    reconnect_emitter_(std::make_shared<decltype(reconnect_emitter_)::element_type>()),
    reconnect_attempt_emitter_(std::make_shared<decltype(reconnect_attempt_emitter_)::element_type>()),
    reconnect_failed_emitter_(std::make_shared<decltype(reconnect_failed_emitter_)::element_type>()),
    reconnect_error_emitter_(std::make_shared<decltype(reconnect_error_emitter_)::element_type>()),
    reconnecting_emitter_(std::make_shared<decltype(reconnecting_emitter_)::element_type>()),
    ping_emitter_(std::make_shared<decltype(ping_emitter_)::element_type>()),
    pong_emitter_(std::make_shared<decltype(pong_emitter_)::element_type>()),
    packet_emitter_(std::make_shared<decltype(packet_emitter_)::element_type>())
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
        io_->Open(std::function<void(const Optional<Error> &)>()); // ensure open
        if (io_->ready_state() == Manager::ReadyState::Open) {
            OnOpen();
        }
        connecting_emitter()->Emit(None());
    }
    
    void Socket::Emit(const std::string & ev, const Packet & packet) {
        
        auto parser_type = PacketType::Event; // default
//        if (hasBin(args)) { parserType = parser.BINARY_EVENT; } // binary
//        var packet = { type: parserType, data: args };
//        
//        packet.options = {};
//        packet.options.compress = !this.flags || false !== this.flags.compress;
//        
//        // event ack callback
//        if ('function' == typeof args[args.length - 1]) {
//            debug('emitting packet with ack id %d', this.ids);
//            this.acks[this.ids] = args.pop();
//            packet.id = this.ids++;
//        }
//        
//        if (this.connected) {
//            this.packet(packet);
//        } else {
//            this.sendBuffer.push(packet);
//        }
//        
//        delete this.flags;
//        
//        return this;
    }
    
    void Socket::OnOpen() {
        
    }
    
    void Socket::OnPacket(const Packet & packet) {
        
    }
    
    void Socket::OnClose() {
        
    }
    
}
}
