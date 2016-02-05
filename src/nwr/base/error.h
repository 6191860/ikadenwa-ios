//
//  error.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/20.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <memory>
#include <string>
#include <vector>

namespace nwr {
    class Error;
    
    using ErrorPtr = std::shared_ptr<Error>;

    class Error {
    public:
        Error(const Error & copy);
        Error & operator=(const Error & copy);
        
        Error(const std::string & type, const std::string & message);
        Error(const std::string & type, const std::string & message, const ErrorPtr & causer);
        
        std::string type() const;
        std::string message() const;
        ErrorPtr causer() const;
        
        std::string Dump() const;
    private:
        std::string type_;
        std::string message_;
        ErrorPtr causer_;
    };
}

