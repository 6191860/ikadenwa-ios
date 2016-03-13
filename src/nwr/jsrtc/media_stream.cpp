//
//  media_stream.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/16.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "media_stream.h"

#include "media_stream_track.h"
#include "rtc_peer_connection_factory.h"

namespace nwr {
namespace jsrtc {
    
    std::shared_ptr<MediaStream> MediaStream::Create(const std::shared_ptr<TaskQueue> & queue,
                                                     webrtc::MediaStreamInterface & inner_stream,
                                                     bool remote)
    {
        auto thiz = std::shared_ptr<MediaStream>(new MediaStream(queue));
        thiz->Init(inner_stream, remote);
        return thiz;
    }
    
    MediaStream::~MediaStream() {
        Close();
    }
    
    webrtc::MediaStreamInterface &  MediaStream::inner_stream() {
        return *inner_stream_;
    }
    
    std::string MediaStream::id() {
        return id_;
    }
    
    std::string MediaStream::label() {
        return inner_stream_->label();
    }
    
    std::vector<std::shared_ptr<MediaStreamTrack>> MediaStream::audio_tracks() {
        return audio_tracks_;
    }
    
    std::vector<std::shared_ptr<MediaStreamTrack>> MediaStream::video_tracks() {
        return video_tracks_;
    }
    
    std::vector<std::shared_ptr<MediaStreamTrack>> MediaStream::tracks() {
        std::vector<std::shared_ptr<MediaStreamTrack>> ret;
        
        auto audio_tracks = this->audio_tracks();
        ret.insert(ret.end(), audio_tracks.begin(), audio_tracks.end());
        
        auto video_tracks = this->video_tracks();
        ret.insert(ret.end(), video_tracks.begin(), video_tracks.end());
        
        return ret;
    }
    
    std::shared_ptr<MediaStreamTrack> MediaStream::GetTrackById(const std::string & track_id) {
        auto tracks = this->tracks();
        int index = IndexOfIf(tracks, [track_id](const std::shared_ptr<MediaStreamTrack> & x) {
            return x->id() == track_id;
        });
        if (index == -1) { return nullptr; }
        return tracks[index];
    }
    
    void MediaStream::AddTrack(const std::shared_ptr<MediaStreamTrack> & track) {
        auto inner_audio_track = track->inner_audio_track();
        if (inner_audio_track) {
            inner_stream_->AddTrack(inner_audio_track);
            AddTrackTo(track, audio_tracks_);
            return;
        }
        
        auto inner_video_track = track->inner_video_track();
        if (inner_video_track) {
            inner_stream_->AddTrack(inner_video_track);
            AddTrackTo(track, video_tracks_);
            return;
        }
    }
    void MediaStream::RemoveTrack(const std::shared_ptr<MediaStreamTrack> & track) {
        auto inner_audio_track = track->inner_audio_track();
        if (inner_audio_track) {
            bool ok = inner_stream_->RemoveTrack(inner_audio_track);
            if (!ok) { Fatal("RemoveTrack failed"); }
            RemoveTrackFrom(track, audio_tracks_);
            return;
        }
        
        auto inner_video_track = track->inner_video_track();
        if (inner_video_track) {
            bool ok = inner_stream_->RemoveTrack(inner_video_track);
            if (!ok) { Fatal("RemoveTrack failed"); }
            RemoveTrackFrom(track, video_tracks_);
            return;
        }
    }
    
    std::shared_ptr<MediaStream> MediaStream::Clone(const std::shared_ptr<RtcPeerConnectionFactory> & factory) {
        auto cloned_stream = factory->CreateMediaStream(label());
        
        for (const auto & track : tracks()) {
            auto cloned_track = track->Clone(factory);
            cloned_stream->AddTrack(cloned_track);
        }

        return cloned_stream;
    }
    
    bool MediaStream::active() {
        return active_;
    }
    
    void MediaStream::set_on_active(const std::function<void()> & value) {
        on_active_ = value;
    }
    void MediaStream::set_on_inactive(const std::function<void()> & value) {
        on_inactive_ = value;
    }
    
    void MediaStream::OnClose() {
        for (const auto & track : tracks()) {
            RemoveTrack(track);
        }
        
        inner_stream_ = nullptr;
        id_.clear();
        audio_tracks_.clear();
        video_tracks_.clear();
        active_ = false;
        on_active_ = nullptr;
        on_inactive_ = nullptr;
        track_change_listener_ = nullptr;
    }
    
    MediaStream::MediaStream(const std::shared_ptr<TaskQueue> & queue):
    PostTarget<nwr::jsrtc::MediaStream>(queue)
    {
    }
    
    void MediaStream::Init(webrtc::MediaStreamInterface & inner_stream, bool remote) {
        inner_stream_ = rtc::scoped_refptr<webrtc::MediaStreamInterface>(&inner_stream);
        active_ = false;
        
        id_ = GetRandomString(20);
        
        track_change_listener_ = FuncMake([this](const None & _){
            this->OnTracksUpdate();
        });
        
        for (const auto & inner_track : inner_stream.GetAudioTracks()) {
            auto track = MediaStreamTrack::Create(queue(), *inner_track, remote);
            audio_tracks_.push_back(track);
            SubscribeTrackChange(track);
        }
        for (const auto & inner_track : inner_stream.GetVideoTracks()) {
            auto track = MediaStreamTrack::Create(queue(), *inner_track, remote);
            video_tracks_.push_back(track);
            SubscribeTrackChange(track);
        }
        
        active_ = ComputeActive();
    }
    
    void MediaStream::OnTracksUpdate() {
        bool new_active = ComputeActive();
        if (active_ != new_active) {
            active_ = new_active;
            if (new_active) {
                FuncCall(on_active_);
            } else {
                FuncCall(on_inactive_);
            }
        }
    }
    
    void MediaStream::AddTrackTo(const std::shared_ptr<MediaStreamTrack> & track,
                                 std::vector<std::shared_ptr<MediaStreamTrack>> & tracks)
    {
        if (IndexOf(tracks, track) != -1) {
            Fatal("already added");
        }
        tracks.push_back(track);
        SubscribeTrackChange(track);
        OnTracksUpdate();
    }
    void MediaStream::RemoveTrackFrom(const std::shared_ptr<MediaStreamTrack> & track,
                                      std::vector<std::shared_ptr<MediaStreamTrack>> & tracks)
    {
        Remove(tracks, track);
        UnsubscribeTrackChange(track);
        OnTracksUpdate();
    }
    
    void MediaStream::SubscribeTrackChange(const std::shared_ptr<MediaStreamTrack> & track) {
        track->change_emitter()->On(track_change_listener_);
    }
    void MediaStream::UnsubscribeTrackChange(const std::shared_ptr<MediaStreamTrack> & track) {
        track->change_emitter()->Off(track_change_listener_);
    }
    
    bool MediaStream::ComputeActive() {
        for (const auto & track : tracks()) {
            if (track->ready_state() == MediaStreamTrackState::Live) {
                return true;
            }
        }
        return false;
    }
    
}
}