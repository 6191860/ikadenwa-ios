//
//  sio_test.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/11.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <memory>
#include <nwr/base/any.h>
#include <nwr/base/timer.h>
#include <nwr/engineio/socket.h>
#include <nwr/socketio/io.h>
#include <nwr/socketio0/io.h>

namespace app {
    class NwrTestSet {
    public:
        void TestAnyType();
        void TestEio();
        void TestSio();
        void TestSio0();
    };
}
