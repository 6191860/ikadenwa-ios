//
//  any_func.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/21.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include "any_func_forward.h"
#include "any.h"

namespace nwr {
    template <> class AnyFuncImpl <void()> : public AnyFunc {
    public:
        AnyFuncImpl(const std::function<void()> & func): func_(func) {}
        Any Call(const std::vector<Any> & args) const override {
            func_();
            return nullptr;
        }
    private:
        std::function<void()> func_;
    };
    
    template <> class AnyFuncImpl <Any()> : public AnyFunc {
    public:
        AnyFuncImpl(const std::function<Any()> & func): func_(func) {}
        Any Call(const std::vector<Any> & args) const override {
            return func_();
        }
    private:
        std::function<Any()> func_;
    };
    
    template <> class AnyFuncImpl <void(const Any &)> : public AnyFunc {
    public:
        AnyFuncImpl(const std::function<void(const Any &)> & func): func_(func) {}
        Any Call(const std::vector<Any> & args) const override {
            Any a0 = 0 < args.size() ? args[0] : nullptr;
            func_(a0);
            return nullptr;
        }
    private:
        std::function<void(const Any &)> func_;
    };
    
    template <> class AnyFuncImpl <Any(const Any &)> : public AnyFunc {
    public:
        AnyFuncImpl(const std::function<Any(const Any &)> & func): func_(func) {}
        Any Call(const std::vector<Any> & args) const override {
            Any a0 = 0 < args.size() ? args[0] : nullptr;
            return func_(a0);
        }
    private:
        std::function<Any(const Any &)> func_;
    };
    
    template <> class AnyFuncImpl <void(const Any &, const Any &)> : public AnyFunc {
    public:
        AnyFuncImpl(const std::function<void(const Any &, const Any &)> & func): func_(func) {}
        Any Call(const std::vector<Any> & args) const override {
            Any a0 = 0 < args.size() ? args[0] : nullptr;
            Any a1 = 1 < args.size() ? args[1] : nullptr;
            func_(a0, a1);
            return nullptr;
        }
    private:
        std::function<void(const Any &, const Any &)> func_;
    };
    
    template <> class AnyFuncImpl <Any(const Any &, const Any &)> : public AnyFunc {
    public:
        AnyFuncImpl(const std::function<Any(const Any &, const Any &)> & func): func_(func) {}
        Any Call(const std::vector<Any> & args) const override {
            Any a0 = 0 < args.size() ? args[0] : nullptr;
            Any a1 = 1 < args.size() ? args[1] : nullptr;
            return func_(a0, a1);
        }
    private:
        std::function<Any(const Any &, const Any &)> func_;
    };
    
    template <> class AnyFuncImpl <void(const std::vector<Any> &)> : public AnyFunc {
    public:
        AnyFuncImpl(const std::function<void(const std::vector<Any> &)> & func): func_(func) {}
        Any Call(const std::vector<Any> & args) const override {
            func_(args);
            return nullptr;
        }
    private:
        std::function<void(const std::vector<Any> &)> func_;
    };
    
    template <> class AnyFuncImpl <Any(const std::vector<Any> &)> : public AnyFunc {
    public:
        AnyFuncImpl(const std::function<Any(const std::vector<Any> &)> & func): func_(func) {}
        Any Call(const std::vector<Any> & args) const override {
            return func_(args);
        }
    private:
        std::function<Any(const std::vector<Any> &)> func_;
    };
    
    template <typename F>
    AnyFuncPtr AnyFuncMake(const F & func) {
        auto impl = new AnyFuncImpl<typename functor_func_ptr_type<decltype(&F::operator())>::type>(func);
        return AnyFuncPtr(impl);
    }
}