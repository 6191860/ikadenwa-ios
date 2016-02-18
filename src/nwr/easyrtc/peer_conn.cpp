//
//  peer_conn.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/13.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "peer_conn.h"

#include "easyrtc.h"

namespace nwr {
namespace ert {
    PeerConn::PeerConn(const std::string & other_user):
    closed_(false),
    other_user_(other_user),
    started_av_(false),
    data_channel_ready_(false),
    sharing_audio_(false),
    sharing_video_(false),
    sharing_data_(false),
    canceled_(false),
    connection_accepted_(false),
    is_initiator_(false),
    enable_negotiate_listener_(false)
    {}
    
#warning todo
    void PeerConn::Close() {
        if (closed_) { return; }
        
        if (pc_) {
            pc_->Close();
            pc_ = nullptr;
        }
        
        started_av_ = false;
        if (data_channel_s_) {
            data_channel_s_->Close();
            data_channel_s_ = nullptr;
        }
        if (data_channel_r_) {
            data_channel_r_->Close();
            data_channel_r_ = nullptr;
        }
        data_channel_ready_ = false;
        connect_time_ = None();
        sharing_audio_ = false;
        sharing_video_ = false;
        sharing_data_ = false;
        canceled_ = true;
        candidates_to_send_.clear();
        streams_added_acks_.clear();
        if (pc_) {
            pc_->Close();
            pc_ = nullptr;
        }
        connection_accepted_ = false;
        is_initiator_ = false;
        remote_stream_id_to_name_.clear();
        live_remote_streams_.clear();
        enable_negotiate_listener_ = false;
        call_success_cb_ = nullptr;
        call_failure_cb_ = nullptr;
        was_accepted_cb_ = nullptr;
        failing_ = None();
                
        closed_ = true;
    }
    
    std::shared_ptr<MediaStream>
    PeerConn::GetRemoteStreamByName(Easyrtc & ert,
                                    const Optional<std::string> & arg_stream_name) const
    {
        std::string stream_name = arg_stream_name || std::string("default");
        
        Optional<std::string> key_to_match;
        
        auto remote_streams = pc_->remote_streams();
        
        if (stream_name == "default") {
            if (remote_streams.size() > 0) {
                return remote_streams[0];
            }
            else {
                return nullptr;
            }
        }
        for (const std::string & room_name : Keys(ert.room_data_)) {
            Any media_ids = ert.GetRoomApiField(room_name, other_user_, "mediaIds");
            
            key_to_match = media_ids.GetAt(stream_name).AsString();
            if (key_to_match) {
                break;
            }
        }
        if (!key_to_match) {
            ert.ShowError(Easyrtc::err_codes_DEVELOPER_ERR_,
                          "remote peer does not have media stream called " + stream_name);
        }
        
        for (int i = 0; i < remote_streams.size(); i++) {
            std::string remote_id;
            
            remote_id = remote_streams[i]->id();
            
            if (!key_to_match || Some(remote_id) == key_to_match) {
                return remote_streams[i];
            }
        }
        return nullptr;
    }
}
}