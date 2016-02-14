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
    PeerConn::PeerConn():
    started_av_(false),
    data_channel_ready_(false),
    sharing_audio_(false),
    sharing_video_(false),
    sharing_data_(false)
    {}
    
    rtc::scoped_refptr<webrtc::MediaStreamInterface>
    PeerConn::GetRemoteStreamByName(Easyrtc & ert,
                                    const Optional<std::string> & arg_stream_name) const
    {
        std::string stream_name = arg_stream_name || std::string("default");
        
        Optional<std::string> key_to_match;
        
        auto remote_streams = pc_->remote_streams();
        
        if (stream_name == "default") {
            if (remote_streams->count() > 0) {
                return rtc::scoped_refptr<webrtc::MediaStreamInterface>(remote_streams->at(0));
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
        
        for (int i = 0; i < remote_streams->count(); i++) {
            std::string remote_id;
            
            remote_id = remote_streams->at(i)->label();
            
            if (!key_to_match || Some(remote_id) == key_to_match) {
                return rtc::scoped_refptr<webrtc::MediaStreamInterface>(remote_streams->at(i));
            }
        }
        return nullptr;
    }
}
}