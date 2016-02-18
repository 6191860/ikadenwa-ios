//
//  rtc_peer_connection.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/16.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <memory>
#include <functional>
#include <nwr/base/env.h>
#include <nwr/base/array.h>
#include <nwr/base/func.h>
#include <nwr/base/task_queue.h>
#include "lib_webrtc.h"
#include "post_target.h"

namespace nwr {
    namespace jsrtc {
        class RtcSessionDescription;
        class RtcIceCandidate;
        class RtcPeerConnectionFactory;
        class RtcDataChannel;
        class MediaStream;
        
        class RtcPeerConnection : public PostTarget<RtcPeerConnection> {
            friend RtcPeerConnectionFactory;
        public:
            void CreateOffer(const webrtc::MediaConstraintsInterface * options,
                             const std::function<void(const std::shared_ptr<RtcSessionDescription> &)> & success,
                             const std::function<void(const std::string &)> & failure);
            void CreateAnswer(const webrtc::MediaConstraintsInterface * options,
                              const std::function<void(const std::shared_ptr<RtcSessionDescription> &)> & success,
                              const std::function<void(const std::string &)> & failure);
            void SetLocalDescription(const std::shared_ptr<const RtcSessionDescription> & description,
                                     const std::function<void()> & success,
                                     const std::function<void(const std::string &)> & failure);
            std::shared_ptr<const RtcSessionDescription> local_description();
            std::shared_ptr<const RtcSessionDescription> current_local_description();
            std::shared_ptr<const RtcSessionDescription> pending_local_description();
            void SetRemoteDescription(const std::shared_ptr<const RtcSessionDescription> & description,
                                      const std::function<void()> & success,
                                      const std::function<void(const std::string &)> & failure);
            std::shared_ptr<const RtcSessionDescription> remote_description();
            std::shared_ptr<const RtcSessionDescription> current_remote_description();
            std::shared_ptr<const RtcSessionDescription> pending_remote_description();
#warning todo callback (issue)
            void AddIceCandidate(const std::shared_ptr<const RtcIceCandidate> & candidate);
            webrtc::PeerConnectionInterface::SignalingState signaling_state();
            webrtc::PeerConnectionInterface::IceGatheringState ice_gathering_state();
            webrtc::PeerConnectionInterface::IceConnectionState ice_connection_state();
//            readonly        attribute RTCPeerConnectionState    connectionState;
//            readonly        attribute boolean?                  canTrickleIceCandidates;
//            static readonly attribute FrozenArray<RTCIceServer> defaultIceServers;
//            RTCConfiguration               getConfiguration ();
//            void                           setConfiguration (RTCConfiguration configuration);            
            void Close();
            void set_on_negotiation_needed(const std::function<void()> & value);
            void set_on_ice_candidate(const std::function<void(const std::shared_ptr<RtcIceCandidate> &)> & value);
            void set_on_signaling_state_change(const std::function<void(webrtc::PeerConnectionInterface::SignalingState)> & value);
            void set_on_ice_connection_state_change(const std::function<void(webrtc::PeerConnectionInterface::IceConnectionState)> & value);
            void set_on_ice_gathering_state_change(const std::function<void(webrtc::PeerConnectionInterface::IceGatheringState)> & value);
            
            std::vector<rtc::scoped_refptr<webrtc::RtpSenderInterface>> senders();
            std::vector<rtc::scoped_refptr<webrtc::RtpReceiverInterface>> receivers();
            //sequence<RTCRtpTransceiver> getTransceivers ();
            rtc::scoped_refptr<webrtc::RtpSenderInterface> AddTrack(webrtc::MediaStreamTrackInterface * track,
                                                                    std::vector<webrtc::MediaStreamInterface*> streams);
            bool RemoveTrack(webrtc::RtpSenderInterface* sender);
//            RTCRtpTransceiver           addTransceiver ((MediaStreamTrack or DOMString) trackOrKind, RTCRtpTransceiverInit init);
//            attribute EventHandler ontrack;
            
//            readonly        attribute RTCSctpTransport? sctp;
            std::shared_ptr<RtcDataChannel> CreateDataChannel(const std::string & label,
                                                              const webrtc::DataChannelInit * config);
            void set_on_data_channel(const std::function<void(const std::shared_ptr<RtcDataChannel> &)> & value);
            
#warning onconnection
            
