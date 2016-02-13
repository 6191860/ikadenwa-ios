//
//  easyrtc.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/12.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "easyrtc.h"

namespace nwr {
namespace ert {
    Easyrtc::Easyrtc() {
        closed_ = false;
    }
    
    void Easyrtc::Init() {
        rtc_signaling_thread_ = rtc::scoped_ptr<rtc::Thread>(new rtc::Thread());
        bool ret = rtc_signaling_thread_->Start();
        if (!ret) { Fatal("signaling thread start failed"); }
        
        rtc_worker_thread_ = rtc::scoped_ptr<rtc::Thread>(new rtc::Thread());
        ret = rtc_worker_thread_->Start();
        if (!ret) { Fatal("worker thread start failed"); }
        
        peer_connection_factory_ =
        webrtc::CreatePeerConnectionFactory(rtc_signaling_thread_.get(),
                                            rtc_worker_thread_.get(),
                                            nullptr,
                                            nullptr,
                                            nullptr);
        if (!peer_connection_factory_) {
            Fatal("CreatePeerConnectionFactory failed");
        }
        
        auto_init_user_media_ = true;
        sdp_local_filter_ = 0;
        sdp_remote_filter_ = 0;
        ice_candidate_filter_ = 0;
        
        connection_option_timeout_ = 10000;
        connection_option_force_new_connection_ = true;
        
        have_audio_ = false;
        have_video_ = false;
        
        allowed_events_ = {
            "roomOccupant",
            "roomOccupants"
        };
        
        api_version_ = "1.0.15";
        Any ack_message_ = Any(Any::ObjectType {
            { "msgType", Any("ack") }
        });
        username_regexp_ = std::regex("^(.){1,64}$");
        cookie_id_ = "easyrtcsid";
        username_ = "";
        logging_out_ = false;
        disconnecting_ = false;
        received_media_constraints_ = Any(Any::ObjectType {
            { "mandatory", Any(Any::ObjectType {
                { "OfferToReceiveAudio", Any(true) },
                { "OfferToReceiveVideo", Any(true) }
            }) }
        });
        audio_enabled_ = true;
        video_enabled_ = true;
        data_channel_name_ = "dc";
        debug_printer_ = nullptr;
        my_easyrtcid_ = "";
        old_config_ = 0;
        offers_pending_ = 0;
        native_video_height_ = 0;
        max_p2p_message_length_ = 1000;
        native_video_width_ = 0;
        credential_ = 0;
        desired_video_properties_ = Any(Any::ObjectType {
        });
        application_name_ = "";
        data_enabled_ = false;
        
        on_error_ = FuncMake([this](const Any & error) {
            FuncCall(debug_printer_,
                     "saw error" + (error.GetAt("errorText").AsString() || std::string("")));
            printf("[Easyrtc::on_error_] %s\n", error.ToJsonString().c_str());
        });
        
    }
    
    Easyrtc::~Easyrtc() {
        if (!closed_) {
            Fatal("Easyrtc not closed");
        }
    }
    
    void Easyrtc::Close() {
        if (closed_) { return; }
        
        peer_connection_factory_ = nullptr;
        rtc_worker_thread_ = nullptr;
        rtc_signaling_thread_ = nullptr;
        
        closed_ = true;
    }
    
#warning TODO
    void Easyrtc::StopStream(webrtc::MediaStreamInterface * stream) {

    }
    
    void Easyrtc::set_sdp_filters(int local_filter, int remote_filter) {
        sdp_local_filter_ = local_filter;
        sdp_remote_filter_ = remote_filter;
    }
    
    void Easyrtc::set_peer_closed_listener(const Func<void()> & handler) {
        on_peer_closed_ = handler;
    }
    
    void Easyrtc::set_peer_failing_listener(const Func<void()> & failing_handler,
                                            const Func<void()> & recovered_handler)
    {
        on_peer_failing_ = failing_handler;
        on_peer_recovered_ = recovered_handler;
    }
    
    void Easyrtc::set_ice_candidate_filter(int filter) {
        ice_candidate_filter_ = filter;
    }
    
    void Easyrtc::set_auto_init_user_media(bool flag) {
        auto_init_user_media_ = flag;
    }
    
