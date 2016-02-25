//
//  transport.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/25.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <memory>
#include <nwr/base/timer.h>
#include <nwr/base/emitter.h>

namespace nwr {
namespace sio0 {
    class Packet;
    class CoreSocket;
    class Transport : public std::enable_shared_from_this<Transport> {
    public:
        Transport(const std::shared_ptr<CoreSocket> & socket, const std::string & sessid);
        
        virtual std::string name() = 0;
        virtual void Open() = 0;
        virtual void Send(const std::string & data) = 0;
        virtual void SendPayload(const std::vector<Packet> & payload) = 0;
        virtual void Close() = 0;
        virtual std::string scheme() = 0;

        bool heartbeats();
    protected:
        std::shared_ptr<CoreSocket> socket();
    public:
        void OnData(const std::string & data);
    private:
        void OnPacket(const Packet & packet);
        void SetCloseTimeout();
        void OnDisconnect();
        void OnConnect();
        void ClearCloseTimeout();
    public:
        void ClearTimeouts();
    
        void SendPacket(const Packet & packet);
    private:
        void OnHeartbeat();
    public:
        void OnOpen();
        void OnClose();
        std::string PrepareUrl();
        
        std::shared_ptr<CoreSocket> socket_;
        std::string sessid_;
        bool is_open_;
        TimerPtr close_timeout_;
        TimerPtr reopen_timeout_;
    };
}
}