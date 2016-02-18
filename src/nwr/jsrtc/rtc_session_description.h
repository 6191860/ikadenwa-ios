//
//  rtc_session_description.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/17.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <memory>
#include <nwr/base/any.h>
#include "lib_webrtc.h"

namespace nwr {
namespace jsrtc {
    class RtcSessionDescription {
    public:
        RtcSessionDescription(const std::string & type,
                              const std::string & sdp);
        
        std::string type() const;
        void set_type(const std::string & value);
        std::string sdp() const;
        void set_sdp(const std::string & value);
        
        Any ToAny() const;
        static std::shared_ptr<RtcSessionDescription> FromAny(const Any & any);
        
        webrtc::SessionDescriptionInterface * CreateWebrtc() const;
        static webrtc::SessionDescriptionInterface * CreateWebrtc(const std::string & type,
                                                                  const std::string & sdp);
        static std::shared_ptr<RtcSessionDescription> FromWebrtc(const webrtc::SessionDescriptionInterface & wdesc);
    private:
        std::string type_;
        std::string sdp_;
    };
    
}
}
