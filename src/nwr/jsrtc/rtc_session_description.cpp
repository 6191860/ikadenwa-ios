//
//  rtc_session_description.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/17.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "rtc_session_description.h"

namespace nwr {
namespace jsrtc {
    RtcSessionDescription::RtcSessionDescription(const std::string & type,
                                                 const std::string & sdp):
    type_(type),
    sdp_(sdp)
    {}
    
    std::string RtcSessionDescription::type() const {
        return type_;
    }
    void RtcSessionDescription::set_type(const std::string & value) {
        type_ = value;
    }
    std::string RtcSessionDescription::sdp() const {
        return sdp_;
    }
    void RtcSessionDescription::set_sdp(const std::string & value) {
        sdp_ = value;
    }

    
    Any RtcSessionDescription::ToAny() const {
        return Any(Any::ObjectType {
            { "type", Any(type_) },
            { "sdp", Any(sdp_) }
        });
    }
    
    std::shared_ptr<RtcSessionDescription> RtcSessionDescription::FromAny(const Any & any) {
        std::string type = any.GetAt("type").AsString() || std::string("");
        if (type == "offer" || type == "pranswer" || type == "answer" || type == "rollback") {
        } else {
            printf("invalid session description type: %s\n", type.c_str());
            return nullptr;
        }
        
        std::string sdp = any.GetAt("sdp").AsString() || std::string("");
        
        //  validate content
        auto * wdesc = CreateWebrtc(type, sdp);
        if (!wdesc) { return nullptr; }
        delete wdesc;
        
        return std::make_shared<RtcSessionDescription>(type, sdp);
    }
    
    webrtc::SessionDescriptionInterface * RtcSessionDescription::CreateWebrtc() const {
        return CreateWebrtc(type_, sdp_);
    }
    
    webrtc::SessionDescriptionInterface * CreateWebrtc(const std::string & type,
                                                       const std::string & sdp)
    {
        webrtc::SdpParseError err;
        auto * wdesc = webrtc::CreateSessionDescription(type, sdp, &err);
        if (!wdesc) {
            printf("invalid session description sdp: line %s, %s; sdp=%s\n",
                   err.line.c_str(), err.description.c_str(), sdp.c_str());
            return nullptr;
        }
        return wdesc;
    }
    
    std::shared_ptr<RtcSessionDescription> RtcSessionDescription::
    FromWebrtc(const webrtc::SessionDescriptionInterface & wdesc)
    {
        std::string sdp;
        wdesc.ToString(&sdp);
        return std::make_shared<RtcSessionDescription>(wdesc.type(), sdp);
    }
 
}
}
