//
//  media_constraints.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/13.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "media_constraints.h"

namespace nwr {
namespace ert {
    void MediaConstraintsSet(webrtc::MediaConstraintsInterface::Constraints & constraints,
                             const std::string & key,
                             const std::string & value)
    {
        RemoveIf(constraints, [key](const webrtc::MediaConstraintsInterface::Constraint & x){
            return x.key == key;
        });
        constraints.push_back(webrtc::MediaConstraintsInterface::Constraint(key, value));
    }
    
    MediaConstraints::MediaConstraints() {
        
    }
    
    MediaConstraints::MediaConstraints(const webrtc::MediaConstraintsInterface::Constraints & mangatory,
                                       const webrtc::MediaConstraintsInterface::Constraints & optional):
    mandatory(mandatory),
    optional(optional){}
    
    const webrtc::MediaConstraintsInterface::Constraints& MediaConstraints::GetMandatory() const {
        return mandatory;
    }
    
    const webrtc::MediaConstraintsInterface::Constraints& MediaConstraints::GetOptional() const {
        return optional;
    }
    
    VideoAudioMediaConstraints::VideoAudioMediaConstraints():
    audio(false)
    {}
}
}