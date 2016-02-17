//
//  media_constraints.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/13.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <vector>
#include <algorithm>
#include <functional>
#include <nwr/base/optional.h>
#include <nwr/jsrtc/media_constraints.h>

namespace nwr {
namespace ert {
    
    struct VideoAudioConstraints {
        VideoAudioConstraints();
        
        Optional<jsrtc::MediaConstraints> video;
        bool audio;
    };
}
}
