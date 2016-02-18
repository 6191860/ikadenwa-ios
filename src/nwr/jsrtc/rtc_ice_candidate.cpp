//
//  rtc_ice_candidate.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/19.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "rtc_ice_candidate.h"

namespace nwr {
namespace jsrtc {
    
    RtcIceCandidate::RtcIceCandidate(const std::string & sdp_mid,
                                     int sdp_mline_index,
                                     const std::string & candidate):
    sdp_mid_(sdp_mid),
    sdp_mline_index_(sdp_mline_index),
    candidate_(candidate)
    {}
    
    std::string RtcIceCandidate::sdp_mid() const {
        return sdp_mid_;
    }
    void RtcIceCandidate::set_sdp_mid(const std::string & value) {
        sdp_mid_ = value;
    }
    int RtcIceCandidate::sdp_mline_index() const {
        return sdp_mline_index_;
    }
    void RtcIceCandidate::set_sdp_mline_index(int value) {
        sdp_mline_index_ = value;
    }
    std::string RtcIceCandidate::candidate() const {
        return candidate_;
    }
    void RtcIceCandidate::set_candidate(const std::string & value) {
        candidate_ = value;
    }
    
    Any RtcIceCandidate::ToAny() const {
        return Any(Any::ObjectType {
            { "sdpMid", Any(sdp_mid_) },
            { "sdpMLineIndex", Any(sdp_mline_index_) },
            { "candidate", Any(candidate_) },
        });
    }
    std::shared_ptr<RtcIceCandidate> RtcIceCandidate::FromAny(const Any & any) {
        std::string sdp_mid = any.GetAt("sdpMid").AsString() || std::string();
        int sdp_mline_index = any.GetAt("sdpMLineIndex").AsInt() || 0;
        std::string candidate = any.GetAt("candidate").AsString() || std::string();
        
        auto * wcand = CreateWebrtc(sdp_mid, sdp_mline_index, candidate);
        if (!wcand) { return nullptr; }
        delete wcand;
        
        return std::make_shared<RtcIceCandidate>(sdp_mid, sdp_mline_index, candidate);
    }
    
    webrtc::IceCandidateInterface * RtcIceCandidate::CreateWebrtc() const {
        return CreateWebrtc(sdp_mid_, sdp_mline_index_, candidate_);
    }
    webrtc::IceCandidateInterface * RtcIceCandidate::CreateWebrtc(const std::string & sdp_mid,
                                                                  int sdp_mline_index,
                                                                  const std::string & candidate)
    {
        webrtc::SdpParseError err;
        auto * wcand = webrtc::CreateIceCandidate(sdp_mid, sdp_mline_index, candidate, &err);
        if (!wcand) {
            printf("invalid session description sdp: line %s, %s; candidate=%s\n",
                   err.line.c_str(), err.description.c_str(), candidate.c_str());
        }
        return wcand;        
    }
    std::shared_ptr<RtcIceCandidate> RtcIceCandidate::FromWebrtc(const webrtc::IceCandidateInterface & wcand) {
        std::string candidate;
        wcand.ToString(&candidate);
        return std::make_shared<RtcIceCandidate>(wcand.sdp_mid(), wcand.sdp_mline_index(), candidate);
    }
    
}
}
