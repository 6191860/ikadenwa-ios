//
//  error.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/20.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "error.h"
#include "env.h"
#include "string.h"

namespace nwr {
    Error::Error(const Error & copy) {
        *this = copy;
    }
    Error & Error::operator=(const Error & copy) {
        type_ = copy.type_;
        message_ = copy.message_;
        causer_ = copy.causer_;
        return *this;
    }
    
    Error::Error(const std::string & type, const std::string & message):
    Error(type, message, nullptr){}
    Error::Error(const std::string & type, const std::string & message, const ErrorPtr & causer):
    type_(type), message_(message), causer_(causer) {}
    
    std::string Error::type() const { return type_; }
    std::string Error::message() const { return message_; }
    ErrorPtr Error::causer() const { return causer_; }
    
    std::string Error::Dump() const {
        std::vector<std::string> lines;
        auto err = std::make_shared<Error>(*this);
        while (err) {
            lines.push_back(Format("%s:%s", err->type_.c_str(), err->message_.c_str()));
        }
        return Join(lines, "\n");
    }
}