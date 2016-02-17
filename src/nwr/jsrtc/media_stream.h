//
//  media_stream.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/16.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <string>
#include <vector>
#include <nwr/base/env.h>
#include <nwr/base/string.h>
#include <nwr/base/optional.h>
#include <nwr/base/array.h>
#include <nwr/base/emitter.h>
#include <nwr/jsrtc/lib_webrtc.h>

#include "post_target.h"
#include "media_stream_track.h"

namespace nwr {
namespace jsrtc {
    class RtcPeerConnection;
    
    class MediaStream : public PostTarget<MediaStream> {
    public:
        MediaStream(webrtc::MediaStreamInterface & inner_stream);
        webrtc::MediaStreamInterface & inner_stream();

        std::string id();
        std::string label();
        std::vector<std::shared_ptr<MediaStreamTrack>> audio_tracks();
        std::vector<std::shared_ptr<MediaStreamTrack>> video_tracks();
        std::vector<std::shared_ptr<MediaStreamTrack>> tracks();
        std::shared_ptr<MediaStreamTrack> GetTrackById(const std::string & track_id);
        void AddTrack(const std::shared_ptr<MediaStreamTrack> & track);
        void RemoveTrack(const std::shared_ptr<MediaStreamTrack> & track);
        std::shared_ptr<MediaStream> Clone(const std::shared_ptr<RtcPeerConnectionFactory> & factory);
        bool active();
        void set_on_active(const std::function<void()> & value);
        void set_on_inactive(const std::function<void()> & value);
//        attribute EventHandler onaddtrack;
//        attribute EventHandler onremovetrack;
    private:
        void OnTracksUpdate();

        void AddTrackTo(const std::shared_ptr<MediaStreamTrack> & track,
                        std::vector<std::shared_ptr<MediaStreamTrack>> & tracks);
        void RemoveTrackFrom(const std::shared_ptr<MediaStreamTrack> & track,
                             std::vector<std::shared_ptr<MediaStreamTrack>> & tracks);
        
        void SubscribeTrackChange(const std::shared_ptr<MediaStreamTrack> & track);
        void UnsubscribeTrackChange(const std::shared_ptr<MediaStreamTrack> & track);
        
        void set_active(bool value);
        bool ComputeActive();
        
        rtc::scoped_refptr<webrtc::MediaStreamInterface> inner_stream_;
        std::string id_;
        std::vector<std::shared_ptr<MediaStreamTrack>> audio_tracks_;
        std::vector<std::shared_ptr<MediaStreamTrack>> video_tracks_;
        bool active_;
        std::function<void()> on_active_;
        std::function<void()> on_inactive_;
        Func<void(const None &)> track_change_listener_;
        
    };
}
}
