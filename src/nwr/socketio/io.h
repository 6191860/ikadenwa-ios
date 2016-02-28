//
//  io.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/11.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <memory>
#include <string>
#include <map>

#include <nwr/engineio/socket.h>
#include "socket.h"

namespace nwr {
namespace sio {
    class Socket;
    
    struct IoParams {
        IoParams();
        
        bool force_new;
        bool multiplex;
    };
    
    std::shared_ptr<Socket> Io(const std::string & uri, const eio::Socket::ConstructorParams & params);
    
    extern std::map<std::string, std::shared_ptr<Manager>> cache_;
}
}
