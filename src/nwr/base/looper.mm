//
//  looper.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/19.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "looper.h"

#include "ios_looper.h"

namespace nwr {
    std::shared_ptr<Looper> Looper::Create() {
        std::shared_ptr<Looper> thiz(new IosLooper());
        return thiz;
    }

}