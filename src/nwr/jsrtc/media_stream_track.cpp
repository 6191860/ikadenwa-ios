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
    inner_track_(&inner_track),
    change_emitter_(std::make_shared<decltype(change_emitter_)::element_type>())
    {
        inner_observer_ = std::make_shared<ChangeObserver>(*this);
        inner_track_->RegisterObserver(inner_observer_.get());
        
        id_ = GetRandomString(20);
        
        enabled_ = inner_track_->enabled();
        ready_state_ = ComputeState(inner_track_->state());
    }
    
    MediaStreamTrack::~MediaStreamTrack() {
        inner_track_->UnregisterObserver(inner_observer_.get());
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
        inner_set_enabled(value);
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
        auto inner_track = inner_audio_track();
        if (inner_track) {
            return inner_track->GetSource();
        }
        return nullptr;
    }
    webrtc::VideoSourceInterface * MediaStreamTrack::inner_video_source() {
        auto inner_track = inner_video_track();
        if (inner_track) {
            return inner_track->GetSource();
        }
        return nullptr;
    }
    
    EmitterPtr<None> MediaStreamTrack::change_emitter() const {
        return change_emitter_;
    }
    
    void MediaStreamTrack::AddVideoRenderer(webrtc::VideoRendererInterface & renderer) {
        auto inner_track = inner_video_track();
        if (!inner_track) { Fatal("not video track"); }
        inner_track->AddRenderer(&renderer);
    }
    void MediaStreamTrack::RemoveVideoRenderer(webrtc::VideoRendererInterface & renderer) {
        auto inner_track = inner_video_track();
        if (!inner_track) { Fatal("not video track"); }
        inner_track->RemoveRenderer(&renderer);
    }

    void MediaStreamTrack::Close() {
        if (closed_) { return; }
        
        Stop();
        inner_track_ = nullptr;
        inner_observer_ = nullptr;
        
        id_.clear();
        enabled_ = false;
        ready_state_ = MediaStreamTrackState::Ended;
        on_ended_ = nullptr;
        change_emitter_->RemoveAllListeners();
        
        ClosePostTarget();
        
        closed_ = true;
    }
    
    MediaStreamTrack::ChangeObserver::
    ChangeObserver(MediaStreamTrack & owner):
    owner(owner)
    {}
    
    void MediaStreamTrack::ChangeObserver::OnChanged() {
        bool enabled = owner.inner_track_->enabled();
        MediaStreamTrackState state = owner.ComputeState(owner.inner_track_->state());
        
        owner.Post([enabled, state](MediaStreamTrack & owner){
            owner.inner_set_enabled(enabled);
            owner.inner_set_ready_state(state);
        });
    }
    
    void MediaStreamTrack::inner_set_enabled(bool value) {
        if (enabled_ != value) {
            enabled_ = value;
            change_emitter_->Emit(None());
        }
    }
    
    void MediaStreamTrack::inner_set_ready_state(MediaStreamTrackState value) {
        if (ready_state_ != value) {
            ready_state_ = value;
            if (value == MediaStreamTrackState::Ended) {
                FuncCall(on_ended_);
            }
            change_emitter_->Emit(None());
        }
    }
    
    MediaStreamTrackState MediaStreamTrack::ComputeState(webrtc::MediaStreamTrackInterface::TrackState state) {
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
