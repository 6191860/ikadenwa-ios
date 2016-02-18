//
//  rtc_ice_candidate.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/19.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <string>
#include <memory>
#include <nwr/base/any.h>
#include "lib_webrtc.h"

namespace nwr {
namespace jsrtc {
    class RtcIceCandidate {
    public:
        RtcIceCandidate(const std::string & sdp_mid,
                        int sdp_mline_index,
                        const std::string & candidate);
        
        std::string sdp_mid() const;
        void set_sdp_mid(const std::string & value);
        int sdp_mline_index() const;
        void set_sdp_mline_index(int value);
        std::string candidate() const;
        void set_candidate(const std::string & value);
        
        Any ToAny() const;
        static std::shared_ptr<RtcIceCandidate> FromAny(const Any & any);
        
        webrtc::IceCandidateInterface * CreateWebrtc() const;
        static webrtc::IceCandidateInterface * CreateWebrtc(const std::string & sdp_mid,
                                                            int sdp_mline_index,
                                                            const std::string & candidate);
        static std::shared_ptr<RtcIceCandidate> FromWebrtc(const webrtc::IceCandidateInterface & wcand);
    private:
        std::string sdp_mid_;
        int sdp_mline_index_;
        std::string candidate_;
    };
}
}
