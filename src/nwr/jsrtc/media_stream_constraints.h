//
//  media_stream_constraints.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/22.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <memory>
#include <nwr/base/optional.h>
#include "media_track_constraints.h"

namespace nwr {
namespace jsrtc {
    class MediaStreamConstraints {
    public:
        const std::shared_ptr<MediaTrackConstraints> video() const;
        void set_video(const std::shared_ptr<MediaTrackConstraints> & value);
        const std::shared_ptr<MediaTrackConstraints> audio() const;
        void set_audio(const std::shared_ptr<MediaTrackConstraints> & value);
    private:
        std::shared_ptr<MediaTrackConstraints> video_;
        std::shared_ptr<MediaTrackConstraints> audio_;
    };
}
}
