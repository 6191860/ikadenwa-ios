//
//  http_operation_impl.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/25.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#import <Foundation/Foundation.h>

#include <memory>
#include <functional>
#include <nwr/base/data.h>
#include <nwr/base/func.h>
#include <nwr/base/ios_task_queue.h>
#include "http_operation.h"

@class NWRHttpOperationNSURLSessionDelegateAdapter;

namespace nwr {
    class HttpOperationImpl : public std::enable_shared_from_this<HttpOperationImpl> {
        friend HttpOperation;
    public:
        HttpOperationImpl();
        void Start(const HttpRequest & request);
        void Cancel();
    private:
        bool canceled_;
        NSURLSession * session_;
        NSURLSessionDataTask * task_;
        NWRHttpOperationNSURLSessionDelegateAdapter * delegate_adapter_;
        HttpResponse response_;
        Data data_;
        
        std::function<void(const HttpResponse &)> on_success_;
        std::function<void(const std::string &)> on_failure_;
    };
}

