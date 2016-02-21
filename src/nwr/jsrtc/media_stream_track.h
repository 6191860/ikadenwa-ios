//
//  media_stream_track.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/16.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <string>
#include <nwr/base/env.h>
#include <nwr/base/string.h>
#include <nwr/base/func.h>
#include <nwr/base/none.h>
#include <nwr/base/emitter.h>
#include <nwr/base/lib_webrtc.h>
#include "post_target.h"

namespace nwr {
namespace jsrtc {
    class MediaStream;
    class RtcPeerConnectionFactory;
    
    enum class MediaStreamTrackState {
        Live,
        Ended
    };
    
    class MediaStreamTrack : public PostTarget<MediaStreamTrack> {
    public:
        MediaStreamTrack(webrtc::MediaStreamTrackInterface & inner_track);
        virtual ~MediaStreamTrack();
        
        webrtc::MediaStreamTrackInterface & inner_track();
        webrtc::AudioTrackInterface * inner_audio_track();
        webrtc::VideoTrackInterface * inner_video_track();
        
        std::string kind();
        std::string id();
        std::string label();
        bool enabled();
        void set_enabled(bool value);
//        readonly    attribute boolean               muted;
//        attribute EventHandler          onmute;
//        attribute EventHandler          onunmute;
//        readonly    attribute boolean               _readonly;
        bool remote();
        MediaStreamTrackState ready_state();
        void set_on_ended(const std::function<void()> & value);
        std::shared_ptr<MediaStreamTrack> Clone(const std::shared_ptr<RtcPeerConnectionFactory> & factory);
        void Stop();
//        MediaTrackCapabilities getCapabilities ();
//        MediaTrackConstraints  getConstraints ();
//        MediaTrackSettings     getSettings ();
//        Promise<void>          applyConstraints (MediaTrackConstraints constraints);
//        attribute EventHandler          onoverconstrained;

        webrtc::MediaSourceInterface * inner_source();
        webrtc::AudioSourceInterface * inner_audio_source();
        webrtc::VideoSourceInterface * inner_video_source();
        
        EmitterPtr<None> change_emitter() const { return change_emitter_; }
        
        void Close();
    private:
        struct ChangeObserver: public webrtc::ObserverInterface {
            ChangeObserver(MediaStreamTrack & owner);
            
            void OnChanged() override;
            
            MediaStreamTrack & owner;
        };
        
        void inner_set_enabled(bool value);
        void inner_set_ready_state(MediaStreamTrackState value);
        void OnInnerUpdate();
        MediaStreamTrackState ComputeState(webrtc::MediaStreamTrackInterface::TrackState state);
        
        bool closed_;
        
        rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> inner_track_;
        std::shared_ptr<ChangeObserver> inner_observer_;
        
        std::string id_;
        bool enabled_;
        MediaStreamTrackState ready_state_;
        std::function<void()> on_ended_;
        
        EmitterPtr<None> change_emitter_;
    };
}
}
