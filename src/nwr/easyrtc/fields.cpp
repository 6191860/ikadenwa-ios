//
//  fields.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/21.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "fields.h"

namespace nwr {
namespace ert {
    void Fields::Clear() {
        rooms.clear();
        application.clear();
        connection.clear();
    }
}
}