//
//  peer_conn.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/13.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <string>
#include <map>

#include <nwr/base/map.h>
#include <nwr/base/func.h>
#include <nwr/base/optional.h>
#include <nwr/base/lib_webrtc.h>

namespace nwr {
namespace ert {
    class Easyrtc;
    
    class PeerConn {
    public:
        PeerConn();
        
        bool started_av() const { return started_av_; }
        rtc::scoped_refptr<webrtc::DataChannel> data_channel_s() const { return data_channel_s_; }
        bool data_channel_ready() const { return data_channel_ready_; }
        bool sharing_audio() const { return sharing_audio_; }
        bool sharing_video() const { return sharing_video_; }
        bool sharing_data() const { return sharing_data_; }
        rtc::scoped_refptr<webrtc::PeerConnectionInterface> pc() const { return pc_; }
        std::map<std::string, std::string> remote_stream_id_to_name() const { return remote_stream_id_to_name_; }
        
        Func<void(const std::string &)> call_success_cb() const { return call_success_cb_; }
        void set_call_success_cb(const Func<void(const std::string &)> & value) { call_success_cb_ = value; }
        Func<void(const std::string &,
                  const std::string &)> call_failure_cb() const { return call_failure_cb_; }
        void set_call_failure_cb(const Func<void(const std::string &,
                                                 const std::string &)> & value) { call_failure_cb_ = value; }
        Func<void(bool, const std::string &)> was_accepted_cb() const {
            return was_accepted_cb_;
        }
        void set_was_accepted_cb(const Func<void(bool, const std::string &)> & value) {
            was_accepted_cb_ = value;
        }
        
        rtc::scoped_refptr<webrtc::MediaStreamInterface>
        GetRemoteStreamByName(Easyrtc & ert,
                              const Optional<std::string> & stream_name) const;
        
        
    private:
        //  emulate lambda capture
        std::string other_user_;
        
        bool started_av_;
        rtc::scoped_refptr<webrtc::DataChannel> data_channel_s_;
        //  data_channel_r
        bool data_channel_ready_;
        //  connect_time
        bool sharing_audio_;
        bool sharing_video_;
        bool sharing_data_;
        //  cancelled
        //  candidates_to_send
        //  streams_added_acks
        rtc::scoped_refptr<webrtc::PeerConnectionInterface> pc_;
        //  media_stream
        //  connection_accepted
        //  is_initiator
        std::map<std::string, std::string> remote_stream_id_to_name_;
        //  live_remote_streams
        
        Func<void(const std::string &)> call_success_cb_;
        Func<void(const std::string &,
                  const std::string &)> call_failure_cb_;
        Func<void(bool, const std::string &)> was_accepted_cb_;
        
//    {  startedAV: boolean,  -- true if we have traded audio/video streams
//        dataChannelS: RTPDataChannel for outgoing messages if present
//        dataChannelR: RTPDataChannel for incoming messages if present
//        dataChannelReady: true if the data channel can be used for sending yet
//        connectTime: timestamp when the connection was started
//        sharingAudio: true if audio is being shared
//        sharingVideo: true if video is being shared
//        cancelled: temporarily true if a connection was cancelled by the peer asking to initiate it
//        candidatesToSend: SDP candidates temporarily queued
//        streamsAddedAcks: ack callbacks waiting for stream received messages
//        pc: RTCPeerConnection
//        mediaStream: mediaStream
//        function callSuccessCB(string) - see the easyrtc.call documentation.
//        function callFailureCB(errorCode, string) - see the easyrtc.call documentation.
//        function wasAcceptedCB(boolean,string) - see the easyrtc.call documentation.
//     }
        
//    pc: pc,
//    candidatesToSend: [],
//    startedAV: false,
//    connectionAccepted: false,
//    isInitiator: isInitiator,
//    remoteStreamIdToName: {},
//    streamsAddedAcks: {},
//    liveRemoteStreams: {},

    };
}
}