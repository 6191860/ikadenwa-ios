//
//  media_stream_track.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/13.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <vector>
#include <nwr/base/lib_webrtc.h>

namespace nwr {
namespace ert {
    using MediaStreamTrackVector = std::vector<rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>>;
    
    MediaStreamTrackVector ToMediaStreamTrackVector(const webrtc::VideoTrackVector & vector);
    MediaStreamTrackVector ToMediaStreamTrackVector(const webrtc::AudioTrackVector & vector);
    
    class MediaStreamTrack {
    public:
        MediaStreamTrack(const rtc::scoped_refptr<webrtc::AudioTrack> & track);
        MediaStreamTrack(const rtc::scoped_refptr<webrtc::VideoTrack> & track);
        
        rtc::scoped_refptr<webrtc::AudioTrack> audio() { return audio_; }
        rtc::scoped_refptr<webrtc::VideoTrack> video() { return video_; }
        
        bool enabled();
        void set_enabled(bool value);
    private:
        rtc::scoped_refptr<webrtc::AudioTrack> audio_;
        rtc::scoped_refptr<webrtc::VideoTrack> video_;
    };
}
}
