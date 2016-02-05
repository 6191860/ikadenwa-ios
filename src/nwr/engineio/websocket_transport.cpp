//
//  websocket_transport.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/21.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "websocket_transport.h"

#include "yeast.h"

namespace nwr {
namespace eio {
    
    WebsocketTransport::WebsocketTransport(const Transport::ConstructorParams & params):
    Transport(params){
        
    }
    
    void WebsocketTransport::DoOpen() {
        auto uri = this->uri();
        
        ws_ = Websocket::Create(uri, origin_);
        
        AddEventListeners();
    }
    
    void WebsocketTransport::AddEventListeners() {
        ws_->set_on_open([this]{
            OnOpen();
        });
        ws_->set_on_close([this]{
            OnClose();
        });
        ws_->set_on_message([this](const Websocket::Message & message){
            OnData(message);
        });
        ws_->set_on_error([this](const std::string & error){
            OnError(Format("websocket error: %s", error.c_str()));
        });
    }
    
    std::string WebsocketTransport::uri() {
        auto query = query_;
        std::string schema = secure_ ? "wss" : "ws";
    
        // avoid port if default for schema
        std::string port;
        if (port_ >= 0 && (("wss" == schema && port_ != 443) ||
                           ("ws" == schema && port_ != 80)))
        {
            port = Format(":%d", port_);
        }
        
        // append timestamp to URI
        if (timestamp_requests_) {
            query[timestamp_param_] = Yeast();
        }
        
        auto query_str = QueryStringEncode(query);
    
        // prepend ? to query
        if (query_str.length() > 0) {
            query_str = "?" + query_str;
        }
    
        bool ipv6 = hostname_.find(":") != std::string::npos;
        
        auto hostname_str = ipv6 ? "[" + hostname_ + "]" : hostname_;
        
        return schema + "://" + hostname_str + port + path_ + query_str;
    }
    
    void WebsocketTransport::Write(const std::vector<Packet> & packets) {
        writable_ = false;
        
        // encodePacket efficient as it uses WS framing
        // no need for encodePayload
        for (auto packet : packets) {
            auto message = EncodePacket(packet);
            
            ws_->Send(message);
        }
        
        flush_emitter_->Emit(None());
        
        // fake drain
        // defer to next tick to allow Socket to clear writeBuffer
        timer_pool_.SetTimeout(TimeDuration(0), [this]{
            writable_ = true;
            drain_emitter_->Emit(None());
        });
    }
    
    void WebsocketTransport::DoClose() {
        if (ws_) {
            ws_->Close();
            ws_ = nullptr;
        }
    }
    
}
}