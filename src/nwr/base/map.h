//
//  map.h
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/12.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <vector>
#include <map>
#include <functional>

namespace nwr {
    template <typename K, typename V, typename F>
    std::vector<std::pair<K, V>> Map(const std::map<K, V> & map, const F & mapf) {
        std::vector<std::pair<K, V>> r;
        for (const auto & pair : map) {
            r.push_back(mapf(pair.first, pair.second));
        }
        return r;
    }
    
    template <typename K, typename V, typename F>
    std::map<K, V> MapToMap(const std::map<K, V> & map, const F & mapf) {
        std::vector<std::pair<K, V>> pairs = Map(map, mapf);
        return std::map<K, V>(pairs.begin(), pairs.end());
    }
}