            std::vector<std::shared_ptr<MediaStream>> local_streams();
            std::vector<std::shared_ptr<MediaStream>> remote_streams();
            void AddStream(const std::shared_ptr<MediaStream> & stream);
            void RemoveStream(const std::shared_ptr<MediaStream> & stream);
            void set_on_add_stream(const std::function<void(const std::shared_ptr<MediaStream> &)> & value);
            void set_on_remove_stream(const std::function<void(const std::shared_ptr<MediaStream> &)> & value);
        private:
            struct InnerObserver:
            public webrtc::PeerConnectionObserver
            {
                InnerObserver(const std::shared_ptr<RtcPeerConnection> & owner);
                
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
                
                std::shared_ptr<RtcPeerConnection> owner;
            };
            
            struct CreateSessionDescriptionObserver:
            public rtc::RefCountedObject<webrtc::CreateSessionDescriptionObserver>
            {
                CreateSessionDescriptionObserver(const std::shared_ptr<RtcPeerConnection> & owner,
                                                 const std::function<void(const std::shared_ptr<RtcSessionDescription> &)> & success,
                                                 const std::function<void(const std::string &)> & failure);
                
                void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;
                void OnFailure(const std::string& error) override;
                
                std::shared_ptr<RtcPeerConnection> owner;
                std::function<void(const std::shared_ptr<RtcSessionDescription> &)> success;
                std::function<void(const std::string &)> failure;
            };
            
            struct SetSessionDescriptionObserver:
            public rtc::RefCountedObject<webrtc::SetSessionDescriptionObserver>
            {
                SetSessionDescriptionObserver(const std::shared_ptr<RtcPeerConnection> & owner,
                                              const std::function<void()> & success,
                                              const std::function<void(const std::string &)> & failure);
                
                void OnSuccess() override;
                void OnFailure(const std::string& error) override;
                
                std::shared_ptr<RtcPeerConnection> owner;
                std::function<void()> success;
                std::function<void(const std::string &)> failure;
            };
            
            RtcPeerConnection();
            void Init(webrtc::PeerConnectionInterface & inner_connection,
                      const std::shared_ptr<InnerObserver> & inner_observer);
            
            std::shared_ptr<MediaStream> FindRemoteStreamByInnerStream(webrtc::MediaStreamInterface * inner_stream);
            
            bool closed_;
            rtc::scoped_refptr<webrtc::PeerConnectionInterface> inner_connection_;
            std::shared_ptr<InnerObserver> inner_observer_;
            
            std::shared_ptr<const RtcSessionDescription> current_local_description_;
            std::shared_ptr<const RtcSessionDescription> pending_local_description_;
            std::shared_ptr<const RtcSessionDescription> current_remote_description_;
            std::shared_ptr<const RtcSessionDescription> pending_remote_description_;
            std::function<void()> on_negotiation_needed_;
            std::function<void(const std::shared_ptr<RtcIceCandidate> &)> on_ice_candidate_;
//            std::function<void()> on_ice_candidate_error_;
            std::function<void(webrtc::PeerConnectionInterface::SignalingState)> on_signaling_state_change_;
            std::function<void(webrtc::PeerConnectionInterface::IceConnectionState)> on_ice_connection_state_change_;
            std::function<void(webrtc::PeerConnectionInterface::IceGatheringState)> on_ice_gathering_state_change_;
//            std::function<void()> on_connection_state_change_;
            
            std::function<void(const std::shared_ptr<RtcDataChannel> &)> on_data_channel_;
            
            std::vector<std::shared_ptr<MediaStream>> local_streams_;
            std::vector<std::shared_ptr<MediaStream>> remote_streams_;
            std::function<void(const std::shared_ptr<MediaStream> &)> on_add_stream_;
            std::function<void(const std::shared_ptr<MediaStream> &)> on_remove_stream_;
        };
        
        
    }
}
