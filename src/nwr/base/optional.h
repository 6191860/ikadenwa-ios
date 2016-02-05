//
//  optional.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/24.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <utility>

namespace nwr {
    template <typename T> class Optional {
    public:
        Optional(): value_(nullptr) {
        }
        Optional(const Optional & copy): value_(nullptr) {
            *this = copy;
        }

        Optional(Optional && move): value_(nullptr) {
            *this = move;
        }
        
        explicit Optional(const T & value): value_(nullptr) {
            set_value(value);
        }
        explicit Optional(T && value): value_(nullptr) {
            set_value(value);
        }
        
        ~Optional() {
            set_null();
        }
        
        bool presented() const { return value_ != nullptr; }
        
        T value() const { return *value_; }
        
        T Recover(const T & value) const {
            return presented() ? this->value() : value;
        }
        
        Optional<T> & operator=(const Optional & copy) {
            if (copy.value_) {
                set_value(*copy.value_);
            } else {
                set_null();
            }
            return *this;
        }
        Optional<T> & operator=(Optional && move) {
            set_null();
            if (move.value_) {
                value_ = move.value_;
                move.value_ = nullptr;
            }
            return *this;
        }
    private:
        void set_null() {
            Release();
            value_ = nullptr;
        }
        void set_value(const T & value) {
            Release();
            value_ = new T (value);
        }
        void set_value(T && value) {
            Release();
            value_ = new T (value);
        }
        void Release() {
            if (value_) {
                delete value_;
                value_ = nullptr;
            }
        }
        T * value_;
    };
    
    template <typename T> Optional<T> OptionalSome(const T & value) {
        return Optional<T>(value);
    }
}