//
//  media_track_constraint.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/22.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <memory>
#include <nwr/base/array.h>
#include <nwr/base/string.h>
#include <nwr/base/lib_webrtc.h>

namespace nwr {
namespace jsrtc {
    
    class MediaTrackConstraintSet {
    public:
        webrtc::MediaConstraintsInterface::Constraints & inner_entries();
        const webrtc::MediaConstraintsInterface::Constraints & inner_entries() const;
        
        void SetEntry(const std::string & key, const std::string & value);
        void SetEntry(const std::string & key, bool value);
        void SetEntry(const std::string & key, int value);
        void SetEntry(const std::string & key, double value);
    private:
        webrtc::MediaConstraintsInterface::Constraints entries_;
    };

    class MediaTrackConstraints {
    public:
        MediaTrackConstraints();
        
        webrtc::MediaConstraintsInterface & inner_constraints();
        const webrtc::MediaConstraintsInterface & inner_constraints() const;
        
        MediaTrackConstraintSet & mandatory();
        const MediaTrackConstraintSet & mandatory() const;
        MediaTrackConstraintSet & optional();
        const MediaTrackConstraintSet & optional() const;
    private:
        std::shared_ptr<webrtc::MediaConstraintsInterface> adapter_;
        MediaTrackConstraintSet mandatory_;
        MediaTrackConstraintSet optional_;
    };

}
}
