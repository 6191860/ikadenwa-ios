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
            set_value(new T (value));
        }
        explicit Optional(T && value): value_(nullptr) {
            set_value(new T (value));
        }
        ~Optional() {
            set_value(nullptr);
        }
        
        explicit operator bool() const { return value_ != nullptr; }
        
        const T & value() const { return *value_; }
        T & value() { return * value_; }
        
        T Recover(const T & value) const {
            return *this ? this->value() : value;
        }
        
        Optional<T> & operator=(const Optional & copy) {
            if (copy.value_) {
                set_value(new T(*copy.value_));
            } else {
                set_value(nullptr);
            }
            return *this;
        }
        Optional<T> & operator=(Optional && move) {
            if (move.value_) {
                set_value(move.value_);
                move.value_ = nullptr;
            } else {
                set_value(nullptr);
            }
            return *this;
        }
    private:
        void set_value(T * value) {
            Release();
            value_ = value;
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
    template <typename T> Optional<T> OptionalNone() {
        return Optional<T>();
    }
}