    std::string Easyrtc::Format(const std::string & format, const std::vector<std::string> & args) {
        std::string formatted = format;
        for (int i = 0; i < args.size(); i++) {
            std::regex regexp(nwr::Format("\\{%d\\}", i), std::regex_constants::icase);
            formatted = std::regex_replace(formatted, regexp, args[i]);
        }
        return formatted;
    }
    
    bool Easyrtc::IsSocketConnected(const std::shared_ptr<sio::Socket> & socket) {
        return socket->connected();
    }
    
    std::string Easyrtc::GetConstantString(const std::string & key) {
        if (HasKey(constant_strings_, key)) {
            return constant_strings_[key];
        }
        else {
            printf("[%s] Could not find key='%s' in easyrtc_constantStrings\n", __PRETTY_FUNCTION__, key.c_str());
            return key;
        }
    }
    
    void Easyrtc::CheckEvent(const std::string & event_name, const std::string & calling_function) {
        if (IndexOf(allowed_events_, event_name) == -1) {
            ShowError(err_codes_DEVELOPER_ERR_,
                      calling_function + " called with a bad event name = " + event_name);
            Fatal("developer error");
        }
    }
    
    void Easyrtc::AddEventListener(const std::string & event_name, const AnyEventListener & event_listener) {
        CheckEvent(event_name, "addEventListener");
        //
        // remove the event listener if it's already present so we don't end up with two copies
        //
        RemoveEventListener(event_name, event_listener);
        event_listeners_[event_name].push_back(event_listener);
    }
    
    void Easyrtc::RemoveEventListener(const std::string & event_name, const AnyEventListener & event_listener) {
        CheckEvent(event_name, "removeEventListener");
        Remove(event_listeners_[event_name], event_listener);
        if (event_listeners_[event_name].size() == 0) {
            event_listeners_.erase(event_name);
        }
    }
    
    void Easyrtc::EmitEvent(const std::string & event_name, const Any & event_data) {
        auto listeners = event_listeners_[event_name];
        for (auto & listener : listeners) {
            (*listener)( { event_data } );
        }
    }
    
    std::string Easyrtc::err_codes_BAD_NAME_ = "BAD_NAME";
    std::string Easyrtc::err_codes_CALL_ERR_ = "CALL_ERR";
    std::string Easyrtc::err_codes_DEVELOPER_ERR_ = "DEVELOPER_ERR";
    std::string Easyrtc::err_codes_SYSTEM_ERR_ = "SYSTEM_ERR";
    std::string Easyrtc::err_codes_CONNECT_ERR_ = "CONNECT_ERR";
    std::string Easyrtc::err_codes_MEDIA_ERR_ = "MEDIA_ERR";
    std::string Easyrtc::err_codes_MEDIA_WARNING_ = "MEDIA_WARNING";
    std::string Easyrtc::err_codes_INTERNAL_ERR_ = "INTERNAL_ERR";
    std::string Easyrtc::err_codes_PEER_GONE_ = "PEER_GONE";
    std::string Easyrtc::err_codes_ALREADY_CONNECTED_ = "ALREADY_CONNECTED";
    std::string Easyrtc::err_codes_BAD_CREDENTIAL_ = "BAD_CREDENTIAL";
    std::string Easyrtc::err_codes_ICECANDIDATE_ERROR_ = "ICECANDIDATE_ERROR";
    
    void Easyrtc::EnableAudioReceive(bool value) {
        received_media_constraints_.GetAt("mandatory").SetAt("OfferToReceiveAudio", Any(value));
    }
    
    void Easyrtc::EnableVideoReceive(bool value) {
        received_media_constraints_.GetAt("mandatory").SetAt("OfferToReceiveVideo", Any(value));
    }
    
    bool Easyrtc::IsNameValid(const std::string & name) {
        return std::regex_match(name, username_regexp_);
    }
    
    void Easyrtc::set_cookie_id(const std::string & cookie_id) {
        cookie_id_ = cookie_id;
    }
    
