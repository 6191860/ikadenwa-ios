//
//  eio_test.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/05.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <memory>

#include <nwr/engineio/socket.h>

namespace app {
    
    class EioTest: public std::enable_shared_from_this<EioTest> {
    public:
        void Run();
    };
}