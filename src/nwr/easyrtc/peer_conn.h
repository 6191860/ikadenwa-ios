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
#include <nwr/base/optional.h>
#include <nwr/base/lib_webrtc.h>

namespace nwr {
namespace ert {
    class Easyrtc;
    
    class PeerConn {
    public:
        PeerConn();
        
        rtc::scoped_refptr<webrtc::DataChannel> data_channel_s() const { return data_channel_s_; }
        bool data_channel_ready() const { return data_channel_ready_; }
        rtc::scoped_refptr<webrtc::PeerConnectionInterface> pc() const { return pc_; }
        std::map<std::string, std::string> remote_stream_id_to_name() const { return remote_stream_id_to_name_; }
        
        rtc::scoped_refptr<webrtc::MediaStreamInterface>
        GetRemoteStreamByName(Easyrtc & ert,
                              const Optional<std::string> & stream_name) const;
        
        
    private:
        //  emulate lambda capture
        std::string other_user_;
        
        //  startedAV
        rtc::scoped_refptr<webrtc::DataChannel> data_channel_s_;
        //  data_channel_r
        bool data_channel_ready_;
        //  connect_time
        //  sharing_audio
        //  sharing_video
        //  cancelled
        //  candidates_to_send
        //  streams_added_acks
        rtc::scoped_refptr<webrtc::PeerConnectionInterface> pc_;
        //  media_stream
        //  connection_accepted
        //  is_initiator
        std::map<std::string, std::string> remote_stream_id_to_name_;
        //  live_remote_streams
        
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