    void Easyrtc::JoinRoom(const std::string & room_name,
                           const Any &  room_parameters,
                           const std::function<void(const std::string &)> & success_cb,
                           const std::function<void(const std::string &,
                                                    const std::string &,
                                                    const std::string &)> & failure_cb)
    {
        auto thiz = shared_from_this();
        
        if (HasKey(room_join_, room_name)) {
            printf("Developer error: attempt to join room %s which you are already in.\n", room_name.c_str());
            return;
        }
        
        Any new_room_data(Any::ObjectType {
            { "roomName", Any(room_name) }
        });
        if (room_parameters.type() == Any::Type::Object) {
            Any parameters(Any::ObjectType(room_parameters.AsObject().value()));
            new_room_data.SetAt("roomParameter", parameters);
        }
        
        Any msg_data(Any::ObjectType {
            { "roomJoin", Any(Any::ObjectType()) }
        });
        
        if (websocket_) {
            
            msg_data.GetAt("roomJoin").SetAt(room_name, new_room_data);
            
            std::function<void(const std::string &, const Any &)> signaling_success =
            [thiz, room_name, new_room_data, success_cb]
            (const std::string & msg_type, const Any & msg_data) {
                
                Any room_data = msg_data.GetAt("roomData");
    
                thiz->room_join_[room_name] = new_room_data;
                
                if (success_cb) {
                    success_cb(room_name);
                }
                
                thiz->ProcessRoomData(room_data);
            };
            
            std::function<void(const std::string &,
                               const std::string &)> signaling_failure =
            [thiz, room_name, failure_cb]
            (const std::string & error_code, const std::string & error_text) {
                if (failure_cb) {
                    failure_cb(error_code, error_text, room_name);
                }
                else {
                    thiz->ShowError(error_code,
                                    thiz->Format(thiz->GetConstantString("unableToEnterRoom"), { room_name, error_text } ));
                }
            };
            
            SendSignaling(None(), "roomJoin", msg_data, signaling_success, signaling_failure);
        }
        else {
            room_join_[room_name] = new_room_data;
        }
    }
    
    void Easyrtc::LeaveRoom(const std::string & room_name,
                            const std::function<void(const std::string &)> & success_callback,
                            const std::function<void(const std::string &,
                                                     const std::string &,
                                                     const std::string &)> & failure_callback)
    {
        auto thiz = shared_from_this();
        
        if (HasKey(room_join_, room_name)) {
            if (!websocket_) {
                room_join_.erase(room_name);
            }
            else {
                Any room_item(Any::ObjectType{});
                
                room_item.SetAt(room_name, Any(Any::ObjectType {
                    { "roomName", Any(room_name) }
                }));
                
                SendSignaling(None(), "roomLeave",
                              Any(Any::ObjectType { { "roomLeave", room_item } }),
                              [thiz, room_name, success_callback](const std::string & msg_type, const Any & msg_data) {
                                  Any room_data = msg_data.GetAt("roomData");
                                  thiz->ProcessRoomData(room_data);
                                  if (success_callback) {
                                      success_callback(room_name);
                                  }
                              },
                              [room_name, failure_callback](const std::string & error_code, const std::string & error_text) {
                                  if (failure_callback) {
                                      failure_callback(error_code, error_text, room_name);
                                  }
                              });
            }
        }
    }
    
    void Easyrtc::set_video_dims(int width, int height, const Optional<double> & frame_rate) {
        if (width == 0) {
            width = 1280;
            height = 720;
        }
        
        desired_video_properties_.SetAt("width", Any(width));
        desired_video_properties_.SetAt("height", Any(height));
        if (frame_rate) {
            desired_video_properties_.SetAt("frameRate", Any(*frame_rate));
        }
    }
    
