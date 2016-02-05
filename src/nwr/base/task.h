//
//  task.h
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/18.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <functional>

namespace nwr {
    using Task = std::function<void()>;
}
