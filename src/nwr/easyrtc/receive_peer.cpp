//
//  receive_peer.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/13.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "receive_peer.h"

namespace nwr {
namespace ert {
    
    void ReceivePeer::Clear() {
        cb = nullptr;
        msg_types.clear();
    }
    
}
}