    VideoAudioMediaConstraints Easyrtc::GetUserMediaConstraints() {
        VideoAudioMediaConstraints constraints;
        //
        // _presetMediaConstraints allow you to provide your own constraints to be used
        // with initMediaSource.
        //
        if (preset_media_constraints_) {
            constraints = *preset_media_constraints_;
            preset_media_constraints_ = None();
            return constraints;
        }
        //  else if (self._desiredVideoProperties.screenCapture) { ... }
        else if (!video_enabled_) {
            constraints.video = None();
        }
        else {
            constraints.video = Some(MediaConstraints());
            
            if (desired_video_properties_.GetAt("width").AsInt()){
                MediaConstraintsSet(constraints.video->mandatory, webrtc::MediaConstraintsInterface::kMaxWidth,
                                    nwr::Format("%d", desired_video_properties_.GetAt("width").AsInt().value()));
                MediaConstraintsSet(constraints.video->mandatory, webrtc::MediaConstraintsInterface::kMinWidth,
                                    nwr::Format("%d", desired_video_properties_.GetAt("width").AsInt().value()));
            }
            if (desired_video_properties_.GetAt("height").AsInt()) {
                MediaConstraintsSet(constraints.video->mandatory, webrtc::MediaConstraintsInterface::kMaxHeight,
                                    nwr::Format("%d", desired_video_properties_.GetAt("height").AsInt().value()));
                MediaConstraintsSet(constraints.video->mandatory, webrtc::MediaConstraintsInterface::kMaxHeight,
                                    nwr::Format("%d", desired_video_properties_.GetAt("height").AsInt().value()));
            }
            if (desired_video_properties_.GetAt("frameRate").AsDouble()) {
                MediaConstraintsSet(constraints.video->mandatory, webrtc::MediaConstraintsInterface::kMaxFrameRate,
                                    nwr::Format("%f", desired_video_properties_.GetAt("frameRate").AsDouble().value()));
            }
//            if (self._desiredVideoProperties.videoSrcId) {
//                constraints.video.optional.push({sourceId: self._desiredVideoProperties.videoSrcId});
//            }
        }
        constraints.audio = audio_enabled_;

        return constraints;
    }
    
    void Easyrtc::set_application_name(const std::string & application_name) {
        application_name_ = application_name;
    }
    void Easyrtc::EnableDebug(bool enable) {
        if (enable) {
            debug_printer_ = FuncMake([](const std::string & message) {
                printf("[Easyrtc::debug_printer_] %s\n", message.c_str());
            });
        }
        else {
            debug_printer_ = nullptr;
        }
    }
    
    void Easyrtc::UpdatePresence(const std::string & state, const std::string & status_text) {
        presence_show_ = state;
        presence_status_ = status_text;
    }
    
    bool Easyrtc::SupportsGetUserMedia() {
//        return !!getUserMedia;
        return true;
    }
    
    bool Easyrtc::SupportsPeerConnections() {
        if (!SupportsGetUserMedia()) {
            return false;
        }
//        if (!window.RTCPeerConnection) {
//            return false;
//        }
//        try {
//            self.createRTCPeerConnection({"iceServers": []}, null);
//        } catch (oops) {
//            return false;
//        }
        return true;
    }
    
    rtc::scoped_refptr<webrtc::PeerConnectionInterface>
    Easyrtc::CreateRtcPeerConnection(const webrtc::PeerConnectionInterface::RTCConfiguration & configuration,
                                     const webrtc::MediaConstraintsInterface * constraints,
                                     webrtc::PeerConnectionObserver * observer)
    {
        return peer_connection_factory_->CreatePeerConnection(configuration,
                                                              constraints,
                                                              nullptr,
                                                              nullptr,
                                                              observer);
    }
    
    void Easyrtc::Disconnect() {
        
    }
    
    void Easyrtc::SetRoomApiField(const std::string & room_name)
    {
        room_api_fields_.erase(room_name);
    }
    
    void Easyrtc::SetRoomApiField(const std::string & room_name,
                                  const std::string & field_name)
    {
        room_api_fields_[room_name].erase(field_name);
    }
    
    void Easyrtc::SetRoomApiField(const std::string & room_name,
                                  const std::string & field_name,
                                  const Any & field_value)
    {
        room_api_fields_[room_name][field_name] = Any(Any::ObjectType {
            { "fieldName", Any(field_name) },
            { "fieldValue", field_value }
        });
        
        if (websocket_connected_) {
            EnqueueSendRoomApi(room_name);
        }
    }
    
    void Easyrtc::EnqueueSendRoomApi(const std::string & room_name) {
        auto thiz = shared_from_this();
        
        if (room_api_field_timer_) {
            room_api_field_timer_->Cancel();
        }

        room_api_field_timer_ = Timer::Create(TimeDuration(0.01), [thiz, room_name]{
            thiz->SendRoomApiFields(room_name, thiz->room_api_fields_[room_name]);
            thiz->room_api_field_timer_->Cancel();
            thiz->room_api_field_timer_ = nullptr;
        });
    }
    
