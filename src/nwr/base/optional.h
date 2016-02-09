//
//  optional.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/24.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <utility>

#include "none.h"

namespace nwr {
    template <typename T> class Optional {
    public:
        Optional(): value_(nullptr) {
        }
        Optional(const Optional<T> & copy): value_(nullptr) {
            *this = copy;
        }
        Optional(Optional<T> && move): value_(nullptr) {
            *this = move;
        }
        Optional(None value): value_(nullptr) {
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
        
        const T & operator* () const { return *value_; }
        T & operator* () { return *value_; }
        
        const T * operator-> () const { return value_; }
        T * operator-> () { return value_; }
        
        T operator||(const T & value) const {
            return *this ? **this : value;
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
    
    template <typename T> Optional<T> Some(const T & value) {
        return Optional<T>(value);
    }
}