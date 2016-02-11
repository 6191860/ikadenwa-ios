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

namespace nwr {
    template <typename T>
    void Remove(std::vector<T> & array, const T & item) {
        auto iter = std::remove(array.begin(), array.end(), item);
        array.erase(iter, array.end());
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
    
    template <typename T, typename U>
    std::vector<U> Map(const std::vector<T> & array, const std::function<U (const T &)> & map) {
        std::vector<U> r;
        for (const auto & x : array) {
            r.push_back(map(x));
        }
        return r;
    }
}