    void Easyrtc::SendRoomApiFields(const std::string & roomName,
                                    const std::map<std::string, Any> & fields)
    {
        auto thiz = shared_from_this();
        
        Any data_to_ship(Any::ObjectType {
            { "msgType", Any("setRoomApiField") },
            { "msgData", Any(Any::ObjectType {
                { "setRoomApiField", Any(Any::ObjectType {
                    { "roomName", Any(roomName) },
                    { "field", Any(fields) }
                }) }
            }) }
        });
        
        websocket_->Emit("easyrtcCmd", { data_to_ship },
                         [thiz](const Any & ack_msg) {
                             if (ack_msg.GetAt("msgType").AsString() == Some(std::string("error"))) {
                                 thiz->ShowError(ack_msg.GetAt("msgData").GetAt("errorCode").AsString() || std::string(""),
                                                 ack_msg.GetAt("msgData").GetAt("errorText").AsString() || std::string(""));
                             }
                         });
    }
    
    void Easyrtc::ShowError(const std::string & message_code, const std::string & message) {
        FuncCall(on_error_, Any(Any::ObjectType {
            { "errorCode", Any(message_code) },
            { "errorText", Any(message) }
        }) );
    }
    
    std::string Easyrtc::CleanId(const std::string & id_string) {
        std::map<std::string, std::string> map {
            { "&", "&amp;" },
            { "<", "&lt;" },
            { ">", "&gt;" }
        };
        return Replace(id_string, std::regex("[&<>]"), [& map](const std::string & str) {
            return map[str];
        });
    }
    
    void Easyrtc::set_room_entry_listener(const Func<void()> & handler) {
        room_entry_listener_ = handler;
    }
    
    void Easyrtc::set_room_occupant_listener(const Func<void()> & listener) {
        room_occupant_listener_ = listener;
    }
    
    void Easyrtc::set_data_channel_open_listener(const Func<void()> & listener) {
        on_data_channel_open_ = listener;
    }
    
    void Easyrtc::set_data_channel_close_listener(const Func<void()> & listener) {
        on_data_channel_close_ = listener;
    }
    
    int Easyrtc::connection_count() {
        int count = 0;
        for (const auto & i : peer_conns_) {
            if (GetConnectStatus(i.first) == ConnectStatus::IsConnected) {
                count += 1;
            }
        }
        return count;
    }
    
    void Easyrtc::set_max_p2p_message_length(int max_length) {
        max_p2p_message_length_ = max_length;
    }
    
    void Easyrtc::EnableAudio(bool enabled) {
        audio_enabled_ = enabled;
    }
    
    void Easyrtc::EnableVideo(bool enabled) {
        video_enabled_ = enabled;
    }
    
    void Easyrtc::EnableDataChannels(bool enabled) {
        data_enabled_ = enabled;
    }
    
    void Easyrtc::EnableMediaTracks(bool enable,
                                    const std::vector<rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>> & tracks)
    {
        for (const auto & track : tracks) {
            track->set_enabled(enable);
        }
    }
    
    rtc::scoped_refptr<webrtc::MediaStreamInterface>
    Easyrtc::GetLocalMediaStreamByName() {
        return GetLocalMediaStreamByName("default");
    }
    
    rtc::scoped_refptr<webrtc::MediaStreamInterface>
    Easyrtc::GetLocalMediaStreamByName(const std::string & stream_name) {
        if (HasKey(named_local_media_streams_, stream_name)) {
            return named_local_media_streams_[stream_name];
        } else {
            return nullptr;
        }
    }
    
    std::vector<std::string> Easyrtc::GetLocalMediaIds() {
        return Keys(named_local_media_streams_);
    }
    
    Any Easyrtc::BuildMediaIds() {
        std::map<std::string, Any> media_map;
        for (auto iter : named_local_media_streams_) {
            auto id = iter.second->label();
            media_map[iter.first] = Any(id != "" ? id : "default");
        }
        return Any(media_map);
    }
    
    void Easyrtc::RegisterLocalMediaStreamByName(const rtc::scoped_refptr<webrtc::MediaStreamInterface> & stream) {
        RegisterLocalMediaStreamByName(stream, "default");
    }
    
    void Easyrtc::RegisterLocalMediaStreamByName(const rtc::scoped_refptr<webrtc::MediaStreamInterface> & stream,
                                                 const std::string & stream_name)
    {
//        stream.streamName = streamName;
        named_local_media_streams_[stream_name] = stream;
        
        if (stream_name != "default") {
            auto media_ids = BuildMediaIds();
            for (const auto & i : room_data_) {
                SetRoomApiField(i.first, "mediaIds", media_ids);
            }
        }
    }

