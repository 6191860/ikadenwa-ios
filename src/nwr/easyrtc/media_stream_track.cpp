//
//  media_stream_track.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/13.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "media_stream_track.h"

namespace nwr {
namespace ert {
    MediaStreamTrackVector ToMediaStreamTrackVector(const webrtc::VideoTrackVector & vector) {
        return MediaStreamTrackVector(vector.begin(), vector.end());
    }
    MediaStreamTrackVector ToMediaStreamTrackVector(const webrtc::AudioTrackVector & vector) {
        return MediaStreamTrackVector(vector.begin(), vector.end());
    }
    
    MediaStreamTrack::MediaStreamTrack(const rtc::scoped_refptr<webrtc::AudioTrack> & track):
    audio_(track)
    {}
    
    MediaStreamTrack::MediaStreamTrack(const rtc::scoped_refptr<webrtc::VideoTrack> & track):
    video_(track)
    {}
    
    bool MediaStreamTrack::enabled() {
        if (audio_) { return audio_->webrtc::MediaStreamTrackInterface::enabled() ; }
    }
    void MediaStreamTrack::set_enabled(bool value) {
        
    }
}
}