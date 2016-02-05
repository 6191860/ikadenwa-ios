//
//  ios_looper.h
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/19.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#import <Foundation/Foundation.h>

#include "looper.h"

namespace nwr {
    class IosLooper: public Looper {
    public:
        IosLooper();
        virtual ~IosLooper() {}
        virtual void PostTask(const Task & task);
        virtual void Quit();
    private:
        NSOperationQueue * operation_queue_;
    };
}