    Optional<std::string> Easyrtc::GetNameOfRemoteStream(const std::string & easyrtcid) {
        return GetNameOfRemoteStream(easyrtcid);
    }
    
    Optional<std::string> Easyrtc::GetNameOfRemoteStream(const std::string & easyrtc_id, const std::string & webrtc_stream_id) {
        if (HasKey(peer_conns_, easyrtc_id)) {
            return Some(peer_conns_[easyrtc_id].remote_stream_id_to_name[webrtc_stream_id]);
        }
        
        for (const auto & i : room_data_) {
            const std::string & room_name = i.first;
            
            Any media_ids = GetRoomApiField(room_name, easyrtc_id, "mediaIds");
            if (!media_ids) {
                continue;
            }

            for (const auto & stream_name : media_ids.keys()) {
                if (media_ids.GetAt(stream_name).AsString() == Some(webrtc_stream_id)) {
                    return Some(stream_name);
                }
            }

        }
        return None();
    }
    
    void Easyrtc::CloseLocalMediaStreamByName() {
        CloseLocalMediaStreamByName("default");
    }
    
    void Easyrtc::CloseLocalMediaStreamByName(const std::string & stream_name) {
        
        rtc::scoped_refptr<webrtc::MediaStreamInterface> stream = GetLocalStream(stream_name);
        if (!stream) {
            return;
        }
        
        std::string stream_id = stream->label();
        
        if (HasKey(named_local_media_streams_, stream_name)) {
            for (const auto & id : Keys(peer_conns_)) {
                peer_conns_[id].pc->RemoveStream(stream.get());
                SendPeerMessage(id, "__closingMediaStream", Any(Any::ObjectType {
                    { "streamId", Any(stream_id) },
                    { "streamName", Any(stream_name) }
                }));
            }
            
            StopStream(named_local_media_streams_[stream_name].get());
            named_local_media_streams_.erase(stream_name);
            
            if (stream_name != "default") {
                auto media_ids = BuildMediaIds();
                for (const auto & i : room_data_) {
                    SetRoomApiField(i.first, "mediaIds", media_ids);
                }
            }
        }
    }
    
    void Easyrtc::EnableCamera(bool enable, const std::string & stream_name) {
        auto stream = GetLocalMediaStreamByName(stream_name);
        if (stream) {
            EnableMediaTracks(enable, ToMediaStreamTrackVector(stream->GetVideoTracks()));
        }
    }
    
    void Easyrtc::EnableMicrophone(bool enable, const std::string & stream_name) {
        auto stream = GetLocalMediaStreamByName(stream_name);
        if (stream) {
            EnableMediaTracks(enable, ToMediaStreamTrackVector(stream->GetAudioTracks()));
        }
    };
    

    
    
    
    
    
    Easyrtc::ConnectStatus Easyrtc::GetConnectStatus(const std::string & other_user) {
#warning todo
        return ConnectStatus::IsConnected;
    }
    
    void Easyrtc::SendSignaling(const Optional<std::string> & dest_user,
                                const std::string & msg_type,
                                const Any & msg_data,
                                const std::function<void (const std::string &, const Any &)> & success_callback,
                                const std::function<void (const std::string &, const std::string &)> & error_callback)
    {
        
    }
    
    void Easyrtc::ProcessRoomData(const Any & room_data) {
        
    }
    

    
    std::map<std::string, std::string> Easyrtc::constant_strings_ = {
        { "unableToEnterRoom", "Unable to enter room {0} because {1}" },
        { "resolutionWarning", "Requested video size of {0}x{1} but got size of {2}x{3}" },
        { "badUserName", "Illegal username {0}" },
        { "localMediaError", "Error getting local media stream: {0}" },
        { "miscSignalError", "Miscellaneous error from signalling server. It may be ignorable." },
        { "noServer", "Unable to reach the EasyRTC signalling server." },
        { "badsocket", "Socket.io connect event fired with bad websocket." },
        { "icf", "Internal communications failure" },
        { "statsNotSupported", "call statistics not supported by this browser, try Chrome." },
        { "noWebrtcSupport", "Your browser doesn't appear to support WebRTC." },
        { "gumFailed", "Failed to get access to local media. Error code was {0}." },
        { "requireAudioOrVideo", "At least one of audio and video must be provided" }
    };
    
}
}
