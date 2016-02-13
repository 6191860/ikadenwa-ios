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

#include <nwr/base/lib_webrtc.h>

namespace nwr {
namespace ert {
    struct PeerConn {
        PeerConn();
        
        rtc::scoped_refptr<webrtc::PeerConnectionInterface> pc;
        std::map<std::string, std::string> remote_stream_id_to_name;
        
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
//        getRemoteStreamByName
    };
}
}