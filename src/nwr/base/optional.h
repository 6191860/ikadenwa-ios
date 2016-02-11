//
//  optional.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/24.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <utility>
#include <functional>

#include "none.h"

namespace nwr {
    template <typename T> class Optional;
    
    template <typename T> Optional<T> Some(const T & value);
    
    template <typename T> class Optional {
    public:
        using ValueType = T;
        
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
            set_value_raw(new T (value));
        }
        explicit Optional(T && value): value_(nullptr) {
            set_value_raw(new T (value));
        }
        ~Optional() {
            set_value_raw(nullptr);
        }
        
        explicit operator bool() const { return value_ != nullptr; }
        
        const T & value() const { return *value_; }
        T & value() { return *value_; }
        
        const T & operator* () const { return *value_; }
        T & operator* () { return *value_; }
        
        const T * operator-> () const { return value_; }
        T * operator-> () { return value_; }
        
        T operator||(const T & value) const {
            return *this ? **this : value;
        }
        
        Optional<T> & operator=(const Optional<T> & copy) {
            if (copy.value_) {
                set_value_raw(new T(*copy.value_));
            } else {
                set_value_raw(nullptr);
            }
            return *this;
        }
        Optional<T> & operator=(Optional<T> && move) {
            if (move.value_) {
                set_value_raw(move.value_);
                move.value_ = nullptr;
            } else {
                set_value_raw(nullptr);
            }
            return *this;
        }
        
        bool operator== (const Optional<T> & cmp) const {
            if (*this) {
                return cmp && value() == cmp.value();
            } else {
                return !cmp;
            }
        }
        bool operator!= (const Optional<T> & cmp) const {
            return !(*this == cmp);
        }
        
        template <typename U> Optional<U> Map (const std::function<U(const T &)> & f) const {
            return *this ? Some(f(value())) : None();
        }
        template <typename U> Optional<U> FlatMap (const std::function<Optional<U>(const T &)> & f) const {
            return *this ? f(value()) : None();
        }
    private:
        void set_value_raw(T * value) {
            if (value_ != value) {
                Release();
                value_ = value;
            }
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