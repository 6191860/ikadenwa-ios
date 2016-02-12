//
//  peer_connection_func_observer.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/13.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <talk/app/webrtc/peerconnectioninterface.h>

namespace nwr {
namespace ert {
    class PeerConnectionObserver: public webrtc::PeerConnectionObserver {
    public:
        void OnSignalingChange(PeerConnectionInterface::SignalingState new_state) override;
        void OnStateChange(StateType state_changed) override;
        void OnAddStream(MediaStreamInterface* stream) override;
        void OnRemoveStream(MediaStreamInterface* stream) override;
        void OnDataChannel(DataChannelInterface* data_channel) override;
        void OnRenegotiationNeeded() override;
        void OnIceConnectionChange(PeerConnectionInterface::IceConnectionState new_state) override;
        void OnIceGatheringChange(PeerConnectionInterface::IceGatheringState new_state) override;
        void OnIceCandidate(const IceCandidateInterface* candidate) override;
        void OnIceComplete() override;
        void OnIceConnectionReceivingChange(bool receiving) override;
    };
}
}
