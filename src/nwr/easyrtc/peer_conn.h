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
#include <chrono>

#include <nwr/base/map.h>
#include <nwr/base/func.h>
#include <nwr/base/any.h>
#include <nwr/base/optional.h>
#include <nwr/jsrtc/rtc_peer_connection.h>

namespace nwr {
namespace ert {
    using namespace jsrtc;
    
    class Easyrtc;
    
    class PeerConn {
    public:
        PeerConn();
        void Close();
        
        bool started_av() const { return started_av_; }
        void set_started_av(bool value) { started_av_ = value; }
        rtc::scoped_refptr<webrtc::DataChannel> data_channel_s() const { return data_channel_s_; }
        bool data_channel_ready() const { return data_channel_ready_; }
        Optional<std::chrono::system_clock::time_point> connect_time() const { return connect_time_; }
        void set_connect_time(const Optional<std::chrono::system_clock::time_point> & value) { connect_time_ = value; }
        bool sharing_audio() const { return sharing_audio_; }
        void set_sharing_audio(bool value) { sharing_audio_ = value; }
        bool sharing_video() const { return sharing_video_; }
        void set_sharing_video(bool value) { sharing_video_ = value; }
        bool sharing_data() const { return sharing_data_; }
        void set_sharing_data(bool value) { sharing_data_ = value; }
        bool canceled() const { return canceled_; }
        void set_canceled(bool value) { canceled_ = value; }
        std::vector<Any> & candidates_to_send() { return candidates_to_send_; }
        std::map<std::string, std::function<void()>> & streams_added_acks() { return streams_added_acks_; }
        std::shared_ptr<RtcPeerConnection> pc() const { return pc_; }
        void set_pc(const std::shared_ptr<RtcPeerConnection> & value) { pc_ = value; }
        bool connection_accepted() const { return connection_accepted_; }
        void set_connection_accepted(bool value) { connection_accepted_ = value; }
        bool is_initiator() const { return is_initiator_; }
        void set_is_initiator(bool value) { is_initiator_ = value; }
        std::map<std::string, std::string> & remote_stream_id_to_name() { return remote_stream_id_to_name_; }
        std::map<std::string, bool> & live_remote_streams() { return live_remote_streams_; }
        bool enable_negotiate_listener() const { return enable_negotiate_listener_; }
        void set_enable_negotiate_listener(bool value) { enable_negotiate_listener_ = value; }
        std::function<void(const std::string &,
                           const std::string &)> call_success_cb() const { return call_success_cb_; }
        void set_call_success_cb(const std::function<void(const std::string &,
                                                          const std::string &)> & value) { call_success_cb_ = value; }
        std::function<void(const std::string &,
                           const std::string &)> call_failure_cb() const { return call_failure_cb_; }
        void set_call_failure_cb(const std::function<void(const std::string &,
                                                          const std::string &)> & value) { call_failure_cb_ = value; }
        std::function<void(bool, const std::string &)> was_accepted_cb() const {
            return was_accepted_cb_;
        }
        void set_was_accepted_cb(const std::function<void(bool, const std::string &)> & value) {
            was_accepted_cb_ = value;
        }
        Optional<std::chrono::system_clock::time_point> failing() { return failing_; }
        void set_failing(const Optional<std::chrono::system_clock::time_point> & value) { failing_ = value; }
        
        
        std::shared_ptr<MediaStream>
        GetRemoteStreamByName(Easyrtc & ert,
                              const Optional<std::string> & stream_name) const;
        
        
    private:
        //  emulate lambda capture
        std::string other_user_;
        
        bool started_av_;
        rtc::scoped_refptr<webrtc::DataChannel> data_channel_s_;
        //  data_channel_r
        bool data_channel_ready_;
        Optional<std::chrono::system_clock::time_point> connect_time_;
        bool sharing_audio_;
        bool sharing_video_;
        bool sharing_data_;
        bool canceled_;
        std::vector<Any> candidates_to_send_;
        std::map<std::string, std::function<void()>> streams_added_acks_;
        std::shared_ptr<RtcPeerConnection> pc_;
        //  media_stream
        bool connection_accepted_;
        bool is_initiator_;
        std::map<std::string, std::string> remote_stream_id_to_name_;
        std::map<std::string, bool> live_remote_streams_;
        bool enable_negotiate_listener_;
        
        std::function<void(const std::string &,
                           const std::string &)> call_success_cb_;
        std::function<void(const std::string &,
                           const std::string &)> call_failure_cb_;
        std::function<void(bool, const std::string &)> was_accepted_cb_;
        
        Optional<std::chrono::system_clock::time_point> failing_;
        
        
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