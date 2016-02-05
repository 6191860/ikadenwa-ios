//
//  socket.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/24.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <nwr/base/emitter.h>
#include <nwr/base/none.h>

namespace nwr {
namespace sio {
    
    class Socket {
    public:
        Socket(int io, int nsp);
    private:
        EmitterPtr<None> connect_emitter_;
        EmitterPtr<None> connect_error_emitter_;
        EmitterPtr<None> connect_timeout_emitter_;
        EmitterPtr<None> connecting_emitter_;
        EmitterPtr<None> disconnect_emitter_;
        EmitterPtr<None> error_emitter_;
        EmitterPtr<None> reconnect_emitter_;
        EmitterPtr<None> reconnect_attempt_emitter_;
        EmitterPtr<None> reconnect_failed_emitter_;
        EmitterPtr<None> reconnect_error_emitter_;
        EmitterPtr<None> reconnecting_emitter_;
        EmitterPtr<None> ping_emitter_;
        EmitterPtr<None> pong_emitter_;
    };
    
    
}
}
