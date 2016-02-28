//
//  io.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/25.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <string>
#include <nwr/base/url.h>
#include <nwr/base/any.h>

#include "socket.h"
#include "namespace.h"

namespace nwr {
namespace sio0 {
    class Socket;
    class Io {
    public:
        static std::string version_;
        static int protocol_;
        static std::map<std::string, std::shared_ptr<CoreSocket>> sockets_;
        //  transports
        //  j
        //  sockets
        
        static std::shared_ptr<Socket> Connect(const std::string & url, const SocketOptions & details);
    };
}
}
