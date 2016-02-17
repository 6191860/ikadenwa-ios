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
#include "lib_webrtc.h"

namespace nwr {
namespace jsrtc {
    class RtcPeerConnection;
    class MediaStream;
    class MediaStreamTrack;
        
    class RtcPeerConnectionFactory {
    public:
        RtcPeerConnectionFactory();
        
        std::shared_ptr<RtcPeerConnection>
        CreatePeerConnection(const webrtc::PeerConnectionInterface::RTCConfiguration & configuration,
                             const webrtc::MediaConstraintsInterface & constraints);
        std::shared_ptr<MediaStream>
        CreateMediaStream(const std::string & label);
//        std::shared_ptr<MediaStream>
//        CreateLocalMediaStraem(const std::string & label);
//        rtc::scoped_refptr<webrtc::AudioSourceInterface>
//        CreateAudioSource(const MediaConstraintsInterface* constraints);
//        rtc::scoped_refptr<webrtc::VideoSourceInterface>
//        CreateVideoSource(cricket::VideoCapturer* capturer,
//                          const MediaConstraintsInterface* constraints);
        
        std::shared_ptr<MediaStreamTrack>
        CreateAudioTrack(const std::string& label,
                         webrtc::AudioSourceInterface* source);
        std::shared_ptr<MediaStreamTrack>
        CreateVideoTrack(const std::string& label,
                         webrtc::VideoSourceInterface * source);
//        bool StartAecDump(rtc::PlatformFile file, int64_t max_size_bytes);
//        void StopAecDump();
//        bool StartRtcEventLog(rtc::PlatformFile file);
//        void StopRtcEventLog();

    private:
        rtc::scoped_ptr<rtc::Thread> signaling_thread_;
        rtc::scoped_ptr<rtc::Thread> worker_thread_;
        
        rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> inner_factory_;
    };
}
}
