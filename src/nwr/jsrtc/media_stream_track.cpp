//
//  media_stream_track.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/16.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "media_stream_track.h"
#include "rtc_peer_connection_factory.h"

namespace nwr {
namespace jsrtc {
    MediaStreamTrack::MediaStreamTrack(webrtc::MediaStreamTrackInterface & inner_track):
    inner_track_(&inner_track)
    {
        inner_observer_ = std::make_shared<ChangeObserver>(*this);
        inner_track_->RegisterObserver(inner_observer_.get());
        
        id_ = GetRandomString(20);
        
        enabled_ = ComputeEnabled();
        ready_state_ = ComputeState();
    }
    
    webrtc::MediaStreamTrackInterface & MediaStreamTrack::inner_track() {
        return *inner_track_.get();
    }
    
    webrtc::AudioTrackInterface * MediaStreamTrack::inner_audio_track() {
        if (kind() == webrtc::MediaStreamTrackInterface::kAudioKind) {
            return static_cast<webrtc::AudioTrackInterface *>(inner_track_.get());
        }
        return nullptr;
    }
    webrtc::VideoTrackInterface * MediaStreamTrack::inner_video_track() {
        if (kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
            return static_cast<webrtc::VideoTrackInterface *>(inner_track_.get());
        }
        return nullptr;
    }
    
    std::string MediaStreamTrack::kind() {
        return inner_track_->kind();
    }

    std::string MediaStreamTrack::id() {
        return id_;
    }
    
    std::string MediaStreamTrack::label() {
        return inner_track_->id();
    }
    
    bool MediaStreamTrack::enabled() {
        return enabled_;
    }
    
    void MediaStreamTrack::set_enabled(bool value) {
        inner_track_->set_enabled(value);
    }
    
    bool MediaStreamTrack::remote() {
        return inner_source()->remote();
    }
    
    MediaStreamTrackState MediaStreamTrack::ready_state() {
        return ready_state_;
    }
    
    void MediaStreamTrack::set_on_ended(const std::function<void()> & value) {
        on_ended_ = value;
    }
    
    std::shared_ptr<MediaStreamTrack> MediaStreamTrack::Clone(const std::shared_ptr<RtcPeerConnectionFactory> & factory) {
        std::shared_ptr<MediaStreamTrack> track = nullptr;
        if (!track) {
            auto inner_audio_track = this->inner_audio_track();
            if (inner_audio_track) {
                track = factory->CreateAudioTrack(label(), inner_audio_track->GetSource());
            }
        }
        if (!track) {
            auto inner_video_track = this->inner_video_track();
            if (inner_video_track) {
                track = factory->CreateVideoTrack(label(), inner_video_track->GetSource());
            }
        }
        track->set_enabled(enabled());
        track->inner_track().set_state(inner_track().state());
        return track;
    }
    
    void MediaStreamTrack::Stop() {
        if (remote()) { return; }
        inner_track_->set_state(webrtc::MediaStreamTrackInterface::kEnded);
    }

    webrtc::MediaSourceInterface * MediaStreamTrack::inner_source() {
        auto audio_source = inner_audio_source();
        if (audio_source) { return audio_source; }
        auto video_source = inner_video_source();
        if (video_source) { return video_source; }
        Fatal("invalid state");
    }
    webrtc::AudioSourceInterface * MediaStreamTrack::inner_audio_source() {
        auto audio_track = inner_audio_track();
        if (audio_track) {
            return audio_track->GetSource();
        }
        return nullptr;
    }
    webrtc::VideoSourceInterface * MediaStreamTrack::inner_video_source() {
        auto video_track = inner_video_track();
        if (video_track) {
            return video_track->GetSource();
        }
        return nullptr;
    }

    
    MediaStreamTrack::ChangeObserver::
    ChangeObserver(MediaStreamTrack & owner):
    owner(owner)
    {}
    
    void MediaStreamTrack::ChangeObserver::OnChanged() {
        
    }
    
    void MediaStreamTrack::OnInnerUpdate() {
        auto new_enabled = ComputeEnabled();
        if (enabled_ != new_enabled) {
            enabled_ = new_enabled;
        }
        
        auto new_state = ComputeState();
        if (ready_state_ != new_state) {
            ready_state_ = new_state;
            if (new_state == MediaStreamTrackState::Ended) {
                FuncCall(on_ended_);
            }
        }
    }
    
    bool MediaStreamTrack::ComputeEnabled() {
        return inner_track_->enabled();
    }
    
    MediaStreamTrackState MediaStreamTrack::ComputeState() {
        switch (inner_track_->state()) {
            case webrtc::MediaStreamTrackInterface::kInitializing:
            case webrtc::MediaStreamTrackInterface::kLive:
                return MediaStreamTrackState::Live;
            case webrtc::MediaStreamTrackInterface::kEnded:
            case webrtc::MediaStreamTrackInterface::kFailed:
                return MediaStreamTrackState::Ended;
        }
    }
    
}
}
