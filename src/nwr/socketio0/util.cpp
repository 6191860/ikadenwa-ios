//
//  util.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/26.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "util.h"

namespace nwr {
namespace sio0 {
    std::string MergeQuery(const std::string & base, const std::string & add) {
        auto base_query = QueryStringDecode(base);
        auto add_query = QueryStringDecode(add);
        
        for (const auto & key : Keys(add_query)) {
            base_query[key] = add_query[key];
        }
        
        if (base_query.size() == 0) { return ""; }
        
        return "?" + QueryStringEncode(base_query);
    }
}
}

