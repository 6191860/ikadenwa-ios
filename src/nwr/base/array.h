//
//  array.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/11.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <vector>
#include <algorithm>
#include <functional>

#include "type_helper.h"

namespace nwr {
    template <typename T>
    void Remove(std::vector<T> & array, const T & item) {
        auto iter = std::remove(array.begin(), array.end(), item);
        array.erase(iter, array.end());
    }
    template <typename T, typename F>
    void RemoveIf(std::vector<T> & array, const F & pred) {
        auto iter = std::remove_if(array.begin(), array.end(), pred);
        array.erase(iter, array.end());
    }
    template <typename T>
    void RemoveItems(std::vector<T> & array, const std::vector<T> & items) {
        RemoveIf(array, [items](const T & x) {
            return IndexOf(items, x) != -1;
        });
    }
    
    template <typename T>
    int IndexOf(const std::vector<T> & array, const T & item) {
        auto iter = std::find(array.begin(), array.end(), item);
        if (iter != array.end()){
            return static_cast<int>(iter - array.begin());
        } else {
            return -1;
        }
    }
    template <typename T, typename F>
    int IndexOfIf(const std::vector<T> & array, const F & pred) {
        auto iter = std::find_if(array.begin(), array.end(), pred);
        if (iter != array.end()){
            return static_cast<int>(iter - array.begin());
        } else {
            return -1;
        }
    }
    
//    template <typename T, typename U>
//    std::vector<U> Map(const std::vector<T> & array, const std::function<U (const T &)> & map) {
//        std::vector<U> r;
//        for (const auto & x : array) {
//            r.push_back(map(x));
//        }
//        return r;
//    }

    template <typename T, typename F>
    std::vector<typename functor_return_type<decltype(&F::operator())>::type>
    Map(const std::vector<T> & array, const F & mapf) {
        std::vector<typename functor_return_type<decltype(&F::operator())>::type> r;
        for (const auto & x : array) {
            r.push_back(mapf(x));
        }
        return r;
    }

    template <typename T, typename F>
    std::vector<T> Filter(const std::vector<T> & array, const F & pred) {
        std::vector<T> r;
        for (const auto & x : array) {
            if (pred(x)) {
                r.push_back(x);
            }
        }
        return r;
    }
}
