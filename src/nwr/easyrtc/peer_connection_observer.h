//
//  peer_connection_func_observer.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/13.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <nwr/base/lib_webrtc.h>

namespace nwr {
namespace ert {
    class PeerConnectionObserver: public webrtc::PeerConnectionObserver {
    public:
        void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override;
        void OnStateChange(StateType state_changed) override;
        void OnAddStream(webrtc::MediaStreamInterface* stream) override;
        void OnRemoveStream(webrtc::MediaStreamInterface* stream) override;
        void OnDataChannel(webrtc::DataChannelInterface* data_channel) override;
        void OnRenegotiationNeeded() override;
        void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override;
        void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override;
        void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
        void OnIceComplete() override;
        void OnIceConnectionReceivingChange(bool receiving) override;
    };
}
}
