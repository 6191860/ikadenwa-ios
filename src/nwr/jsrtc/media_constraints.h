//
//  media_constraints.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/16.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <nwr/base/array.h>
#include "lib_webrtc.h"

namespace nwr {
namespace jsrtc {
    class MediaConstraints: public webrtc::MediaConstraintsInterface {
    public:
        MediaConstraints();
        MediaConstraints(const webrtc::MediaConstraintsInterface::Constraints & mandatory,
                         const webrtc::MediaConstraintsInterface::Constraints & optional);
        
        const webrtc::MediaConstraintsInterface::Constraints & GetMandatory() const override;
        const webrtc::MediaConstraintsInterface::Constraints & GetOptional() const override;
        
        webrtc::MediaConstraintsInterface::Constraints & mandatory();
        webrtc::MediaConstraintsInterface::Constraints & optional();
    private:
        webrtc::MediaConstraintsInterface::Constraints mandatory_;
        webrtc::MediaConstraintsInterface::Constraints optional_;
    };
    
    void MediaConstraintsSet(webrtc::MediaConstraintsInterface::Constraints & constraints,
                             const std::string & key,
                             const std::string & value);
    std::string MediaConstraintsBoolValue(bool value);
}
}
