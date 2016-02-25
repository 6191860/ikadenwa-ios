//
//  http_operation.cpp
//  :
//
//  Created by omochimetaru on 2016/02/25.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "http_operation.h"

#include "http_operation_impl.h"

namespace nwr {
    HttpRequest::HttpRequest()
    {}
    HttpRequest::HttpRequest(const std::string & url,
                             const std::string & method,
                             const std::map<std::string, std::string> & headers):
    url(url),
    method(method),
    headers(headers)
    {}
    
    
    std::shared_ptr<HttpOperation> HttpOperation::Create(const HttpRequest & request)
    {
        auto thiz = std::shared_ptr<HttpOperation>(new HttpOperation());
        thiz->impl_->Start(request);
        return thiz;
    }
    void HttpOperation::set_on_success(const std::function<void(const HttpResponse &)> & value)
    {
        impl_->on_success_ = value;
    }
    void HttpOperation::set_on_failure(const std::function<void(const std::string &)> & value)
    {
        impl_->on_failure_ = value;
    }
    void HttpOperation::Cancel() {
        impl_->Cancel();
    }
    HttpOperation::HttpOperation():
    impl_(std::make_shared<HttpOperationImpl>())
    {}
}
