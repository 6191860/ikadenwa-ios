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
    const std::shared_ptr<MediaTrackConstraints> MediaStreamConstraints::video() const {
        return video_;
    }
    
    void MediaStreamConstraints::set_video(const std::shared_ptr<MediaTrackConstraints> & value) {
        video_ = value;
    }

    const std::shared_ptr<MediaTrackConstraints> MediaStreamConstraints::audio() const {
        return audio_;
    }
    
    void MediaStreamConstraints::set_audio(const std::shared_ptr<MediaTrackConstraints> & value) {
        audio_ = value;
    }
}
}
