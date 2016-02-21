//
//  media_stream_constraints.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/22.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "media_stream_constraints.h"

namespace nwr {
namespace jsrtc {
    Optional<MediaTrackConstraints> & MediaStreamConstraints::video() {
        return video_;
    }
    Optional<MediaTrackConstraints> & MediaStreamConstraints::audio() {
        return audio_;
    }
}
}
