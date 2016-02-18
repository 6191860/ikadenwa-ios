//
//  websocket_listener.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/19.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <string>
#include <nwr/base/any_emitter.h>

namespace nwr {
namespace ert {
    struct WebsocketListenerEntry {
        std::string event;
        AnyEventListener handler;
    };
}
}
