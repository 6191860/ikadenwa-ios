//
//  ios_looper.mm
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/19.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "ios_looper.h"

namespace nwr {
    IosLooper::IosLooper() {
        operation_queue_ = [[NSOperationQueue alloc] init];
    }
    void IosLooper::PostTask(const Task & task) {
        Task task_copy = task;
        [operation_queue_ addOperationWithBlock:^{
            task_copy();
        }];
    }
    void IosLooper::Quit() {
        [operation_queue_ waitUntilAllOperationsAreFinished];
        operation_queue_ = nil;
    }
}
