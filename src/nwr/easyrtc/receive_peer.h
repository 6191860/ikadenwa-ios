//
//  receive_peer.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/13.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <string>
#include <map>
#include <functional>

#include <nwr/base/func.h>
#include <nwr/base/any.h>

namespace nwr {
namespace ert {
    using ReceivePeerCallback = std::function<void(const std::string &,
                                                   const std::string &,
                                                   const Any &,
                                                   const Any &)>;
    
    struct ReceivePeerSourceEntry {
        ReceivePeerCallback cb;
    };
    
    struct ReceivePeerMsgEntry {
        ReceivePeerCallback cb;
        std::map<std::string, ReceivePeerSourceEntry> sources;
    };
    
    struct ReceivePeer {
        void Clear();
        
        ReceivePeerCallback cb;
        std::map<std::string, ReceivePeerMsgEntry> msg_types;
    };
}
}
