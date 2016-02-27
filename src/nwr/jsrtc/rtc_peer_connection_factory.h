//
//  rtc_peer_connection_factory.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/15.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <memory>
#include <nwr/base/env.h>
#include <nwr/base/task_queue.h>
#include <nwr/base/lib_webrtc.h>

#include "closable.h"

namespace nwr {
namespace jsrtc {
    class RtcPeerConnection;
    class MediaStream;
    class MediaStreamTrack;
    class MediaTrackConstraints;
    class MediaStreamConstraints;
        
    class RtcPeerConnectionFactory : public ClosableImpl {
    public:
        RtcPeerConnectionFactory();
        virtual ~RtcPeerConnectionFactory();

        std::shared_ptr<RtcPeerConnection>
        CreatePeerConnection(const webrtc::PeerConnectionInterface::RTCConfiguration & configuration,
                             const MediaTrackConstraints * constraints);
        std::shared_ptr<MediaStream>
        CreateMediaStream(const std::string & label);

        rtc::scoped_refptr<webrtc::AudioSourceInterface>
        CreateAudioSource(const MediaTrackConstraints * constraints);
        
        rtc::scoped_refptr<webrtc::VideoSourceInterface>
        CreateVideoSource(cricket::VideoCapturer* capturer,
                          const MediaTrackConstraints * constraints);
        
#warning todo; apply constraints
        std::shared_ptr<MediaStreamTrack>
        CreateAudioTrack(const std::string& label,
                         webrtc::AudioSourceInterface * source);
        std::shared_ptr<MediaStreamTrack>
        CreateVideoTrack(const std::string& label,
                         webrtc::VideoSourceInterface * source);
//        bool StartAecDump(rtc::PlatformFile file, int64_t max_size_bytes);
//        void StopAecDump();
//        bool StartRtcEventLog(rtc::PlatformFile file);
//        void StopRtcEventLog();

        void GetUserMedia(const MediaStreamConstraints & constraints,
                          const std::function<void(const std::shared_ptr<MediaStream> &)> & success,
                          const std::function<void(const std::string &)> & failure);
        
        void OnClose() override;
    private:
        rtc::scoped_ptr<rtc::Thread> signaling_thread_;
        rtc::scoped_ptr<rtc::Thread> worker_thread_;
        
        rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> inner_factory_;
    };
}
}
