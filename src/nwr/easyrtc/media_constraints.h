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

#include <talk/app/webrtc/mediaconstraintsinterface.h>

#include <nwr/base/array.h>
#include <nwr/base/optional.h>

namespace nwr {
namespace ert {
    void MediaConstraintsSet(webrtc::MediaConstraintsInterface::Constraints & constraints,
                             const std::string & key,
                             const std::string & value);
    
    struct MediaConstraints: public webrtc::MediaConstraintsInterface {
        MediaConstraints();
        
        const webrtc::MediaConstraintsInterface::Constraints& GetMandatory() const override;
        const webrtc::MediaConstraintsInterface::Constraints& GetOptional() const override;

        webrtc::MediaConstraintsInterface::Constraints mandatory;
        webrtc::MediaConstraintsInterface::Constraints optional;
    };
    
    struct VideoAudioMediaConstraints {
        VideoAudioMediaConstraints();
        
        Optional<MediaConstraints> video;
        bool audio;
    };
}
}
