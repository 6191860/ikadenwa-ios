//
//  media_stream_constraints.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/22.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <nwr/base/optional.h>
#include "media_track_constraints.h"

namespace nwr {
namespace jsrtc {
    class MediaStreamConstraints {
    public:
        Optional<MediaTrackConstraints> & video();
        Optional<MediaTrackConstraints> & audio();
    private:
        Optional<MediaTrackConstraints> video_;
        Optional<MediaTrackConstraints> audio_;
    };
}
}
