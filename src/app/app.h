//
//  App.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/16.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <cstdio>
#include <string>
#include <thread>
#include <memory>

#include <nwr/base/env.h>
#include <nwr/base/string.h>
#include <nwr/base/websocket.h>
#include <nwr/base/timer.h>
#include <nwr/base/base64.h>
#include <nwr/base/time.h>
#include <nwr/engineio/yeast.h>
#include <nwr/engineio/socket.h>

#include "eio_test.h"
#include "any_test.h"
#include "sio_test.h"

namespace app {
    class App {
    public:
        void Test1();
        
        std::shared_ptr<EioTest> eio_test_;
        std::shared_ptr<AnyTest> any_test_;
        std::shared_ptr<SioTest> sio_test_;
    };
}
