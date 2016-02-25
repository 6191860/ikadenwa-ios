//
//  websocket_transport.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/25.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <nwr/base/data.h>
#include <nwr/base/websocket.h>
#include "transport.h"

namespace nwr {
namespace sio0 {
    class CoreSocket;
    class WebsocketTransport : public Transport {
    public:
        WebsocketTransport(const std::shared_ptr<CoreSocket> & socket,
                           const std::string & sessid);
        
        std::string name() override;
        void Open() override;
        void Send(const std::string & data) override;
        void SendPayload(const std::vector<Packet> & payload) override;
        void Close() override;
        std::string scheme() override;
        
    private:
        void OnError(const std::string & error);
        std::shared_ptr<Websocket> websocket_;
        
    };
}
}
