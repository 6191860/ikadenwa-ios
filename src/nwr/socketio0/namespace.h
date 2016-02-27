//
//  namespace.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/25.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <memory>
#include <map>
#include <functional>
#include <nwr/base/map.h>
#include <nwr/base/any.h>
#include <nwr/base/any_emitter.h>
#include <nwr/base/any_func.h>

#include "parser.h"
#include "socket.h"

namespace nwr {
namespace sio0 {
    class Socket : public std::enable_shared_from_this<Socket> {
    public:
        Socket(const std::shared_ptr<CoreSocket> & socket,
               const std::string & name);
        virtual ~Socket();
        
        AnyEmitterPtr emitter() const;
        std::shared_ptr<CoreSocket> socket() const;
        
        std::shared_ptr<Socket> Of(const std::string & name);
        void SendPacket(const Packet & packet);
        void Send(const Any & data,
                  const AnyFuncPtr & ack = nullptr);
        void JsonSend(const Any & data,
                      const AnyFuncPtr & ack = nullptr);
        void Emit(const std::string & name,
                  const std::vector<Any> & args);
        void JsonEmit(const std::string & name,
                      const std::vector<Any> & args);
        void Disconnect();
        void OnPacket(const Packet & packet);

    private:
        std::shared_ptr<CoreSocket> socket_;
        std::string name_;
        std::map<std::string, bool> flags_;
        int ack_packets_;
        std::map<int, AnyFuncPtr> acks_;
        AnyEmitterPtr emitter_;
    };
}
}