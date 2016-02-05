//
//  websocket_transport.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/21.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

//  https://github.com/socketio/engine.io-client

#pragma once

#include <nwr/base/timer.h>
#include <nwr/base/timer_pool.h>

#include "transport.h"

namespace nwr {
namespace eio {
    
    class WebsocketTransport: public Transport {
    public:
        WebsocketTransport(const Transport::ConstructorParams & params);
        virtual ~WebsocketTransport() {}
        
        virtual std::string name() { return "websocket"; }        
    protected:
        
        virtual void DoOpen();
        void AddEventListeners();
        std::string uri();
        virtual void Write(const std::vector<Packet> & packets);
        virtual void DoClose();
        
    private:
        
        std::shared_ptr<Websocket> ws_;
        
        TimerPool timer_pool_;
        

    };
    
}
}