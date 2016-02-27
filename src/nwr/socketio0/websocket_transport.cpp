//
//  websocket_transport.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/25.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "websocket_transport.h"

#include "parser.h"
#include "socket.h"
#include "util.h"

namespace nwr {
namespace sio0 {
    WebsocketTransport::WebsocketTransport(const std::shared_ptr<CoreSocket> & socket,
                                           const std::string & sessid):
    Transport(socket, sessid)
    {}
    
    WebsocketTransport::~WebsocketTransport() {
        
    }
    
    std::string WebsocketTransport::name() {
        return "websocket";
    }
    
    void WebsocketTransport::Open() {
        auto thiz = std::static_pointer_cast<WebsocketTransport>(shared_from_this());
        
        auto query = MergeQuery(socket_->options().query.value(), "");
        
        websocket_ = Websocket::Create(PrepareUrl() + query, "");
        websocket_->set_on_open([thiz](){
            thiz->OnOpen();
            thiz->socket()->SetBuffer(false);
        });
        websocket_->set_on_message([thiz](const Websocket::Message & message){
            thiz->OnData(ToString(*message.data));
        });
        websocket_->set_on_close([thiz](){
            thiz->OnClose();
            thiz->socket()->SetBuffer(true);
        });
        websocket_->set_on_error([thiz](const std::string & error){
            thiz->OnError(error);
        });

    }
    
    void WebsocketTransport::Send(const std::string & data) {
        websocket_->Send(Websocket::Message(data));
    }
    
    void WebsocketTransport::SendPayload(const std::vector<Packet> & payload) {
        for (const auto & packet : payload) {
            SendPacket(packet);
        }
    }
    
    void WebsocketTransport::Close() {
        websocket_->Close();
        websocket_ = nullptr;
    }
    
    void WebsocketTransport::OnError(const std::string & error) {
        socket()->OnError(error);
    }
    
    std::string WebsocketTransport::scheme() {
        return socket()->options().secure.value() ? std::string("wss") : std::string("ws");
    }
}
}
