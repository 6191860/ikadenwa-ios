//
//  transport.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/25.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "transport.h"
#include "parser.h"
#include "socket.h"
#include "io.h"

namespace nwr {
namespace sio0 {
    Transport::Transport(const std::shared_ptr<CoreSocket> & socket, const std::string & sessid):
    socket_(socket),
    sessid_(sessid),
    is_open_(false)
    {}

    bool Transport::heartbeats() {
        return true;
    }
    
    std::shared_ptr<CoreSocket> Transport::socket() {
        return socket_;
    }
    
    void Transport::OnData(const std::string & data) {
        ClearCloseTimeout();
        
        // If the connection in currently open (or in a reopening state) reset the close
        // timeout since we have just received data. This check is necessary so
        // that we don't reset the timeout on an explicitly disconnected connection.
        if (socket_->connected() || socket_->connecting() || socket_->reconnecting()) {
            SetCloseTimeout();
        }

        if (data != "") {
            // todo: we should only do decodePayload for xhr transports
            
            Packet msg = DecodePacket(data);
            OnPacket(msg);
        }
    }
    
    void Transport::OnPacket(const Packet & packet) {
        socket_->SetHeartbeatTimeout();
        
        if (packet.type == PacketType::Heartbeat) {
            OnHeartbeat();
        }
        
        if (packet.type == PacketType::Connect && packet.endpoint == "") {
            OnConnect();
        }
        
        if (packet.type == PacketType::Error && packet.advice == Some(std::string("reconnect")) ) {
            is_open_ = false;
        }
  
        socket_->OnPacket(packet);
    }
    
    void Transport::SetCloseTimeout() {
        if (!close_timeout_) {
            auto thiz = shared_from_this();

            close_timeout_ = Timer::Create(socket_->close_timeout(), [thiz](){
                thiz->OnDisconnect();
            });
        }
    }
    
    void Transport::OnDisconnect() {
        if (is_open_) {
            Close();
        }
        ClearTimeouts();
        socket_->OnDisconnect("");
    }
    
    void Transport::OnConnect() {
        socket_->OnConnect();
    }
    
    void Transport::ClearCloseTimeout() {
        if (close_timeout_) {
            close_timeout_->Cancel();
            close_timeout_ = nullptr;
        }
    }
    
    void Transport::ClearTimeouts() {
        ClearCloseTimeout();

        if (reopen_timeout_) {
            reopen_timeout_->Cancel();
            reopen_timeout_ = nullptr;
        }
    }
    
    void Transport::SendPacket(const Packet & packet) {
        Send(EncodePacket(packet));
    }
    
    void Transport::OnHeartbeat() {
        Packet packet;
        packet.type = PacketType::Heartbeat;
        SendPacket(packet);
    }
    
    void Transport::OnOpen() {
        is_open_ = true;
        ClearCloseTimeout();
        socket_->OnOpen();
    }
    
    void Transport::OnClose() {
        auto thiz = shared_from_this();
        
        /* FIXME: reopen delay causing a infinit loop
         this.reopenTimeout = setTimeout(function () {
         self.open();
         }, this.socket.options['reopen delay']);*/
        
        is_open_ = false;
        socket_->OnClose();
        OnDisconnect();
    }
    
    std::string Transport::PrepareUrl() {
        auto & options = socket()->options();
        
        std::string url =
        scheme() + std::string("://") +
        Format("%s:%d/",
               options.host.value().c_str(),
               options.port.value()) +
        Format("%s/%d/%s/%s",
               options.resource.value().c_str(),
               Io::protocol_,
               name().c_str(),
               sessid_.c_str());
        
        return url;
    }
}
}
