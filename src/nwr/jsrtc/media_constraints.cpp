//
//  media_constraints.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/16.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "media_constraints.h"

namespace nwr {
namespace jsrtc {
    
    MediaConstraints::MediaConstraints() {
        
    }
    
    MediaConstraints::MediaConstraints(const webrtc::MediaConstraintsInterface::Constraints & mandatory,
                                       const webrtc::MediaConstraintsInterface::Constraints & optional):
    mandatory_(mandatory),
    optional_(optional){}
    
    const webrtc::MediaConstraintsInterface::Constraints& MediaConstraints::GetMandatory() const {
        return mandatory_;
    }
    
    const webrtc::MediaConstraintsInterface::Constraints& MediaConstraints::GetOptional() const {
        return optional_;
    }
    
    webrtc::MediaConstraintsInterface::Constraints & MediaConstraints::mandatory() {
        return mandatory_;
    }
    webrtc::MediaConstraintsInterface::Constraints & MediaConstraints::optional() {
        return optional_;
    }
    
    void MediaConstraintsSet(webrtc::MediaConstraintsInterface::Constraints & constraints,
                             const std::string & key,
                             const std::string & value)
    {
        RemoveIf(constraints, [key](const webrtc::MediaConstraintsInterface::Constraint & x){
            return x.key == key;
        });
        constraints.push_back(webrtc::MediaConstraintsInterface::Constraint(key, value));
    }
    
    std::string MediaConstraintsBoolValue(bool value) {
        return value ? webrtc::MediaConstraintsInterface::kValueTrue : webrtc::MediaConstraintsInterface::kValueFalse;
    }
    
}
}