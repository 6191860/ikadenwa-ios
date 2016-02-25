//
//  http_operation.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/25.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <memory>
#include <string>
#include <map>
#include <nwr/base/data.h>

namespace nwr {
    class HttpOperationImpl;
    
    struct HttpRequest {
        HttpRequest();
        HttpRequest(const std::string & url,
                    const std::string & method,
                    const std::map<std::string, std::string> & headers);
        std::string url;
        std::string method;
        std::map<std::string, std::string> headers;
    };
    
    struct HttpResponse {
        int code;
        std::map<std::string, std::string> headers;
        DataPtr data;
    };
    
    class HttpOperation : public std::enable_shared_from_this<HttpOperation> {
    public:
        static std::shared_ptr<HttpOperation> Create(const HttpRequest & request);
        
        void set_on_success(const std::function<void(const HttpResponse &)> & value);
        void set_on_failure(const std::function<void(const std::string &)> & value);
        
        void Cancel();
    private:
        HttpOperation();
        std::shared_ptr<HttpOperationImpl> impl_;
    };
    

}
