//
//  rtc_peer_connection_factory.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/15.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "rtc_peer_connection_factory.h"

#include "media_track_constraints.h"
#include "media_stream_constraints.h"
#include "rtc_peer_connection.h"
#include "media_stream_track.h"
#include "media_stream.h"

namespace nwr {
namespace jsrtc {
    RtcPeerConnectionFactory::RtcPeerConnectionFactory() {
        signaling_thread_ = rtc::scoped_ptr<rtc::Thread>(new rtc::Thread());
        bool ret = signaling_thread_->Start();
        if (!ret) { Fatal("signaling thread start failed"); }
        
        worker_thread_ = rtc::scoped_ptr<rtc::Thread>(new rtc::Thread());
        ret = worker_thread_->Start();
        if (!ret) { Fatal("worker thread start failed"); }
        
        inner_factory_ = webrtc::CreatePeerConnectionFactory(worker_thread_.get(),
                                                             signaling_thread_.get(),
                                                             nullptr,
                                                             nullptr,
                                                             nullptr);
    }
    
    RtcPeerConnectionFactory::~RtcPeerConnectionFactory() {
        Close();
    }
    
    std::shared_ptr<RtcPeerConnection> RtcPeerConnectionFactory::
    CreatePeerConnection(const webrtc::PeerConnectionInterface::RTCConfiguration & configuration,
                         const std::shared_ptr<MediaTrackConstraints> & constraints)
    {
        auto connection = std::shared_ptr<RtcPeerConnection>(new RtcPeerConnection());
        auto inner_observer = std::make_shared<RtcPeerConnection::InnerObserver>(connection);
        
        auto inner_connection = inner_factory_->CreatePeerConnection(configuration,
                                                                     constraints ? &constraints->inner_constraints() : nullptr,
                                                                     nullptr,
                                                                     nullptr,
                                                                     inner_observer.get());
        connection->Init(*inner_connection, inner_observer);
        return connection;
    }
    
    std::shared_ptr<MediaStream> RtcPeerConnectionFactory::
    CreateMediaStream(const std::string & label) {
        auto inner_stream = inner_factory_->CreateLocalMediaStream(label);
        return std::make_shared<MediaStream>(*inner_stream);
    }
    
    rtc::scoped_refptr<webrtc::AudioSourceInterface> RtcPeerConnectionFactory::
    CreateAudioSource(const std::shared_ptr<MediaTrackConstraints> & constraints)
    {
        return inner_factory_->CreateAudioSource(constraints ? &constraints->inner_constraints() : nullptr);
    }
    
    rtc::scoped_refptr<webrtc::VideoSourceInterface> RtcPeerConnectionFactory::
    CreateVideoSource(cricket::VideoCapturer* capturer,
                      const std::shared_ptr<MediaTrackConstraints> & constraints)
    {
        return inner_factory_->CreateVideoSource(capturer,
                                                 constraints ? &constraints->inner_constraints() : nullptr);
    }
    
    std::shared_ptr<MediaStreamTrack> RtcPeerConnectionFactory::
    CreateAudioTrack(const std::string& label,
                     webrtc::AudioSourceInterface* source)
    {
        auto inner_track = inner_factory_->CreateAudioTrack(label, source);
        return std::make_shared<MediaStreamTrack>(*inner_track);
    }
    
    std::shared_ptr<MediaStreamTrack> RtcPeerConnectionFactory::
    CreateVideoTrack(const std::string& label,
                     webrtc::VideoSourceInterface * source)
    {
        auto inner_track = inner_factory_->CreateVideoTrack(label, source);
        return std::make_shared<MediaStreamTrack>(*inner_track);
    }
    
    void RtcPeerConnectionFactory::
    GetUserMedia(const MediaStreamConstraints & constraints,
                 const std::function<void(const std::shared_ptr<MediaStream> &)> & success,
                 const std::function<void(const std::string &)> & failure)
    {
        auto stream = CreateMediaStream("LocalStream");
        
#if !TARGET_IPHONE_SIMULATOR && TARGET_OS_IPHONE
        if (constraints.video()) {
            webrtc::AVFoundationVideoCapturer * capturer = new webrtc::AVFoundationVideoCapturer();
            auto video_source = CreateVideoSource(capturer, constraints.video());
            auto video_track = CreateVideoTrack("LocalVideo", video_source);
            stream->AddTrack(video_track);
        }
        
        if (constraints.audio()) {
            auto audio_source = CreateAudioSource(constraints.audio());
            auto audio_track = CreateAudioTrack("LocalAudio", audio_source);
            stream->AddTrack(audio_track);
        }
#endif
        TaskQueue::system_current_queue()->PostTask([stream, success, failure](){
            if (stream) {
                success(stream);
            }
        });
    }
    
    void RtcPeerConnectionFactory::OnClose() {
        inner_factory_ = nullptr;
        worker_thread_->Stop();
        worker_thread_ = nullptr;
        signaling_thread_->Stop();
        signaling_thread_ = nullptr;
    }
}
}