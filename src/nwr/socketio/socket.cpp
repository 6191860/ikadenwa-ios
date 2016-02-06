//
//  socket.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/24.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "socket.h"

namespace nwr {
namespace sio {
    std::shared_ptr<Socket> Socket::Create(int io, int nsp) {
        auto thiz = std::shared_ptr<Socket>(new Socket());
        thiz->Init(io, nsp);
        return thiz;
    }
    Socket::Socket() {
    }
    void Socket::Init(int io, int nsp) {
        
    }
}
}
