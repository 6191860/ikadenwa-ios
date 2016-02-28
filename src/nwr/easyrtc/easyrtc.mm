//
//  easyrtc.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/12.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "easyrtc.h"

#include <nwr/jsrtc/NWRHtmlMediaElementView.h>

namespace nwr {
namespace ert {
    std::shared_ptr<Easyrtc> Easyrtc::Create(const std::string & server_path,
                                             const ObjcPointer & work_view)
    {
        auto thiz = std::shared_ptr<Easyrtc>(new Easyrtc());
        thiz->Init(server_path, work_view);
        return thiz;
    }
    
    Easyrtc::~Easyrtc() {
        if (!closed_) {
            Fatal("Easyrtc not closed");
        }
        printf("[Easyrtc::dtor]\n");
    }
    
    Easyrtc::Easyrtc() {
        closed_ = false;
    }
    
    void Easyrtc::Init(const std::string & server_path,
                       const ObjcPointer & work_view)
    {
        peer_connection_factory_ = std::make_shared<RtcPeerConnectionFactory>();
        
        auto_init_user_media_ = true;
        
        connection_options_.connect_timeout = Some(TimeDuration(10.0));
        connection_options_.force_new_connection = Some(true);
        
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
        logging_out_ = false;
        disconnecting_ = false;
        received_media_constraints_ = std::make_shared<MediaTrackConstraints>();
        received_media_constraints_->mandatory()
        .SetEntry(webrtc::MediaConstraintsInterface::kOfferToReceiveAudio, true);
        received_media_constraints_->mandatory()
        .SetEntry(webrtc::MediaConstraintsInterface::kOfferToReceiveVideo, true);
        audio_enabled_ = true;
        video_enabled_ = true;
        data_channel_name_ = "dc";
        debug_printer_ = nullptr;
        old_config_ = Any(Any::ObjectType{});
        native_video_width_ = 0;
        native_video_height_ = 0;
        max_p2p_message_length_ = 1000;
        desired_video_properties_ = Any(Any::ObjectType {
        });
        application_name_ = "";
        data_enabled_ = false;
        on_error_ = [this](const Any & error) {
            FuncCall(debug_printer_,
                     "saw error" + (error.GetAt("errorText").AsString() || std::string("")));
            printf("[Easyrtc::on_error_] %s\n", error.ToJsonString().c_str());
        };
        pc_config_ = std::make_shared<webrtc::PeerConnectionInterface::RTCConfiguration>();
        use_fresh_ice_each_peer_ = false;
        update_configuration_info_ = [this] {
            UpdateConfiguration();
        };
        auto_add_close_buttons_ = true;
        
        server_path_ = server_path;
        work_view_ = work_view;
    }
    
    void Easyrtc::Close() {
        if (closed_) { return; }
        
        sdp_local_filter_ = nullptr;
        sdp_remote_filter_ = nullptr;
        on_peer_closed_ = nullptr;
        on_peer_failing_ = nullptr;
        on_peer_recovered_ = nullptr;
        ice_candidate_filter_ = nullptr;
        event_listeners_.clear();
        ack_message_ = nullptr;
        session_fields_.clear();
        debug_printer_ = nullptr;
        old_config_ = nullptr;
        offers_pending_.clear();
        room_join_.clear();
        desired_video_properties_ = nullptr;
        last_logged_in_list_.clear();
        receive_peer_.Clear();
        for (const auto & peer : Keys(peer_conns_)) {
            DeletePeerConn(peer);
        }
        acceptance_pending_.clear();
        room_api_fields_.clear();
        if (room_api_field_timer_) {
            room_api_field_timer_->Cancel();
            room_api_field_timer_ = nullptr;
        }
        room_entry_listener_ = nullptr;
        room_occupant_listener_ = nullptr;
        on_data_channel_open_ = nullptr;
        on_data_channel_close_ = nullptr;
        named_local_media_streams_.clear();
        room_data_.clear();
        accept_check_ = nullptr;
        stream_acceptor_ = nullptr;
        on_error_ = nullptr;
        call_canceled_ = nullptr;
        on_stream_closed_ = nullptr;
        receive_server_cb_ = nullptr;
        disconnect_listener_ = nullptr;
        if (websocket_) {
#warning todo : if this is preallocated, I would not close it.
            websocket_->Disconnect();
            websocket_ = nullptr;
        }
        pc_config_ = nullptr;
        pc_config_to_use_ = nullptr;
        if (closed_channel_) {
            closed_channel_->Disconnect();
            closed_channel_ = nullptr;
        }
        fields_.Clear();
        turn_servers_.clear();
        queued_messages_.clear();
        for (const auto & key : Keys(aggregating_timers_)) {
            aggregating_timers_[key].Clear();
        }
        aggregating_timers_.clear();
        websocket_listeners_.clear();
        update_configuration_info_ = nullptr;
        if (preallocated_socket_io_) {
            preallocated_socket_io_->Disconnect();
            preallocated_socket_io_ = nullptr;
        }
        video_ids_.clear();
        video_id_to_caller_map_.clear();
        on_call_ = nullptr;
        on_hangup_ = nullptr;
        refresh_pane_ = None();
        got_media_callback_ = nullptr;
        got_connection_callback_ = nullptr;
        work_view_ = nullptr;
        
        peer_connection_factory_->Close();
        peer_connection_factory_ = nullptr;
        
        closed_ = true;
    }
    
    Any Easyrtc::CreateIceServer(const std::string & url, const std::string & username, const std::string & credential) {
        return Any(Any::ObjectType{
            { "url", Any(url) },
            { "username", Any(username) },
            { "credential", Any(credential) }
        });
    }
    
    void Easyrtc::StopStream(const std::shared_ptr<MediaStream> & stream) {
        auto tracks = stream->audio_tracks();
        for (int i = 0; i < tracks.size(); i++) {
            tracks[i]->Stop();
        }
        tracks = stream->video_tracks();
        for (int i = 0; i < tracks.size(); i++) {
            tracks[i]->Stop();
        }
    }
    
    void Easyrtc::set_sdp_filters(const std::function<std::string(const std::string &)> & local_filter,
                                  const std::function<std::string(const std::string &)> & remote_filter) {
        sdp_local_filter_ = local_filter;
        sdp_remote_filter_ = remote_filter;
    }
    
    void Easyrtc::set_peer_closed_listener(const std::function<void(const std::string &)> & handler) {
        on_peer_closed_ = handler;
    }
    
    void Easyrtc::set_peer_failing_listener(const std::function<void(const std::string &)> & failing_handler,
                                            const std::function<void(const std::string &,
                                                                     const std::chrono::system_clock::time_point &,
                                                                     const std::chrono::system_clock::time_point &)> & recovered_handler)
    {
        on_peer_failing_ = failing_handler;
        on_peer_recovered_ = recovered_handler;
    }
    
    void Easyrtc::set_ice_candidate_filter(const std::function<Any(const Any &, bool)> & filter) {
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
    
    bool Easyrtc::IsSocketConnected(const std::shared_ptr<const sio0::Socket> & socket) {
        return socket && socket->socket() && socket->socket()->connected();
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
    std::string Easyrtc::err_codes_NOVIABLEICE_ = "NOVIABLEICE";
    std::string Easyrtc::err_codes_SIGNAL_ERROR_ = "SIGNAL_ERROR";
    
    void Easyrtc::EnableAudioReceive(bool value) {
        received_media_constraints_->mandatory()
        .SetEntry(webrtc::MediaConstraintsInterface::kOfferToReceiveAudio, value);
    }
    
    void Easyrtc::EnableVideoReceive(bool value) {
        received_media_constraints_->mandatory()
        .SetEntry(webrtc::MediaConstraintsInterface::kOfferToReceiveVideo, value);
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
    
    MediaStreamConstraints Easyrtc::GetUserMediaConstraints() {
        MediaStreamConstraints constraints;
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
            constraints.set_video(nullptr);
        }
        else {
            constraints.set_video(std::make_shared<MediaTrackConstraints>());

            auto & mandatory = constraints.video()->mandatory();
            
            if (desired_video_properties_.GetAt("width").AsInt()){
                mandatory.SetEntry(webrtc::MediaConstraintsInterface::kMaxWidth,
                                   desired_video_properties_.GetAt("width").AsInt().value());
                mandatory.SetEntry(webrtc::MediaConstraintsInterface::kMinWidth,
                                   desired_video_properties_.GetAt("width").AsInt().value());
            }
            if (desired_video_properties_.GetAt("height").AsInt()) {
                mandatory.SetEntry(webrtc::MediaConstraintsInterface::kMaxHeight,
                                   desired_video_properties_.GetAt("height").AsInt().value());
                mandatory.SetEntry(webrtc::MediaConstraintsInterface::kMinHeight,
                                   desired_video_properties_.GetAt("height").AsInt().value());
            }
            if (desired_video_properties_.GetAt("frameRate").AsDouble()) {
                mandatory.SetEntry(webrtc::MediaConstraintsInterface::kMaxFrameRate,
                                   desired_video_properties_.GetAt("frameRate").AsDouble().value());
            }
//            if (self._desiredVideoProperties.videoSrcId) {
//                constraints.video.optional.push({sourceId: self._desiredVideoProperties.videoSrcId});
//            }
        }
        constraints.set_audio(audio_enabled_ ? std::make_shared<MediaTrackConstraints>() : nullptr);

        return constraints;
    }
    
    void Easyrtc::set_application_name(const std::string & application_name) {
        application_name_ = application_name;
    }
    void Easyrtc::EnableDebug(bool enable) {
        if (enable) {
            debug_printer_ = [](const std::string & message) {
                printf("[Easyrtc::debug_printer_] %s\n", message.c_str());
            };
        }
        else {
            debug_printer_ = nullptr;
        }
    }
    
    bool Easyrtc::SupportsGetUserMedia() {
        return true;
    }
    
    bool Easyrtc::SupportsPeerConnections() {
        if (!SupportsGetUserMedia()) {
            return false;
        }
        return true;
    }
    
    std::shared_ptr<RtcPeerConnection>
    Easyrtc::CreateRtcPeerConnection(const webrtc::PeerConnectionInterface::RTCConfiguration & configuration,
                                     const std::shared_ptr<MediaTrackConstraints> & constraints)
    {
        return peer_connection_factory_->CreatePeerConnection(configuration,
                                                              constraints);
    }
    
    webrtc::DataChannelInit Easyrtc::GetDataChannelConstraints() {
        webrtc::DataChannelInit config;
        config.reliable = true;
        return config;
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
        
        websocket_->JsonEmit("easyrtcCmd", {
            data_to_ship,
            AnyFuncMake([thiz](const Any & ack_msg) {
                if (ack_msg.GetAt("msgType").AsString() == Some(std::string("error"))) {
                    thiz->ShowError(ack_msg.GetAt("msgData").GetAt("errorCode").AsString() || std::string(),
                                    ack_msg.GetAt("msgData").GetAt("errorText").AsString() || std::string());
                }
            })
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
    
    void Easyrtc::set_room_entry_listener(const std::function<void(bool, const std::string &)> & handler) {
        room_entry_listener_ = handler;
    }
    
    void Easyrtc::set_room_occupant_listener(const std::function<void(const Optional<std::string> &,
                                                                      const std::map<std::string, Any> &,
                                                                      const Any &)> & listener) {
        room_occupant_listener_ = listener;
    }
    
    void Easyrtc::set_data_channel_open_listener(const std::function<void(const std::string &, bool)> & listener) {
        on_data_channel_open_ = listener;
    }
    
    void Easyrtc::set_data_channel_close_listener(const std::function<void(const std::string &)> & listener) {
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
                                    const std::vector<std::shared_ptr<MediaStreamTrack>> & tracks)
    {
        for (const auto & track : tracks) {
            track->set_enabled(enable);
        }
    }
    
    std::shared_ptr<MediaStream>
    Easyrtc::GetLocalMediaStreamByName(const Optional<std::string> & arg_stream_name) {
        std::string stream_name = arg_stream_name || std::string("default");
        
        if (HasKey(named_local_media_streams_, stream_name)) {
            return named_local_media_streams_[stream_name];
        } else {
            return nullptr;
        }
    }
    
    std::vector<std::string> Easyrtc::GetLocalMediaIds() {
        return Keys(named_local_media_streams_);
    }
    
    std::map<std::string, Any> Easyrtc::BuildMediaIds() {
        std::map<std::string, Any> media_map;
        for (auto iter : named_local_media_streams_) {
            auto id = iter.second->id();
            media_map[iter.first] = Any(std::string(id != "" ? id : "default"));
        }
        return media_map;
    }
    
    void Easyrtc::RegisterLocalMediaStreamByName(const std::shared_ptr<MediaStream> & stream,
                                                 const Optional<std::string> & arg_stream_name)
    {
        std::string stream_name = arg_stream_name || std::string("default");
//        stream.streamName = streamName;
        named_local_media_streams_[stream_name] = stream;
        
        if (stream_name != "default") {
            auto media_ids = BuildMediaIds();
            for (const auto & i : room_data_) {
                SetRoomApiField(i.first, "mediaIds", Any(media_ids));
            }
        }
    }
    
    void Easyrtc::Register3rdPartyLocalMediaStream(const std::shared_ptr<MediaStream> & stream,
                                                   const Optional<std::string> & stream_name)
    {
        RegisterLocalMediaStreamByName(stream, stream_name);
    }
    
    Optional<std::string> Easyrtc::GetNameOfRemoteStream(const std::string & easyrtcid,
                                                         const Optional<std::string> & arg_webrtc_stream_id)
    {
        std::string webrtc_stream_id = arg_webrtc_stream_id || std::string("default");
        
        if (HasKey(peer_conns_, easyrtcid)) {
            return Some(peer_conns_[easyrtcid]->remote_stream_id_to_name()[webrtc_stream_id]);
        }
        
        for (const auto & i : room_data_) {
            const std::string & room_name = i.first;
            
            Any media_ids = GetRoomApiField(room_name, easyrtcid, "mediaIds");
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
    
    void Easyrtc::CloseLocalMediaStreamByName(const Optional<std::string> & arg_stream_name) {
        std::string stream_name = arg_stream_name || std::string("default");
        
        std::shared_ptr<MediaStream> stream = GetLocalStream(Some(stream_name));
        if (!stream) {
            return;
        }
        
        std::string stream_id = stream->id();
        
        if (HasKey(named_local_media_streams_, stream_name)) {
            for (const auto & id : Keys(peer_conns_)) {
                peer_conns_[id]->pc()->RemoveStream(stream);
                SendPeerMessage(Any(id), "__closingMediaStream", Any(Any::ObjectType {
                    { "streamId", Any(stream_id) },
                    { "streamName", Any(stream_name) }
                }), nullptr, nullptr);
            }
            
            StopStream(named_local_media_streams_[stream_name]);
            named_local_media_streams_.erase(stream_name);
            
            if (stream_name != "default") {
                auto media_ids = BuildMediaIds();
                for (const auto & i : room_data_) {
                    SetRoomApiField(i.first, "mediaIds", Any(media_ids));
                }
            }
        }
    }
    
    void Easyrtc::EnableCamera(bool enable, const Optional<std::string> & stream_name) {
        auto stream = GetLocalMediaStreamByName(stream_name);
        if (stream) {
            EnableMediaTracks(enable, stream->video_tracks());
        }
    }
    
    void Easyrtc::EnableMicrophone(bool enable, const Optional<std::string> & stream_name) {
        auto stream = GetLocalMediaStreamByName(stream_name);
        if (stream) {
            EnableMediaTracks(enable, stream->audio_tracks());
        }
    }
    
    std::shared_ptr<MediaStream> Easyrtc::GetLocalStream(const Optional<std::string> & stream_name) {
        return GetLocalMediaStreamByName(stream_name);
    }
    
    void Easyrtc::ClearMediaStream(const ObjcPointer & arg_element) {
        auto element = (NWRHtmlMediaElementView *)ObjcPointerGet(arg_element);
        element.srcObject = nullptr;
    }
    
    void Easyrtc::SetVideoObjectSrc(const ObjcPointer & arg_video_object,
                                    const std::shared_ptr<MediaStream> & stream)
    {
        auto video_object = (NWRHtmlMediaElementView *)ObjcPointerGet(arg_video_object);
        
        if (stream) {
            video_object.srcObject = stream;
        }
        else {
            video_object.srcObject = nullptr;
        }
    }
    
    std::shared_ptr<MediaStream>
    Easyrtc::BuildLocalMediaStream(const std::string & stream_name,
                                   const std::vector<std::shared_ptr<MediaStreamTrack>> & audio_tracks,
                                   const std::vector<std::shared_ptr<MediaStreamTrack>> & video_tracks)
    {
        std::shared_ptr<MediaStream> stream_to_clone = nullptr;
        for (const auto & key : Keys(named_local_media_streams_)) {
            stream_to_clone = named_local_media_streams_[key];
            if (stream_to_clone) { break; }
        }
        
        if (!stream_to_clone) {
            for (const auto & key : Keys(peer_conns_)) {
                auto remote_streams = peer_conns_[key]->pc()->remote_streams();
                if (remote_streams.size() > 0) {
                    stream_to_clone = remote_streams[0];
                }
            }
        }
        
        if (!stream_to_clone) {
            ShowError(err_codes_DEVELOPER_ERR_,
                      "Attempt to create a mediastream without one to clone from");
            return nullptr;
        }
        
        //
        // clone whatever mediastream we found, and remove any of it's
        // tracks.
        //
        
        auto media_clone = peer_connection_factory_->CreateMediaStream(stream_name);
        
        for (const auto & track : audio_tracks) {
            media_clone->AddTrack(track);
        }
        
        for (const auto & track : video_tracks) {
            media_clone->AddTrack(track);
        }
        
        RegisterLocalMediaStreamByName(media_clone, Some(stream_name));
        
        return media_clone;
    }
    
    std::string Easyrtc::FormatError(const Any & error) {
        return error.ToJsonString();
    }
    
    void Easyrtc::InitMediaSource(const Optional<std::string> & arg_stream_name,
                                  const std::function<void(const std::shared_ptr<MediaStream> &)> & success_callback,
                                  const std::function<void(const std::string &,
                                                           const std::string &)> & arg_error_callback)
    {
        UIView * work_view = (UIView *)ObjcPointerGet(work_view_);
        std::string stream_name = arg_stream_name || std::string("default");
        
        auto thiz = shared_from_this();
        
        auto error_callback = arg_error_callback;
        
        FuncCall(debug_printer_, "about to request local media");

        have_audio_ = audio_enabled_;
        have_video_ = video_enabled_;

        if (!error_callback) {
            error_callback = [thiz](const std::string & error_code, const std::string & error_text) {
                std::string message = "easyrtc.initMediaSource: " + error_text;
                FuncCall(thiz->debug_printer_, message);
                thiz->ShowError(err_codes_MEDIA_ERR_, message);
            };
        }
        
        if (!SupportsGetUserMedia()) {
            FuncCall(error_callback, err_codes_MEDIA_ERR_, GetConstantString("noWebrtcSupport"));
            return;
        }
        
        if (!success_callback) {
            ShowError(err_codes_DEVELOPER_ERR_,
                      "easyrtc.initMediaSource not supplied a successCallback");
            return;
        }

        auto mode = GetUserMediaConstraints();
        
        auto on_user_media_success = [thiz, work_view, stream_name, success_callback]
        (const std::shared_ptr<MediaStream> & stream)
        {
            FuncCall(thiz->debug_printer_, "getUserMedia success callback entered");
            FuncCall(thiz->debug_printer_, "successfully got local media");
            
            //            stream.streamName = streamName;
            thiz->RegisterLocalMediaStreamByName(stream, Some(stream_name));
            
            if (thiz->have_video_) {
#warning todo temporary visible
                auto video_obj = [[NWRHtmlMediaElementView alloc] initWithFrame: CGRectMake(0, 0, 100, 100)];
                video_obj.videoSizeEmitter->Once([thiz, stream, video_obj, success_callback](const CGSize & videoSize){
                    thiz->native_video_width_ = videoSize.width;
                    thiz->native_video_height_ = videoSize.height;
                    
                    bool width_not_match = thiz->desired_video_properties_.HasKey("width") &&
                    thiz->desired_video_properties_.GetAt("width").AsInt() != Some(thiz->native_video_width_);
                    bool height_not_match = thiz->desired_video_properties_.HasKey("height") &&
                    thiz->desired_video_properties_.GetAt("height").AsInt() != Some(thiz->native_video_height_);
                    
                    if (width_not_match || height_not_match)
                    {
                        int dw = thiz->desired_video_properties_.GetAt("width").AsInt() || 0;
                        int dh = thiz->desired_video_properties_.GetAt("height").AsInt() || 0;
                        
                        thiz->ShowError(thiz->err_codes_MEDIA_WARNING_,
                                        thiz->Format(thiz->GetConstantString("resolutionWarning"),
                                                     {
                                                         nwr::Format("%d", dw),
                                                         nwr::Format("%d", dh),
                                                         nwr::Format("%d", thiz->native_video_width_),
                                                         nwr::Format("%d", thiz->native_video_height_)
                                                     } ));
                    }
                    
                    thiz->SetVideoObjectSrc(ObjcPointerMake(video_obj), nullptr);
                    [video_obj removeFromSuperview];
                    
                    thiz->UpdateConfigurationInfo();
                    FuncCall(success_callback, stream);
                });
                [work_view addSubview: video_obj];
                thiz->SetVideoObjectSrc(ObjcPointerMake(video_obj), stream);
#warning todo porting timeout
            }
            else {
                thiz->UpdateConfigurationInfo();
                if (success_callback) {
                    success_callback(stream);
                }
            }
        };

        auto on_user_media_error = [thiz, stream_name, error_callback](const std::string & error) {
            printf("getusermedia failed\n");
            FuncCall(thiz->debug_printer_, "failed to get local media");
            
            if (error_callback) {
                printf("invoking error callback: %s\n", error.c_str());
                error_callback(thiz->err_codes_MEDIA_ERR_,
                               thiz->Format(thiz->GetConstantString("gumFailed"), { error }));
            }
            
            thiz->CloseLocalMediaStreamByName(Some(stream_name));
            
            thiz->have_audio_ = false;
            thiz->have_video_ = false;
            thiz->UpdateConfigurationInfo();
        };
        
        if (!audio_enabled_ && !video_enabled_) {
            on_user_media_error(thiz->GetConstantString("requireAudioOrVideo"));
            return;
        }

        peer_connection_factory_->GetUserMedia(mode,
                                               on_user_media_success,
                                               on_user_media_error);
    }
    
    void Easyrtc::set_accept_checker(const std::function<void (const std::string &,
                                                               const std::function<void (bool,
                                                                                         const Optional<std::vector<std::string>> &)> &)> & accept_check)
    {
        accept_check_ = accept_check;
    }
    
    void Easyrtc::set_stream_acceptor(const std::function<void(const std::string &,
                                                               const std::shared_ptr<MediaStream> &,
                                                               const std::string &)> & acceptor) {
        stream_acceptor_ = acceptor;
    }
    
    void Easyrtc::set_on_error(const std::function<void(const Any &)> & err_listener) {
        on_error_ = err_listener;
    }
    
    void Easyrtc::set_call_canceled(const std::function<void(const std::string &, bool)> & call_canceled) {
        call_canceled_ = call_canceled;
    }
    
    void Easyrtc::set_on_stream_closed(const std::function<void(const std::string &,
                                                                const std::shared_ptr<MediaStream> &,
                                                                const std::string &)> & on_stream_closed)
    {
        on_stream_closed_ = on_stream_closed;
    }
    
    bool Easyrtc::SupportsDataChannels() {
        return true;
    }
    
    void Easyrtc::SetPeerListener(const Optional<std::string> & msg_type,
                                  const Optional<std::string> & source,
                                  const ReceivePeerCallback & listener)
    {
        if (!msg_type) {
            receive_peer_.cb = listener;
        }
        else {
            if (!source) {
                receive_peer_.msg_types[*msg_type].cb = listener;
            }
            else {
                receive_peer_.msg_types[*msg_type].sources[*source] = {
                    listener
                };
            }
        }
    }
    
    void Easyrtc::ReceivePeerDistribute(const std::string & easyrtcid,
                                        const Any & msg,
                                        const Any & targeting)
    {
        Optional<std::string> msg_type_opt = msg.GetAt("msgType").AsString();
        Any msg_data = msg.GetAt("msgData");
        if (!msg_type_opt) {
            printf("received peer message without msgType; %s\n", msg.ToJsonString().c_str());
            return;
        }
        std::string msg_type = msg_type_opt.value();
        
        if (HasKey(receive_peer_.msg_types, msg_type)) {
            auto & msg_entry = receive_peer_.msg_types[msg_type];
            
            if (HasKey(msg_entry.sources, easyrtcid) &&
                msg_entry.sources[easyrtcid].cb)
            {
                (msg_entry.sources[easyrtcid].cb)(easyrtcid, msg_type, msg_data, targeting);
                return;
            }
            if (msg_entry.cb) {
                (msg_entry.cb)(easyrtcid, msg_type, msg_data, targeting);
                return;
            }
        }
        if (receive_peer_.cb) {
            (receive_peer_.cb)(easyrtcid, msg_type, msg_data, targeting);
        }
    }
    
    void Easyrtc::set_server_listener(const std::function<void(const std::string &,
                                                               const Any &,
                                                               const Any &)> & listener) {
        receive_server_cb_ = listener;
    }
    
    void Easyrtc::set_socket_url(const std::string & socket_url,
                                 const Optional<sio0::SocketOptions> & options)
    {
        FuncCall(debug_printer_, "WebRTC signaling server URL set to " + socket_url);
        
        server_path_ = socket_url;
        if (options) {
            connection_options_ = *options;
        }
    }
    
    bool Easyrtc::set_user_name(const std::string & username) {
        if (my_easyrtcid_) {
            ShowError(err_codes_DEVELOPER_ERR_, "easyrtc.setUsername called after authentication");
            return false;
        }
        else if (IsNameValid(username)) {
            username_ = Some(username);
            return true;
        }
        else {
            ShowError(err_codes_BAD_NAME_,
                      Format(GetConstantString("badUserName"), { username }));
            return false;
        }
    }
    
    std::vector<std::tuple<std::string, std::string>>
    Easyrtc::UsernameToIds(const std::string & username,
                           const Optional<std::string> & room)
    {
        std::vector<std::tuple<std::string, std::string>> results;
        
        for (const auto & room_name : Keys(last_logged_in_list_)) {
            if (room && room_name != *room) {
                continue;
            }
            
            for (const std::string & id : last_logged_in_list_[room_name].keys()) {
                auto entry = last_logged_in_list_[room_name].GetAt(id);
                if (entry.GetAt("username").AsString() == Some(username)) {
                    results.push_back(std::tuple<std::string, std::string>(id, room_name));
                }
            }
        }
        return results;
    }
    
    Any Easyrtc::GetRoomApiField(const std::string & room_name,
                                 const std::string & easyrtcid,
                                 const std::string & field_name)
    {

        if (HasKey(last_logged_in_list_, room_name) &&
            last_logged_in_list_[room_name].HasKey(easyrtcid))
        {
            auto info = last_logged_in_list_[room_name].GetAt(easyrtcid);
            if (info.GetAt("apiField").HasKey(field_name)) {
                return info.GetAt("apiField").GetAt(field_name).GetAt("fieldValue");
            }
        }
        return nullptr;
    }
    
    void Easyrtc::set_credential(const Any & credential_param) {
        credential_ = Some(credential_param.ToJsonString());
    }
    
    void Easyrtc::set_disconnect_listener(const std::function<void()> & disconnect_listener) {
        disconnect_listener_ = disconnect_listener;
    }
    
    std::string Easyrtc::IdToName(const std::string & easyrtcid) {
        for (const std::string & room_name : Keys(last_logged_in_list_)) {
            if (last_logged_in_list_[room_name].HasKey(easyrtcid)) {
                auto entry = last_logged_in_list_[room_name].GetAt(easyrtcid);
                if (entry.GetAt("username")) {
                    return entry.GetAt("username").AsString().value();
                }
            }
        }
        return easyrtcid;
    }
    
    void Easyrtc::set_use_fresh_ice_each_peer_connection(bool value) {
        use_fresh_ice_each_peer_ = value;
    }
    
    std::shared_ptr<webrtc::PeerConnectionInterface::RTCConfiguration> Easyrtc::server_ice() {
        return pc_config_;
    }
    
    bool Easyrtc::HaveTracks(const Optional<std::string> & easyrtcid,
                             bool check_audio,
                             const Optional<std::string> & stream_name)
    {
        std::shared_ptr<MediaStream> stream;
        
        if (!easyrtcid) {
            stream = GetLocalMediaStreamByName(stream_name);
        }
        else {
            if (!HasKey(peer_conns_, easyrtcid.value())) {
                printf("Developer error: haveTracks called about a peer you don't have a connection to\n");
                return false;
            }
            auto peer_conn_obj = peer_conns_[easyrtcid.value()];
            
            stream = peer_conn_obj->GetRemoteStreamByName(*this, stream_name);
        }
        if (!stream) {
            return false;
        }
        
        std::vector<std::shared_ptr<MediaStreamTrack>> tracks;
        if (check_audio) {
            tracks = stream->audio_tracks();
        }
        else {
            tracks = stream->video_tracks();
        }

        return tracks.size() > 0;
    }
    
    bool Easyrtc::HaveAudioTrack(const Optional<std::string> & easyrtcid, const Optional<std::string> & stream_name) {
        return HaveTracks(easyrtcid, true, stream_name);
    }
    
    bool Easyrtc::HaveVideoTrack(const Optional<std::string> & easyrtcid, const Optional<std::string> & stream_name) {
        return HaveTracks(easyrtcid, false, stream_name);
    }
    
    Any Easyrtc::GetRoomField(const std::string & room_name, const std::string & field_name) {
        auto fields = GetRoomFields(room_name);
        return fields[field_name].GetAt("fieldValue");
    }
    
    bool Easyrtc::SupportsStatistics() {
        return false;
    }
    
    bool Easyrtc::IsEmptyObj(const Any & obj) {
        if (!obj) {
            return true;
        }
        return obj.count() == 0;
    }
    
    void Easyrtc::DisconnectBody() {
        logging_out_ = true;
        acceptance_pending_.clear();
        disconnecting_ = true;
        closed_channel_ = websocket_;
        if (websocket_connected_) {
            if (!preallocated_socket_io_) {
                websocket_->Disconnect();
            }
            websocket_connected_ = false;
        }
        HangupAll();
        if (room_occupant_listener_) {
            for (const auto & key : Keys(last_logged_in_list_)) {
                (room_occupant_listener_)(Some(key), std::map<std::string, Any>{}, Any());
            }
        }
        last_logged_in_list_.clear();
        
        EmitEvent("roomOccupant", Any(Any::ObjectType{}));
        room_data_.clear();
        room_join_.clear();
        logging_out_ = false;
        my_easyrtcid_ = None();
        disconnecting_ = false;
        old_config_ = Any(Any::ObjectType{});
    }
    
    void Easyrtc::Disconnect() {
        auto thiz = shared_from_this();
        
        FuncCall(debug_printer_, "attempt to disconnect from WebRTC signalling server");

        disconnecting_ = true;
        HangupAll();
        logging_out_ = true;

        //
        // The hangupAll may try to send configuration information back to the server.
        // Collecting that information is asynchronous, we don't actually close the
        // connection until it's had a chance to be sent. We allocate 100ms for collecting
        // the info, so 250ms should be sufficient for the disconnecting.
        //
        
        Timer::Create(TimeDuration(0.25), [thiz](){
            if (thiz->websocket_) {
                thiz->websocket_->Disconnect();
                
                thiz->closed_channel_ = thiz->websocket_;
                thiz->websocket_ = nullptr;
            }
            thiz->logging_out_ = false;
            thiz->disconnecting_ = false;
            
            FuncCall(thiz->room_occupant_listener_, None(), std::map<std::string, Any>{}, Any());
            
            thiz->EmitEvent("roomOccupant", Any(Any::ObjectType{}));
            thiz->old_config_ = Any(Any::ObjectType{});
        });
    }
    
    void Easyrtc::SendSignaling(const Optional<std::string> & dest_user,
                                const std::string & msg_type,
                                const Any & msg_data,
                                const std::function<void (const std::string &,
                                                          const Any &)> & success_callback,
                                const std::function<void (const std::string &,
                                                          const std::string &)> & error_callback)
    {
        auto thiz = shared_from_this();
        
        if (!websocket_) {
            Fatal("Attempt to send message without a valid connection to the server.");
        }
        else {
            Any data_to_ship(Any::ObjectType {
                { "msgType", Any(msg_type) }
            });
            
            if (dest_user) {
                data_to_ship.SetAt("targetEasyrtcid", Any(dest_user.value()));
            }
            if (msg_data) {
                data_to_ship.SetAt("msgData", msg_data);
            }

            FuncCall(debug_printer_,
                     std::string("sending socket message ") + data_to_ship.ToJsonString());
            
            websocket_->JsonEmit("easyrtcCmd", {
                data_to_ship,
                AnyFuncMake([thiz, success_callback, error_callback]
                            (const Any & arg_ack_msg) {
                                Any ack_msg = arg_ack_msg;
                                
                                if (ack_msg.GetAt("msgType").AsString() != Some(std::string("error")) ) {
                                    if (!ack_msg.HasKey("msgData")) {
                                        ack_msg.SetAt("msgData", nullptr);
                                    }
                                    FuncCall(success_callback,
                                             ack_msg.GetAt("msgType").AsString() || std::string(),
                                             ack_msg.GetAt("msgData"));
                                }
                                else {
                                    auto msg_data = ack_msg.GetAt("msgData");
                                    auto error_code = msg_data.GetAt("errorCode").AsString() || std::string();
                                    auto error_text = msg_data.GetAt("errorText").AsString() || std::string();
                                    
                                    if (error_callback) {
                                        error_callback(error_code, error_text);
                                    } else {
                                        thiz->ShowError(error_code, error_text);
                                    }
                                }
                            })
            });
        }
    }
    
    void Easyrtc::SendByChunkHelper(const std::string & dest_user, const std::string & msg_data) {
        std::string transfer_id = nwr::Format("%s-%d", dest_user.c_str(), send_by_chunk_uid_counter_);

        int number_of_chunks = static_cast<int>(ceil(static_cast<double>(msg_data.length()) / max_p2p_message_length_));

        Any start_message(Any::ObjectType {
            { "transfer", Any("start") },
            { "transferId", Any(transfer_id) },
            { "parts", Any(number_of_chunks) }
        });
        
        Any end_message(Any::ObjectType {
            { "transfer", Any("end") },
            { "transfer_id", Any(transfer_id) }
        });
        
        peer_conns_[dest_user]->data_channel_s()->Send(eio::PacketData(start_message.ToJsonString()));
        
        int pos = 0;
        int len = static_cast<int>(msg_data.length());
        for (; pos < len; pos += max_p2p_message_length_) {
            Any message(Any::ObjectType {
                { "transfer_id", Any(transfer_id) },
                { "data", Any(msg_data.substr(pos, max_p2p_message_length_)) },
                { "transfer", Any("chunk") }
            });

            peer_conns_[dest_user]->data_channel_s()->Send(eio::PacketData(message.ToJsonString()));
        }
        
        peer_conns_[dest_user]->data_channel_s()->Send(eio::PacketData(end_message.ToJsonString()));
    }
    
    void Easyrtc::SendDataP2P(const std::string & dest_user,
                              const std::string & msg_type,
                              const Any & msg_data)
    {
        std::string flattened_data = Any(Any::ObjectType {
            { "msgType", Any(msg_type) }, { "msgData", msg_data }
        }).ToJsonString();
        
        FuncCall(debug_printer_,
                 nwr::Format("sending p2p message to %s with data=%s",
                             dest_user.c_str(), flattened_data.c_str()));

        if (!HasKey(peer_conns_, dest_user)) {
            ShowError(err_codes_DEVELOPER_ERR_,
                      std::string("Attempt to send data peer to peer without a connection to ") + dest_user + " first.");
        }
        else if (!peer_conns_[dest_user]->data_channel_s()) {
            
            ShowError(err_codes_DEVELOPER_ERR_,
                      std::string("Attempt to send data peer to peer without establishing a data channel to ") + dest_user + " first.");
        }
        else if (!peer_conns_[dest_user]->data_channel_ready()) {
            ShowError(err_codes_DEVELOPER_ERR_,
                      std::string("Attempt to use data channel to ") +
                      dest_user + " before it's ready to send.");
        }
        else {
            if (flattened_data.length() > max_p2p_message_length_) {
                SendByChunkHelper(dest_user, flattened_data);
            } else {
                peer_conns_[dest_user]->data_channel_s()->Send(eio::PacketData(flattened_data));
            }
        }
    }
    
    void Easyrtc::SendDataWS(const Any & destination,
                             const std::string & msg_type,
                             const Any & msg_data,
                             const std::function<void(const Any &)> & arg_ack_handler)
    {
        auto thiz = shared_from_this();
        
        std::function<void(const Any &)> ack_handler = [arg_ack_handler](const Any & a) {
            FuncCall(arg_ack_handler, a);
        };
        
        FuncCall(debug_printer_,
                 std::string("sending client message via websockets to ") + destination.ToJsonString() +
                 " with data=" + msg_data.ToJsonString());
        
        if (!ack_handler) {
            ack_handler = [thiz](const Any & msg) {
                if (msg.GetAt("msgType").AsString() == Some(std::string("error"))) {
                    thiz->ShowError(msg.GetAt("msgData").GetAt("errorCode").AsString() || std::string(),
                                    msg.GetAt("msgData").GetAt("errorText").AsString() || std::string());
                }
            };
        }
        
        Any outgoing_message(Any::ObjectType {
            { "msgType", Any(msg_type) },
            { "msgData", msg_data }
        });
        
        if (destination) {
            if (destination.type() == Any::Type::String) {
                outgoing_message.SetAt("targetEasyrtcid", destination);
            }
            else if (destination.type() == Any::Type::Object) {
                if (destination.GetAt("targetEasyrtcid")) {
                    outgoing_message.SetAt("targetEasyrtcid", destination.GetAt("targetEasyrtcid"));
                }
                if (destination.GetAt("targetRoom")) {
                    outgoing_message.SetAt("targetRoom", destination.GetAt("targetRoom"));
                }
                if (destination.GetAt("targetGroup")) {
                    outgoing_message.SetAt("targetGroup", destination.GetAt("targetGroup"));
                }
            }
        }
        
        if (websocket_) {
            websocket_->JsonEmit("easyrtcMsg", { outgoing_message, AnyFuncMake(ack_handler) });
        }
        else {
            FuncCall(debug_printer_,
                     "websocket failed because no connection to server");
            Fatal("Attempt to send message without a valid connection to the server.");
        }
    }
    
    void Easyrtc::SendData(const std::string & dest_user,
                           const std::string & msg_type,
                           const Any & msg_data,
                           const std::function<void(const Any &)> & ack_handler)
    {
        if (HasKey(peer_conns_, dest_user) && peer_conns_[dest_user]->data_channel_ready()) {
            SendDataP2P(dest_user, msg_type, msg_data);
        }
        else {
            SendDataWS(Any(dest_user), msg_type, msg_data, ack_handler);
        }
    }
    
    void Easyrtc::SendPeerMessage(const Any & destination,
                                  const std::string & msg_type,
                                  const Any & msg_data,
                                  const std::function<void(const std::string &,
                                                           const Any &)> & success_cb,
                                  const std::function<void(const std::string &,
                                                           const std::string &)> & failure_cb)
    {
        if (!destination) {
            printf("Developer error, destination was null in sendPeerMessage\n");
        }

        FuncCall(debug_printer_, std::string("sending peer message ") + msg_data.ToJsonString());

        auto ack_handler = [success_cb, failure_cb](const Any & response) {
            if (response.GetAt("msgType").AsString() == Some(std::string("error"))) {
                FuncCall(failure_cb,
                         response.GetAt("msgData").GetAt("errorCode").AsString() || std::string(),
                         response.GetAt("msgData").GetAt("errorText").AsString() || std::string());
            }
            else {
                FuncCall(success_cb,
                         response.GetAt("msgType").AsString() || std::string(),
                         response.GetAt("msgData"));
            }
        };
        
        SendDataWS(destination, msg_type, msg_data, ack_handler);
    }
    
    void Easyrtc::SendServerMessage(const std::string & msg_type,
                                    const Any & msg_data,
                                    const std::function<void(const std::string &,
                                                             const Any &)> & success_cb,
                                    const std::function<void(const std::string &,
                                                             const std::string &)> & failure_cb)
    {
        if (debug_printer_) {
            Any data_to_ship(Any::ObjectType {
                { "msgType", Any(msg_type) },
                { "msgData", msg_data }
            });
            FuncCall(debug_printer_, std::string("sending server message ") + data_to_ship.ToJsonString());
        }
        
        auto ack_handler = [success_cb, failure_cb](const Any & response){
            if (response.GetAt("msgType").AsString() == Some(std::string("error"))) {
                FuncCall(failure_cb,
                         response.GetAt("msgData").GetAt("errorCode").AsString() || std::string(),
                         response.GetAt("msgData").GetAt("errorText").AsString() || std::string());
            }
            else {
                FuncCall(success_cb,
                         response.GetAt("msgType").AsString() || std::string(),
                         response.GetAt("msgData"));
            }
        };
        
        SendDataWS(nullptr, msg_type, msg_data, ack_handler);
    }
    
    void Easyrtc::GetRoomList(const std::function<void(const Any &)> & callback,
                              const std::function<void(const std::string &,
                                                       const std::string &)> & error_callback)
    {
        auto thiz = shared_from_this();
        
        SendSignaling(None(), "getRoomList", nullptr,
                      [callback](const std::string & msg_type, const Any & msg_data) {
                          FuncCall(callback, msg_data.GetAt("roomList"));
                      },
                      [thiz, error_callback](const std::string & error_code, const std::string & error_text){
                          if (error_callback) {
                              FuncCall(error_callback, error_code, error_text);
                          }
                          else {
                              thiz->ShowError(error_code, error_text);
                          }
                      });
    }
    
    Easyrtc::ConnectStatus Easyrtc::GetConnectStatus(const std::string & other_user) {
        if (!HasKey(peer_conns_, other_user)) {
            return ConnectStatus::NotConnected;
        }
        auto peer = peer_conns_[other_user];
        
        if ((peer->sharing_audio() || peer->sharing_video()) && !peer->started_av()) {
            return ConnectStatus::BecomingConnected;
        }
        else if (peer->sharing_data() && !peer->data_channel_ready()) {
            return ConnectStatus::BecomingConnected;
        }
        else {
            return ConnectStatus::IsConnected;
        }
    }
    
    std::shared_ptr<MediaTrackConstraints> Easyrtc::BuildPeerConstraints() {
        auto consts = std::make_shared<MediaTrackConstraints>();
        
        consts->optional().SetEntry(webrtc::MediaConstraintsInterface::kEnableDtlsSrtp, true); // for interoperability
    
        return consts;
    }
    
    void Easyrtc::Call(const std::string & other_user,
                       const Optional<std::vector<std::string>> & stream_names,
                       const std::function<void(const std::string &,
                                                const std::string &)> & call_success_cb,
                       const std::function<void(const std::string &,
                                                const std::string &)> & call_failure_cb,
                       const std::function<void(bool, const std::string &)> & was_accepted_cb)
    {
        auto thiz = shared_from_this();
        
        FuncCall(debug_printer_,
                 nwr::Format("initiating peer to peer call to %s audio=%d video=%d data=%d",
                             other_user.c_str(), audio_enabled_, video_enabled_, data_enabled_));

        if (!SupportsPeerConnections()) {
            FuncCall(call_failure_cb,
                     err_codes_CALL_ERR_,
                     GetConstantString("noWebrtcSupport"));
            return;
        }
        
        //
        // If we are sharing audio/video and we haven't allocated the local media stream yet,
        // we'll do so, recalling our self on success.
        //
        
        
        if (!stream_names && auto_init_user_media_) {
            auto stream = GetLocalStream(None());
            if (!stream && (audio_enabled_ || video_enabled_)) {
                
                InitMediaSource(None(),
                                [thiz, other_user, call_success_cb, call_failure_cb, was_accepted_cb]
                                (const std::shared_ptr<MediaStream> & stream){
                                    thiz->Call(other_user,
                                               None(),
                                               call_success_cb,
                                               call_failure_cb,
                                               was_accepted_cb);
                                },
                                call_failure_cb);
                return;
            }
        }
        
        if (!websocket_) {
            std::string message("Attempt to make a call prior to connecting to service");
            FuncCall(debug_printer_, message);
            Fatal(message);
        }
        
        //
        // If B calls A, and then A calls B before accepting, then A should treat the attempt to
        // call B as a positive offer to B's offer.
        //
        if (HasKey(offers_pending_, other_user)) {
            FuncCall(was_accepted_cb, true, other_user);
            DoAnswer(other_user, offers_pending_[other_user], stream_names);
            offers_pending_.erase(other_user);
            FuncCall(call_canceled_, other_user, false);
            return;
        }
        
        // do we already have a pending call?
        if (HasKey(acceptance_pending_, other_user)) {
            std::string message("Call already pending acceptance");
            FuncCall(debug_printer_, message);
            FuncCall(call_failure_cb, err_codes_ALREADY_CONNECTED_, message);
            return;
        }
        
        if (use_fresh_ice_each_peer_) {
            GetFreshIceConfig([thiz, other_user,
                               call_success_cb, call_failure_cb,
                               was_accepted_cb,
                               stream_names]
                              (bool succeeded) {
                                  if (succeeded) {
                                      thiz->CallBody(other_user, call_success_cb, call_failure_cb, was_accepted_cb, stream_names);
                                  }
                                  else {
                                      FuncCall(call_failure_cb,
                                               thiz->err_codes_CALL_ERR_,
                                               "Attempt to get fresh ice configuration failed");
                                  }
                              });
        }
        else {
            CallBody(other_user, call_success_cb, call_failure_cb, was_accepted_cb, stream_names);
        }
    }
    
    void Easyrtc::CallBody(const std::string & other_user,
                           const std::function<void(const std::string &,
                                                    const std::string &)> & call_success_cb,
                           const std::function<void(const std::string &,
                                                    const std::string &)> & call_failure_cb,
                           const std::function<void(bool, const std::string &)> & was_accepted_cb,
                           const Optional<std::vector<std::string>> & stream_names)
    {
        auto thiz = shared_from_this();
        
        acceptance_pending_[other_user] = true;
        
        auto pc = BuildPeerConnection(other_user, true, call_failure_cb, stream_names);

        if (!pc) {
            std::string message("buildPeerConnection failed, call not completed");
            FuncCall(debug_printer_, message);
            Fatal(message);
        }
        
        peer_conns_[other_user]->set_call_success_cb(call_success_cb);
        peer_conns_[other_user]->set_call_failure_cb(call_failure_cb);
        peer_conns_[other_user]->set_was_accepted_cb(was_accepted_cb);
        
        auto peer_conn_obj = peer_conns_[other_user];
        
        auto set_local_and_send_message_0 =
        [thiz, pc, peer_conn_obj, other_user, call_failure_cb]
        (const std::shared_ptr<RtcSessionDescription> & session_description) {
            if (peer_conn_obj->canceled()) {
                return;
            }
            
            auto send_offer = [thiz, other_user, session_description, call_failure_cb]() {
                thiz->SendSignaling(Some(other_user),
                                    "offer",
                                    session_description->ToAny(),
                                    nullptr,
                                    call_failure_cb);
            };
            
            if (thiz->sdp_local_filter_) {
                session_description->set_sdp(thiz->sdp_local_filter_(session_description->sdp()));
            }
            
            pc->SetLocalDescription(session_description,
                                    send_offer,
                                    [thiz, call_failure_cb](const std::string & error_text) {
                                        call_failure_cb(thiz->err_codes_CALL_ERR_, error_text);
                                    });
        };
        
        Timer::Create(TimeDuration(0.1),
                      [thiz, other_user, pc, set_local_and_send_message_0, call_failure_cb]() {
                          //
                          // if the call was cancelled, we don't want to continue getting the offer.
                          // we can tell the call was cancelled because there won't be a peerConn object
                          // for it.
                          //
                          if (!thiz->peer_conns_[other_user]) {
                              return;
                          }
                          
                          pc->CreateOffer(thiz->received_media_constraints_,
                                          set_local_and_send_message_0,
                                          [thiz, call_failure_cb](const std::string & error){
                                              FuncCall(call_failure_cb, thiz->err_codes_CALL_ERR_, error);
                                          });
                      });
    }
    
    void Easyrtc::HangupBody(const std::string & other_user) {
        FuncCall(debug_printer_, std::string("Hanging up on " + other_user));
        
        ClearQueuedMessages(other_user);
        
        if (HasKey(peer_conns_, other_user)) {
            if (peer_conns_[other_user]->pc()) {
                auto remote_streams = peer_conns_[other_user]->pc()->remote_streams();
                for (int i = 0; i < remote_streams.size(); i++) {
                    if (remote_streams[i]->active()) {
                        EmitOnStreamClosed(other_user, remote_streams[i]);
                        StopStream(remote_streams[i]);
                    }
                }
                //
                // todo: may need to add a few lines here for closing the data channels
                //
                peer_conns_[other_user]->pc()->Close();
            }
            
            peer_conns_[other_user]->set_canceled(true);
            peer_conns_.erase(other_user);

            if (websocket_) {
                auto thiz = shared_from_this();
                
                SendSignaling(Some(other_user),
                              "hangup",
                              nullptr,
                              nullptr,
                              [thiz](const std::string & error_code, const std::string & error_text) {
                                  FuncCall(thiz->debug_printer_,
                                           std::string("hangup failed:" + error_text));
                              });
            }
            
            if (acceptance_pending_[other_user]) {
                acceptance_pending_.erase(other_user);
            }
        }
    }
    
    void Easyrtc::Hangup(const std::string & other_user) {
        HangupBody(other_user);
        UpdateConfigurationInfo();
    }
    
    void Easyrtc::HangupAll() {
        bool saw_a_connection = false;
        for (const auto & other_user : Keys(peer_conns_)) {
            saw_a_connection = true;
            HangupBody(other_user);
        }
        
        if (saw_a_connection) {
            UpdateConfigurationInfo();
        }
    }
    
    bool Easyrtc::DoesDataChannelWork(const std::string & other_user) {
        if (HasKey(peer_conns_, other_user)) {
            return false;
        }
        return peer_conns_[other_user]->data_channel_ready();
    }
    
    std::shared_ptr<MediaStream>
    Easyrtc::GetRemoteStream(const std::string & easyrtcid,
                             const std::string & remote_stream_name)
    {
        if (!HasKey(peer_conns_, easyrtcid)) {
            ShowError(err_codes_DEVELOPER_ERR_,
                      std::string("attempt to get stream of uncalled party"));
            Fatal("Developer err: no such stream");
        }
        else {
            return peer_conns_[easyrtcid]->GetRemoteStreamByName(*this, Some(remote_stream_name));
        }
    }
    
    void Easyrtc::MakeLocalStreamFromRemoteStream(const std::string & easyrtcid,
                                                  const std::string & remote_stream_name,
                                                  const std::string & local_stream_name)
    {
        if (peer_conns_[easyrtcid]->pc()) {
            auto remote_stream = peer_conns_[easyrtcid]->GetRemoteStreamByName(*this, Some(remote_stream_name));
            if (remote_stream) {
                RegisterLocalMediaStreamByName(remote_stream,
                                               Some(local_stream_name));
            }
            else {
                Fatal("Developer err: no such stream");
            }
        }
        else {
            Fatal("Developer err: no such peer ");
        }
    }
    
    void Easyrtc::AddStreamToCall(const std::string & easyrtcid,
                                  const Optional<std::string> & arg_stream_name,
                                  const std::function<void(const std::string &,
                                                           const std::string &)> & receipt_handler)
    {
        std::string stream_name = arg_stream_name || std::string("default");

        auto stream = GetLocalMediaStreamByName(Some(stream_name));
        if (!stream) {
            printf("attempt to add nonexistent stream %s\n", stream_name.c_str());
        }
        else if (!HasKey(peer_conns_, easyrtcid) || !peer_conns_[easyrtcid]->pc()) {
            printf("Can't add stream before a call has started.\n");
        }
        else {
            auto pc = peer_conns_[easyrtcid]->pc();
            peer_conns_[easyrtcid]->set_enable_negotiate_listener(true);
            pc->AddStream(stream);
            
            if (receipt_handler) {
                peer_conns_[easyrtcid]->streams_added_acks()[stream_name] = receipt_handler;
            }
        }
    }
    
    void Easyrtc::SetupPeerListener1() {
        auto thiz = shared_from_this();
        auto func = [thiz](const std::string & easyrtcid,
                           const std::string & msg_type,
                           const Any & msg_data,
                           const Any & targeting)
        {
            if (!HasKey(thiz->peer_conns_, easyrtcid) || !thiz->peer_conns_[easyrtcid]->pc()) {
                thiz->ShowError(thiz->err_codes_DEVELOPER_ERR_,
                                "Attempt to add additional stream before establishing the base call.");
            }
            else {
                auto sdp = msg_data.GetAt("sdp");
                auto pc = thiz->peer_conns_[easyrtcid]->pc();
                
                auto set_local_and_send_message_1 = [thiz, easyrtcid, pc](const std::shared_ptr<RtcSessionDescription> & session_description){
                    
                    auto send_answer = [thiz, easyrtcid, session_description]() {
                        FuncCall(thiz->debug_printer_, "sending answer");
                        
                        auto on_signal_success = [](const std::string & msg_type, const Any & msg_data){};
                        auto on_signal_failure = [thiz, easyrtcid]
                        (const std::string & error_code,
                         const std::string & error_text)
                        {
                            thiz->peer_conns_.erase(easyrtcid);
                            thiz->ShowError(error_code, error_text);
                        };
                        
                        thiz->SendSignaling(Some(easyrtcid),
                                            "answer",
                                            session_description->ToAny(),
                                            on_signal_success,
                                            on_signal_failure);
                        
                        thiz->peer_conns_[easyrtcid]->set_connection_accepted(true);
                        
                        thiz->SendQueuedCandidates(easyrtcid, on_signal_success, on_signal_failure);
                    };
                    
                    if (thiz->sdp_local_filter_) {
                        session_description->set_sdp(thiz->sdp_local_filter_(session_description->sdp()));
                    }
                  
                    pc->SetLocalDescription(session_description,
                                            send_answer,
                                            [thiz](const std::string & message){
                                                thiz->ShowError(thiz->err_codes_INTERNAL_ERR_,
                                                                std::string("setLocalDescription: " + message));
                                            });
                };
                
                auto invoke_create_answer = [thiz, pc, easyrtcid, sdp, set_local_and_send_message_1]() {
                    pc->CreateAnswer(thiz->received_media_constraints_,
                                     set_local_and_send_message_1,
                                     [thiz](const std::string & message){
                                         thiz->ShowError(thiz->err_codes_INTERNAL_ERR_,
                                                         std::string("create-answer: " + message));
                                     });
                    thiz->SendPeerMessage(Any(easyrtcid),
                                          "__gotAddedMediaStream",
                                          Any(Any::ObjectType
                                              {
                                                  { "sdp", sdp }
                                              }),
                                          nullptr, nullptr);
                };
                
                FuncCall(thiz->debug_printer_, "about to call setRemoteDescription in doAnswer");
                
                if (thiz->sdp_remote_filter_) {
                    std::string sdp_str = sdp.GetAt("sdp").AsString() || std::string();
                    sdp_str = thiz->sdp_remote_filter_(sdp_str);
                    sdp.SetAt("sdp", Any(sdp_str));
                    
                    pc->SetRemoteDescription(RtcSessionDescription::FromAny(sdp),
                                             invoke_create_answer,
                                             [thiz](const std::string & message){
                                                 thiz->ShowError(thiz->err_codes_INTERNAL_ERR_,
                                                                 std::string("set-remote-description: " + message));
                                             });
                }
            }
        };
        
        SetPeerListener(Some(std::string("__addedMediaStream")), None(), func);
    }
    
    void Easyrtc::SetupPeerListener2() {
        auto thiz = shared_from_this();
        auto func = [thiz](const std::string & easyrtcid,
                           const std::string & msg_type,
                           const Any & msg_data,
                           const Any & targeting)
        {
            if (!HasKey(thiz->peer_conns_, easyrtcid) || !thiz->peer_conns_[easyrtcid]->pc()) {
            }
            else {
                auto sdp = msg_data.GetAt("sdp");
                
                if (thiz->sdp_remote_filter_) {
                    std::string sdp_str = sdp.GetAt("sdp").AsString() || std::string();
                    sdp_str = thiz->sdp_remote_filter_(sdp_str);
                    sdp.SetAt("sdp", Any(sdp_str));
                }

                auto pc = thiz->peer_conns_[easyrtcid]->pc();
                
                pc->SetRemoteDescription(RtcSessionDescription::FromAny(sdp),
                                         nullptr,
                                         [thiz](const std::string & message){
                                             thiz->ShowError(thiz->err_codes_INTERNAL_ERR_,
                                                             std::string("set-remote-description: " + message));
                                         });
            }
        };
        
        SetPeerListener(Some(std::string("__gotAddedMediaStream")), None(), func);
    }
    
    void Easyrtc::SetupPeerListener3() {
        auto thiz = shared_from_this();
        
        auto func = [thiz](const std::string & easyrtcid,
                           const std::string & msg_type,
                           const Any & msg_data,
                           const Any & targeting)
        {
            if (!HasKey(thiz->peer_conns_, easyrtcid) || !thiz->peer_conns_[easyrtcid]->pc()) {
            }
            else {
                auto stream = thiz->peer_conns_[easyrtcid]->GetRemoteStreamByName(*thiz,
                                                                                  msg_data.GetAt("streamName").AsString());
                if (stream) {
                    thiz->OnRemoveStreamHelper(easyrtcid, stream);
                    thiz->StopStream(stream);
                }
            }
        };
        
        SetPeerListener(Some(std::string("__closingMediaStream")), None(), func);
    }
    
    void Easyrtc::OnRemoveStreamHelper(const std::string & easyrtcid,
                                       const std::shared_ptr<MediaStream> & stream)
    {
        if (HasKey(peer_conns_, easyrtcid)) {
            EmitOnStreamClosed(easyrtcid, stream);
            UpdateConfigurationInfo();
            if (peer_conns_[easyrtcid]->pc()) {
                peer_conns_[easyrtcid]->pc()->RemoveStream(stream);
            }
        }
    }
    
    void Easyrtc::DumpPeerConnectionInfo() {
        for (const auto & peer : Keys(peer_conns_)) {
            printf("For peer %s\n", peer.c_str());
            auto pc = peer_conns_[peer]->pc();
            auto remotes = pc->remote_streams();
            std::vector<std::string> remote_ids;
            for (int i = 0; i < remotes.size(); i++) {
                remote_ids.push_back(remotes[i]->id());
            }
            auto locals = pc->local_streams();
            std::vector<std::string> local_ids;
            for (int i = 0; i < locals.size(); i++) {
                local_ids.push_back(locals[i]->id());
            }
            
            for (const auto & id : remote_ids) {
                printf("    remote: %s\n", id.c_str());
            }
            for (const auto & id : local_ids) {
                printf("    local: %s\n", id.c_str());
            }
        }
    }
    
    std::shared_ptr<RtcPeerConnection>
    Easyrtc::BuildPeerConnection(const std::string & other_user,
                                 bool is_initiator,
                                 const std::function<void(const std::string &,
                                                          const std::string &)> & failure_cb,
                                 const Optional<std::vector<std::string>> & stream_names)
    {
        auto thiz = shared_from_this();
        
        std::shared_ptr<webrtc::PeerConnectionInterface::RTCConfiguration> ice_config =
        pc_config_to_use_ ? pc_config_to_use_ : pc_config_;
        FuncCall(debug_printer_, std::string("building peer connection to ") + other_user);
        
        //
        // we don't support data channels on chrome versions < 31
        //
        
        auto consts = BuildPeerConstraints();
        auto pc = CreateRtcPeerConnection(*ice_config, consts);
        if (!pc) {
            std::string message("Unable to create PeerConnection object, check your ice configuration");
            //                JSON.stringify(ice_config)
            FuncCall(debug_printer_, message);
            Fatal(message);
        }
        
        pc->set_on_negotiation_needed([thiz, other_user, pc](){
            if (thiz->peer_conns_[other_user]->enable_negotiate_listener()) {
                pc->CreateOffer(nullptr,
                                [thiz, other_user, pc](const std::shared_ptr<RtcSessionDescription> & sdp){
                                    if (thiz->sdp_local_filter_) {
                                        sdp->set_sdp(thiz->sdp_local_filter_(sdp->sdp()));
                                    }
                                    
                                    pc->SetLocalDescription(sdp,
                                                            [thiz, other_user, sdp](){
                                                                thiz->SendPeerMessage(Any(other_user),
                                                                                      "__addedMediaStream",
                                                                                      Any(Any::ObjectType
                                                                                          {
                                                                                              { "sdp", sdp->ToAny() }
                                                                                          }),
                                                                                      nullptr,
                                                                                      nullptr);
                                                            },
                                                            [](const std::string & message){
                                                                printf("unexpected failure: %s\n", message.c_str());
                                                            });
                                },
                                [](const std::string & error){
                                    printf("unexpected error in creating offer; %s\n", error.c_str());
                                });
            }
        });
        
        pc->set_on_ice_connection_state_change([thiz, other_user, failure_cb](webrtc::PeerConnectionInterface::IceConnectionState conn_state){
            switch (conn_state) {
                case webrtc::PeerConnectionInterface::kIceConnectionConnected: {
                    if (thiz->peer_conns_[other_user]->call_success_cb()) {
                        thiz->peer_conns_[other_user]->call_success_cb()(other_user, "connection");
                    }
                    break;
                }
                case webrtc::PeerConnectionInterface::kIceConnectionFailed: {
                    FuncCall(failure_cb, thiz->err_codes_NOVIABLEICE_, "No usable STUN/TURN path");
                    thiz->DeletePeerConn(other_user);
                    break;
                }
                case webrtc::PeerConnectionInterface::kIceConnectionDisconnected: {
                    if (thiz->on_peer_failing_) {
                        thiz->on_peer_failing_(other_user);
                        thiz->peer_conns_[other_user]->set_failing(Some(std::chrono::system_clock::now()));
                    }
                    break;
                }
                case webrtc::PeerConnectionInterface::kIceConnectionClosed: {
                    if (thiz->on_peer_closed_) {
                        thiz->on_peer_closed_(other_user);
                    }
                    break;
                }
                default:
                    break;
            }
            
            if (conn_state == webrtc::PeerConnectionInterface::kIceConnectionConnected ||
                conn_state == webrtc::PeerConnectionInterface::kIceConnectionCompleted)
            {
                if (thiz->peer_conns_[other_user]->failing() && thiz->on_peer_recovered_) {
                    thiz->on_peer_recovered_(other_user,
                                             thiz->peer_conns_[other_user]->failing().value(),
                                             std::chrono::system_clock::now());
                }
                
                thiz->peer_conns_[other_user]->set_failing(None());
            }
        });
        
        auto new_peer_conn = std::make_shared<PeerConn>(other_user);
        new_peer_conn->set_pc(pc);
        new_peer_conn->candidates_to_send().clear();
        new_peer_conn->set_started_av(false);
        new_peer_conn->set_connection_accepted(false);
        new_peer_conn->set_is_initiator(is_initiator);
        new_peer_conn->remote_stream_id_to_name().clear();
        new_peer_conn->streams_added_acks().clear();
        new_peer_conn->live_remote_streams().clear();
        
        pc->set_on_ice_candidate([thiz, new_peer_conn, other_user, failure_cb](const std::shared_ptr<RtcIceCandidate> & candidate){
            if (new_peer_conn->canceled()) {
                return;
            }
            
            if (thiz->peer_conns_[other_user]) {
                Any candidate_data(Any::ObjectType {
                    { "type", Any("candidate") },
                    { "label", Any(candidate->sdp_mline_index()) },
                    { "id", Any(candidate->sdp_mid()) },
                    { "candidate", Any(candidate->candidate())  }
                } );
                                
                if (thiz->ice_candidate_filter_) {
                    candidate_data = thiz->ice_candidate_filter_(candidate_data, false);
                    if( !candidate_data ) {
                        return;
                    }
                }
                //
                // some candidates include ip addresses of turn servers. we'll want those
                // later so we can see if our actual connection uses a turn server.
                // The keyword "relay" in the candidate identifies it as referencing a
                // turn server. The \d symbol in the regular expression matches a number.
                //
                if (IndexOf(candidate->candidate(), "typ relay") != -1) {
                    std::regex regex("(udp|tcp) \\d+ (\\d+\\.\\d+\\.\\d+\\.\\d+)", std::regex_constants::icase);
                    std::smatch match_ret;
                    std::string candidate_str = candidate->candidate();
                    if (std::regex_search(candidate_str, match_ret, regex)) {
                        std::string ip_address = match_ret[2].str();
                        thiz->turn_servers_[ip_address] = true;
                    } else {
                        printf("ip address match failed [%s]\n", candidate_str.c_str());
                    }
                }
                
                if (thiz->peer_conns_[other_user]->connection_accepted()) {
                    
                    thiz->SendSignaling(Some(other_user), "candidate", candidate_data,
                                        nullptr,
                                        [thiz, failure_cb](const std::string & code, const std::string & text) {
                                            FuncCall(failure_cb,
                                                     thiz->err_codes_PEER_GONE_,
                                                     nwr::Format("Candidate disappeared (code=%s, text=%s)", code.c_str(), text.c_str()));
                                        });
                }
                else {
                    thiz->peer_conns_[other_user]->candidates_to_send().push_back(candidate_data);
                }
            }
        });
        
        pc->set_on_add_stream([thiz, other_user, new_peer_conn](const std::shared_ptr<MediaStream> & stream){
            FuncCall(thiz->debug_printer_, "saw incoming media stream");

            if (new_peer_conn->canceled()) {
                return;
            }

            const auto & peer_conn = thiz->peer_conns_[other_user];
            
            if (!peer_conn->started_av()) {
                peer_conn->set_started_av(true);
                peer_conn->set_sharing_audio(thiz->have_audio_);
                peer_conn->set_sharing_video(thiz->have_video_);
                peer_conn->set_connect_time(Some(std::chrono::system_clock::now()));
                if (peer_conn->call_success_cb()) {
                    if (peer_conn->sharing_audio() || peer_conn->sharing_video()) {
                        peer_conn->call_success_cb()(other_user, "audiovideo");
                    }
                }
                if (thiz->audio_enabled_ || thiz->video_enabled_) {
                    thiz->UpdateConfiguration();
                }
            }
            
            
            auto remote_name_opt = thiz->GetNameOfRemoteStream(other_user, Some(stream->id()));
            std::string remote_name = remote_name_opt || std::string("default");

            peer_conn->remote_stream_id_to_name()[stream->id()] = remote_name;
            peer_conn->live_remote_streams()[remote_name] = true;
//            event.stream.streamName = remoteName;
            if (thiz->stream_acceptor_) {
                thiz->stream_acceptor_(other_user, stream, remote_name);
                //
                // Inform the other user that the stream they provided has been received.
                // This should be moved into signalling at some point
                //
                thiz->SendDataWS(Any(other_user), "easyrtc_streamReceived",
                                 Any(Any::ObjectType
                                     {
                                         { "streamName", Any(remote_name) }
                                     }),
                                 nullptr);
            }
        });
        
        pc->set_on_remove_stream([thiz, other_user](const std::shared_ptr<MediaStream> & stream) {
            FuncCall(thiz->debug_printer_, "saw remove on remote media stream");
            
            thiz->OnRemoveStreamHelper(other_user, stream);
        });
        
        SetPeerConn(other_user, new_peer_conn);

        std::shared_ptr<MediaStream> stream = nullptr;
        if (stream_names) {
            for (const auto & stream_name : stream_names.value()) {
                stream = GetLocalMediaStreamByName(Some(stream_name));
                if (stream) {
                    pc->AddStream(stream);
                }
                else {
                    printf("Developer error, attempt to access unknown local media stream %s", stream_name.c_str());
                }
            }
        }
        else if (auto_init_user_media_ && (video_enabled_ || audio_enabled_)) {
            stream = GetLocalStream(None());
            pc->AddStream(stream);
        }
        
        //
        // This function handles data channel message events.
        //
        
        std::shared_ptr<Any> pending_transfer_ptr = std::make_shared<Any>();
        
        auto data_channel_message_handler = [thiz, other_user, pending_transfer_ptr](const eio::PacketData & data) {
            std::string data_str(data.char_ptr(), data.size());
            FuncCall(thiz->debug_printer_, std::string("saw dataChannel.onmessage event: ") + data_str.c_str());

            if (data_str == "dataChannelPrimed") {
                thiz->SendDataWS(Any(other_user), "dataChannelPrimed", Any(""), nullptr);
            }
            else {
                //
                // Chrome and Firefox Interop is passing a event with a strange data="", perhaps
                // as it's own form of priming message. Comparing the data against "" doesn't
                // work, so I'm going with parsing and trapping the parse error.
                //
                
                auto msg = Any::FromJsonString(data_str);
                if (msg) {
                    Optional<std::string> transfer_opt = msg.GetAt("transfer").AsString();
                    Optional<std::string> transfer_id_opt = msg.GetAt("transferId").AsString();
                    if (transfer_opt && transfer_id_opt) {
                        std::string transfer = *transfer_opt;
                        std::string transfer_id = *transfer_id_opt;
                        if (transfer == "start") {
                            FuncCall(thiz->debug_printer_, std::string("start transfer #") + transfer_id);
                            
                            int parts = msg.GetAt("parts").AsInt().value();
                            
                            *pending_transfer_ptr = Any(Any::ObjectType {
                                { "chunks", Any(Any::ArrayType{}) },
                                { "parts", Any(parts) },
                                { "transferId", Any(transfer_id) }
                            });
                        } else if (transfer == "chunk") {
                            FuncCall(thiz->debug_printer_, std::string("got chunk for tranfer #") + transfer_id);
                            
                            // check data is valid
                            auto data_opt = msg.GetAt("data").AsString();
                            if (!(data_opt && data_opt->length() <= thiz->max_p2p_message_length_)) {
                                
                                printf("Developer error, invalid data\n");
                                
                                // check there's a pending transfer
                            } else if (!*pending_transfer_ptr) {
                                printf("Developer error, unexpected chunk\n");
                                
                                // check that transferId is valid
                            } else if (transfer_id != pending_transfer_ptr->GetAt("transferId").AsString().value()) {
                                printf("Developer error, invalid transfer id\n");
                                
                                // check that the max length of transfer is not reached
                            } else if (pending_transfer_ptr->GetAt("chunks").AsArray()->size() + 1 > pending_transfer_ptr->GetAt("parts").AsInt().value()) {
                                
                                printf("Developer error, received too many chunks");
                                
                            } else {
                                pending_transfer_ptr->GetAt("chunks").AsArray()->push_back(Any(data_opt.value()));
                            }
                            
                        } else if (transfer == "end") {
                            FuncCall(thiz->debug_printer_, std::string("end of transfer #") + transfer_id);
                            
                            // check there's a pending transfer
                            if (!*pending_transfer_ptr) {
                                
                                printf("Developer error, unexpected end of transfer\n");
                                
                                // check that transferId is valid
                            } else if (transfer_id != pending_transfer_ptr->GetAt("transferId").AsString().value()) {
                                printf("Developer error, invalid transfer id");
                                
                                // check that all the chunks were received
                            } else if (pending_transfer_ptr->GetAt("chunks").AsArray()->size() != pending_transfer_ptr->GetAt("parts").AsInt().value()) {
                                printf("Developer error, received wrong number of chunks");
                                
                            } else {
                                
                                std::vector<std::string> chunks = Map(pending_transfer_ptr->GetAt("chunks").AsArray().value(),
                                                                      [](const Any & value) -> std::string {
                                                                          return value.AsString().value();
                                                                      });
                                Any chunked_msg = Any::FromJsonString(Join(chunks));
                                if (!chunked_msg) {
                                    printf("Developer error, unable to parse message\n");
                                } else {
                                    thiz->ReceivePeerDistribute(other_user, chunked_msg, nullptr);
                                }
                            }
                            
                            *pending_transfer_ptr = Any();
                            
                        } else {
                            printf("Developer error, got an unknown transfer message %s\n", transfer.c_str());
                        }
                    } else {
                        thiz->ReceivePeerDistribute(other_user, msg, nullptr);
                    }
                } else {
                    printf("[data channel message handler] parse failed\n");
                }
                
            }
        };

        auto init_out_going_channel = [thiz, other_user, pc, data_channel_message_handler](const std::string & other_user){
            FuncCall(thiz->debug_printer_, "saw initOutgoingChannel call");
            
            webrtc::DataChannelInit data_channel_config = thiz->GetDataChannelConstraints();
            auto data_channel = pc->CreateDataChannel(thiz->data_channel_name_, &data_channel_config);
            
            thiz->peer_conns_[other_user]->set_data_channel_s(data_channel);
            thiz->peer_conns_[other_user]->set_data_channel_r(data_channel);
            data_channel->set_on_message(data_channel_message_handler);
            data_channel->set_on_open([thiz, other_user, data_channel](){
                FuncCall(thiz->debug_printer_, "saw dataChannel.onopen event");
                
                if (HasKey(thiz->peer_conns_, other_user)) {
                    data_channel->Send(eio::PacketData("dataChannelPrimed"));
                }
            });
            data_channel->set_on_close([thiz, other_user](){
                FuncCall(thiz->debug_printer_, "saw dataChannelS.onclose event");

                if (HasKey(thiz->peer_conns_, other_user)) {
                    thiz->peer_conns_[other_user]->set_data_channel_ready(false);
                    thiz->peer_conns_[other_user]->set_data_channel_s(nullptr);
                }
                
                FuncCall(thiz->on_data_channel_close_, other_user);
                
                thiz->UpdateConfigurationInfo();
            });
        };

        auto init_incoming_channel = [thiz, other_user, data_channel_message_handler](const std::string & other_user) {
            FuncCall(thiz->debug_printer_, std::string("initializing incoming channel handler for ") + other_user);
            
            thiz->peer_conns_[other_user]->pc()->set_on_data_channel([thiz, other_user, data_channel_message_handler](const std::shared_ptr<RtcDataChannel> & data_channel) {
                FuncCall(thiz->debug_printer_, "saw incoming data channel");
                
                thiz->peer_conns_[other_user]->set_data_channel_r(data_channel);
                thiz->peer_conns_[other_user]->set_data_channel_s(data_channel);
                thiz->peer_conns_[other_user]->set_data_channel_ready(true);
                
                data_channel->set_on_message(data_channel_message_handler);
                
                data_channel->set_on_close([thiz, other_user](){
                    FuncCall(thiz->debug_printer_, "saw dataChannelR.onclose event");
                    
                    if (HasKey(thiz->peer_conns_, other_user)) {
                        thiz->peer_conns_[other_user]->set_data_channel_ready(false);
                        thiz->peer_conns_[other_user]->set_data_channel_r(nullptr);
                    }
                    
                    FuncCall(thiz->on_data_channel_close_, other_user);
                    
                    thiz->UpdateConfigurationInfo();
                });
                
                data_channel->set_on_open([thiz, other_user, data_channel](){
                    FuncCall(thiz->debug_printer_, "saw dataChannel.onopen event");
                    
                    if (HasKey(thiz->peer_conns_, other_user)) {
                        data_channel->Send(eio::PacketData("dataChannelPrimed"));
                    }
                });                
            });
            
        };
        

        //
        //  added for interoperability
        //
        
        auto do_data_channels = data_enabled_;
        if (do_data_channels) {

            // check if both sides have the same browser and versions
        }

        if (do_data_channels) {
            SetPeerListener(Some(std::string("dataChannelPrimed")),
                            Some(other_user),
                            [thiz, other_user](const std::string & easyrtcid,
                                               const std::string & msg_type,
                                               const Any & msd_data,
                                               const Any & targeting)
                            {
                                thiz->peer_conns_[other_user]->set_data_channel_ready(true);
                                
                                if (thiz->peer_conns_[other_user]->call_success_cb()) {
                                    thiz->peer_conns_[other_user]->call_success_cb()(other_user, "datachannel");
                                }
                                if (thiz->on_data_channel_open_) {
                                    thiz->on_data_channel_open_(other_user, true);
                                }
                                thiz->UpdateConfigurationInfo();
                            });
            
            
            if (is_initiator) {
                init_out_going_channel(other_user);
            }
            if (!is_initiator) {
                init_incoming_channel(other_user);
            }
        }


        //
        // Temporary support for responding to acknowledgements of about streams being added.
        //
        
        SetPeerListener(Some(std::string("easyrtc_streamReceived")),
                        Some(other_user),
                        [thiz, new_peer_conn](const std::string & easyrtcid,
                                              const std::string & msg_type,
                                              const Any & msg_data,
                                              const Any & targeting)
                        {
                            std::string stream_name = msg_data.GetAt("streamName").AsString().value();
                            if (HasKey(new_peer_conn->streams_added_acks(), stream_name))  {
                                (new_peer_conn->streams_added_acks()[stream_name])(easyrtcid, stream_name);
                                new_peer_conn->streams_added_acks().erase(stream_name);
                            }
                        });
        
        return pc;
    }
    
    void Easyrtc::DoAnswer(const std::string & caller,
                           const Any & msg_data,
                           const Optional<std::vector<std::string> > &stream_names)
    {
        auto thiz = shared_from_this();
        
        if (!stream_names && auto_init_user_media_) {
            auto local_stream = GetLocalStream(None());
            if (!local_stream && (video_enabled_ || audio_enabled_)) {
                InitMediaSource(None(),
                                [thiz, caller, msg_data](const std::shared_ptr<MediaStream> & stream)
                                {
                                    thiz->DoAnswer(caller, msg_data, None());
                                },
                                [thiz](const std::string & code, const std::string & text){
                                    thiz->ShowError(thiz->err_codes_MEDIA_ERR_,
                                                    thiz->Format(thiz->GetConstantString("localMediaError"), {} ));
                                });
                return;
            }
        }
        if (use_fresh_ice_each_peer_) {
            GetFreshIceConfig([thiz, caller, msg_data, stream_names](bool succeeded){
                if (succeeded) {
                    thiz->DoAnswerBody(caller, msg_data, stream_names);
                }
                else {
                    thiz->ShowError(thiz->err_codes_CALL_ERR_, "Failed to get fresh ice config");
                }
            });
        }
        else {
            DoAnswerBody(caller, msg_data, stream_names);
        }
    }
    
    void Easyrtc::DoAnswerBody(const std::string & caller, const Any & msg_data,
                               const Optional<std::vector<std::string>> & stream_names)
    {
        auto thiz = shared_from_this();
        
        auto pc = BuildPeerConnection(caller, false, [thiz](const std::string & code, const std::string & msg) {
            thiz->ShowError(thiz->err_codes_SYSTEM_ERR_, nwr::Format("%s, %s", code.c_str(), msg.c_str()));
        }, stream_names);
        auto new_peer_conn = thiz->peer_conns_[caller];
        if (!pc) {
            FuncCall(debug_printer_, "buildPeerConnection failed. Call not answered");
            return;
        }
        auto set_local_and_send_message_1 = [thiz, caller, new_peer_conn, pc]
        (const std::shared_ptr<RtcSessionDescription> & session_description) {
            if (new_peer_conn->canceled()) {
                return;
            }
            
            auto send_answer = [thiz, caller, pc, session_description](){
                FuncCall(thiz->debug_printer_, "sending answer");

                auto on_signal_success = [](const std::string & msg_type, const Any & msg_data){
                    
                };
                auto on_signal_failure = [thiz, caller](const std::string & code, const std::string & text){
                    thiz->DeletePeerConn(caller);
                    thiz->ShowError(code, text);
                };
                
                thiz->SendSignaling(Some(caller), "answer", session_description->ToAny(),
                                    on_signal_success, on_signal_failure);
                
                thiz->peer_conns_[caller]->set_connection_accepted(true);
                thiz->SendQueuedCandidates(caller, on_signal_success, on_signal_failure);
            };
            
            if (thiz->sdp_local_filter_) {
                session_description->set_sdp(thiz->sdp_local_filter_(session_description->sdp()));
            }
            pc->SetLocalDescription(session_description, send_answer, [thiz](const std::string & message){
                thiz->ShowError(thiz->err_codes_INTERNAL_ERR_,
                                std::string("setLocalDescription: " + message));
            });
        };
        
        std::shared_ptr<RtcSessionDescription> sd = RtcSessionDescription::FromAny(msg_data);
        
        FuncCall(debug_printer_, std::string("sdp ||  ") + sd->ToAny().ToJsonString());
        
        auto invoke_create_answer = [thiz, new_peer_conn, pc, set_local_and_send_message_1]() {
            if (new_peer_conn->canceled()) {
                return;
            }
            
            pc->CreateAnswer(thiz->received_media_constraints_,
                             set_local_and_send_message_1,
                             [thiz](const std::string & message){
                                 thiz->ShowError(thiz->err_codes_INTERNAL_ERR_, std::string("create-answer: " + message));
                             });
        };
        
        FuncCall(debug_printer_, "about to call setRemoteDescription in doAnswer");

        if (sdp_remote_filter_) {
            sd->set_sdp(sdp_remote_filter_(sd->sdp()));
        }
        
        pc->SetRemoteDescription(sd,
                                 invoke_create_answer,
                                 [thiz](const std::string & message){
                                     thiz->ShowError(thiz->err_codes_INTERNAL_ERR_,
                                                     std::string("set-remote-description: ") + message);
                                 });
    }
    
    void Easyrtc::EmitOnStreamClosed(const std::string & easyrtcid,
                                     const std::shared_ptr<MediaStream> &stream)
    {
        if (!HasKey(peer_conns_, easyrtcid)) {
            return;
        }

        std::string id = stream->id();
        std::string stream_name;
        if (HasKey(peer_conns_[easyrtcid]->remote_stream_id_to_name(), id)) {
            stream_name = peer_conns_[easyrtcid]->remote_stream_id_to_name()[id];
        } else {
            stream_name = "default";
        }
        
        if (HasKey(peer_conns_[easyrtcid]->live_remote_streams(), stream_name) &&
            on_stream_closed_)
        {
            peer_conns_[easyrtcid]->live_remote_streams().erase(stream_name);
            on_stream_closed_(easyrtcid, stream, stream_name);
        }
        
        peer_conns_[easyrtcid]->remote_stream_id_to_name().erase(id);
    }
    
    void Easyrtc::OnRemoteHangup(const std::string & caller) {
        offers_pending_.erase(caller);
        FuncCall(debug_printer_, "Saw onRemote hangup event");
        
        if (HasKey(peer_conns_, caller)) {
            peer_conns_[caller]->set_canceled(true);
            if (peer_conns_[caller]->pc()) {
                //
                // close any remote streams.
                //
                auto remote_streams = peer_conns_[caller]->pc()->remote_streams();
                for (int i = 0; i < remote_streams.size(); i++) {
                    EmitOnStreamClosed(caller, remote_streams[i]);
                    StopStream(remote_streams[i]);
                }
                
                peer_conns_[caller]->pc()->Close();
            }
            else {
                FuncCall(call_canceled_, caller, true);
            }
            DeletePeerConn(caller);
            UpdateConfigurationInfo();
        }
        else {
            FuncCall(call_canceled_, caller, true);
        }
    }
    
    void Easyrtc::ClearQueuedMessages(const std::string & caller) {
        queued_messages_[caller] = Any(Any::ObjectType {
            { "candidates", Any(Any::ArrayType{}) }
        });
    }
    
    bool Easyrtc::IsPeerInAnyRoom(const std::string & id) {
        for (const auto & room_name : Keys(last_logged_in_list_)) {
            if (last_logged_in_list_[room_name].HasKey(id)) {
                return true;
            }
        }
        return false;
    }
    
    void Easyrtc::ProcessLostPeers(const std::map<std::string, Any> & peers_in_room) {
        //
        // check to see the person is still in at least one room. If not, we'll hangup
        // on them. This isn't the correct behavior, but it's the best we can do without
        // changes to the server.
        //
        
        for (const auto & id : Keys(peer_conns_)) {
            if (!HasKey(peers_in_room, id)) {
                if (!IsPeerInAnyRoom(id)) {
                    if (peer_conns_[id]->pc() || peer_conns_[id]->is_initiator()) {
                        OnRemoteHangup(id);
                    }
                    offers_pending_.erase(id);
                    acceptance_pending_.erase(id);
                    ClearQueuedMessages(id);
                }
            }
        }
        
        for (const auto & id : Keys(offers_pending_)) {
            if (!IsPeerInAnyRoom(id)) {
                OnRemoteHangup(id);
                ClearQueuedMessages(id);
                offers_pending_.erase(id);
                acceptance_pending_.erase(id);
            }
        }
        
        for (const auto & id : Keys(acceptance_pending_)) {
            if (!IsPeerInAnyRoom(id)) {
                OnRemoteHangup(id);
                ClearQueuedMessages(id);
                acceptance_pending_.erase(id);
            }
        }
        
    }
    
    void Easyrtc::AddAggregatingTimer(const std::string & key,
                                      const std::function<void()> & callback,
                                      const Optional<TimeDuration> & arg_period)
    {
        auto thiz = shared_from_this();
        
        TimeDuration period = arg_period || TimeDuration(0.1);
        
        int counter = 0;
        if (HasKey(aggregating_timers_, key)) {
            if (aggregating_timers_[key].timer) {
                aggregating_timers_[key].timer->Cancel();
                aggregating_timers_[key].timer = nullptr;
            }
            counter = aggregating_timers_[key].counter;
        }
        if (counter > 20) {
            aggregating_timers_.erase(key);
            FuncCall(callback);
        }
        else {
            aggregating_timers_[key] = AggregatingTimer(counter + 1);
            aggregating_timers_[key].timer = Timer::Create(period, [thiz, key, callback](){
                thiz->aggregating_timers_.erase(key);
                FuncCall(callback);
            });
        }
    }
    
    void Easyrtc::ProcessOccupantList(const std::string & room_name,
                                      std::map<std::string, Any> & occupant_list)
    {
        auto thiz = shared_from_this();
        
        Any my_info;
        std::map<std::string, Any> reduced_list;
        
        for (const auto & id : Keys(occupant_list)) {
            if (Some(id) == my_easyrtcid_) {
                my_info = occupant_list[id];
            }
            else {
                reduced_list[id] = occupant_list[id];
            }
            
        }
        //
        // processLostPeers detects peers that have gone away and performs
        // house keeping accordingly.
        //
        ProcessLostPeers(reduced_list);
        //
        //
        //
        AddAggregatingTimer(std::string("roomOccupants&") + room_name, [thiz, room_name, reduced_list, my_info](){
            if (thiz->room_occupant_listener_) {
                thiz->room_occupant_listener_(Some(room_name), reduced_list, my_info);
            }
            
            thiz->EmitEvent("roomOccupants", Any(Any::ObjectType
                                                 {
                                                     { "roomName", Any(room_name) },
                                                     { "occupants", Any(thiz->last_logged_in_list_) }
                                                 }));
        }, Some(TimeDuration(0.1)));
    }
    
    void Easyrtc::SendQueuedCandidates(const std::string & peer,
                                       const std::function<void (const std::string &,
                                                                 const Any &)> & on_signal_success,
                                       const std::function<void (const std::string &,
                                                                 const std::string &)> & on_signal_failure)
    {
        for (const auto & cand : peer_conns_[peer]->candidates_to_send()) {
            SendSignaling(Some(peer),
                          "candidate",
                          cand,
                          on_signal_success,
                          on_signal_failure);
        }
    }
    
    void Easyrtc::OnChannelMsg(const Any & msg,
                               const std::function<void(const Any &)> & ack_acceptor_func)
    {
        Any targeting = Any(Any::ObjectType{});
        
        if (ack_acceptor_func) {
            ack_acceptor_func(ack_message_);
        }
        
        if (msg.GetAt("targetEasyrtcid")) {
            targeting.SetAt("targetEasyrtcid", msg.GetAt("targetEasyrtcid"));
        }
        if (msg.GetAt("targetRoom")) {
            targeting.SetAt("targetRoom", msg.GetAt("targetRoom"));
        }
        if (msg.GetAt("targetGroup")) {
            targeting.SetAt("targetGroup", msg.GetAt("targetGroup"));
        }
        if (msg.GetAt("senderEasyrtcid")) {
            ReceivePeerDistribute(msg.GetAt("senderEasyrtcid").AsString().value(),
                                  msg, targeting);
        }
        else {
            if (receive_server_cb_){
                receive_server_cb_(msg.GetAt("msgType").AsString().value(),
                                   msg.GetAt("msgData"),
                                   targeting);
            }
            else {
                printf("Unhandled server message %s\n", msg.ToJsonString().c_str());
            }
        }
    }
    
    void Easyrtc::OnChannelCmd(const Any & msg,
                               const std::function<void(const Any &)> & ack_acceptor_fn)
    {
        auto thiz = shared_from_this();
        
        Optional<std::string> caller = msg.GetAt("senderEasyrtcid").AsString();
        std::string msg_type = msg.GetAt("msgType").AsString().value();
        Any msg_data = msg.GetAt("msgData");
        
        auto pc_ptr = std::make_shared<std::shared_ptr<RtcPeerConnection>>(nullptr);
        
        FuncCall(debug_printer_, std::string("received message of type ") + msg_type);

        if (caller && HasKey(queued_messages_, *caller)) {
            ClearQueuedMessages(*caller);
        }
        
        auto process_candidate_body = [thiz, pc_ptr](const std::string & caller, const Any & arg_msg_data){
            Any msg_data = arg_msg_data;
            
            if (thiz->ice_candidate_filter_) {
                msg_data = thiz->ice_candidate_filter_(msg_data, true);
                if (!msg_data) {
                    return;
                }
            }
            
            std::string mid = msg_data.GetAt("id").AsString().value();
            int mline_index = msg_data.GetAt("label").AsInt().value();
            std::string candidate_str = msg_data.GetAt("candidate").AsString().value();

            std::shared_ptr<RtcIceCandidate> candidate =
            std::make_shared<RtcIceCandidate>(mid, mline_index, candidate_str);
            
            *pc_ptr = thiz->peer_conns_[caller]->pc();
            
#warning todo open issue
//            auto ice_add_success = [](){
//                
//            };
//            auto ice_add_failure = [thiz, candidate](const std::string & error){
//                thiz->ShowError(thiz->err_codes_ICECANDIDATE_ERROR_,
//                                nwr::Format("bad ice candidate (%s): %s", error.c_str(), candidate->ToAny().ToJsonString().c_str()));
//            };
            
            (*pc_ptr)->AddIceCandidate(candidate);
            
            if (IndexOf(candidate_str, "typ relay") != -1) {
                std::regex regex("(udp|tcp) \\d+ (\\d+\\.\\d+\\.\\d+\\.\\d+)", std::regex_constants::icase);
                std::smatch match_ret;
                if (std::regex_search(candidate_str, match_ret, regex)) {
                    std::string ip_address = match_ret[2].str();
                    thiz->turn_servers_[ip_address] = true;
                } else {
                    printf("ip address match failed [%s]\n", candidate_str.c_str());
                }
            }
        };
        
        auto flush_cached_candidates = [thiz, process_candidate_body](const std::string & caller) {
            if (HasKey(thiz->queued_messages_, caller)) {
                const auto candidates = thiz->queued_messages_[caller].GetAt("candidates").AsArray().value();
                for (const auto & candidate : candidates) {
                    process_candidate_body(caller, candidate);
                }
                thiz->queued_messages_.erase(caller);
            }
        };
        
        auto process_offer = [thiz, flush_cached_candidates]
        (const std::string & caller, const Any & msg_data)
        {
            std::function<void(bool,
                               const Optional<std::vector<std::string>> &)> helper =
            [thiz, caller, msg_data, flush_cached_candidates]
            (bool was_accepted,
             const Optional<std::vector<std::string>> & stream_names)
            {
                FuncCall(thiz->debug_printer_, nwr::Format("offer accept=%d", was_accepted));
                thiz->offers_pending_.erase(caller);
                
                if (was_accepted) {
                    if (!thiz->SupportsPeerConnections()) {
                        thiz->ShowError(thiz->err_codes_CALL_ERR_,
                                        thiz->GetConstantString("noWebrtcSupport"));
                        return;
                    }
                    thiz->DoAnswer(caller, msg_data, stream_names);
                    flush_cached_candidates(caller);
                }
                else {
                    thiz->SendSignaling(Some(caller),
                                        "reject",
                                        Any(), nullptr, nullptr);
                    thiz->ClearQueuedMessages(caller);
                }
            };
            //
            // There is a very rare case of two callers sending each other offers
            // before receiving the others offer. In such a case, the caller with the
            // greater valued easyrtcid will delete its pending call information and do a
            // simple answer to the other caller's offer.
            //
            
            if (HasKey(thiz->acceptance_pending_, caller) &&
                thiz->my_easyrtcid_ &&
                caller < thiz->my_easyrtcid_.value()
                )
            {
                thiz->acceptance_pending_.erase(caller);
                if (HasKey(thiz->queued_messages_, caller)) {
                    thiz->queued_messages_.erase(caller);
                }
                FuncCall(thiz->peer_conns_[caller]->was_accepted_cb(), true, caller);
                
                thiz->DeletePeerConn(caller);
                helper(true, None());
                return;
            }
            
            thiz->offers_pending_[caller] = msg_data;
            
            if (!thiz->accept_check_) {
                helper(true, None());
            }
            else {
                thiz->accept_check_(caller, helper);
            }
        };
        
        auto process_reject = [thiz](const std::string & caller){
            thiz->acceptance_pending_.erase(caller);
            if (HasKey(thiz->queued_messages_, caller)) {
                thiz->queued_messages_.erase(caller);
            }
            if (HasKey(thiz->peer_conns_, caller)) {
                FuncCall(thiz->peer_conns_[caller]->was_accepted_cb(), false, caller);
                thiz->DeletePeerConn(caller);
            }
        };

        auto process_answer = [thiz, pc_ptr, flush_cached_candidates](const std::string & caller, const Any & msg_data) {
            thiz->acceptance_pending_.erase(caller);

            //
            // if we've discarded the peer connection, ignore the answer.
            //
            
            if (!HasKey(thiz->peer_conns_, caller)) {
                return;
            }
            thiz->peer_conns_[caller]->set_connection_accepted(true);

            FuncCall(thiz->peer_conns_[caller]->was_accepted_cb(), true, caller);

            auto on_signal_success = [](const std::string & msg_type,
                                        const Any & data) {};
            auto on_signal_failure = [thiz, caller](const std::string & code, const std::string & text) {
                if (HasKey(thiz->peer_conns_, caller)) {
                    thiz->DeletePeerConn(caller);
                }
                thiz->ShowError(code, text);
            };

            // peerConns[caller].startedAV = true;
            thiz->SendQueuedCandidates(caller, on_signal_success, on_signal_failure);
            
            *pc_ptr = thiz->peer_conns_[caller]->pc();
            
            std::shared_ptr<RtcSessionDescription> sd = RtcSessionDescription::FromAny(msg_data);
            if (!sd) {
                Fatal("Could not create the RTCSessionDescription");
            }
            
            FuncCall(thiz->debug_printer_, "about to call initiating setRemoteDescription");
            
            if (thiz->sdp_remote_filter_) {
                sd->set_sdp(thiz->sdp_remote_filter_(sd->sdp()));
            }
            
            (*pc_ptr)->SetRemoteDescription(sd,
                                            [](){
                                                
                                            },
                                            [](const std::string & message){
                                                printf("setRemoteDescription failed %s", message.c_str());
                                            });
            
            flush_cached_candidates(caller);
        };
        
        auto process_candidate_queue = [thiz, process_candidate_body]
        (const std::string & caller, const Any & msg_data)
        {
            if (HasKey(thiz->peer_conns_, caller) && thiz->peer_conns_[caller]->pc()) {
                process_candidate_body(caller, msg_data);
            }
            else {
                if (!HasKey(thiz->peer_conns_, caller)) {
                    thiz->queued_messages_[caller] = Any(Any::ObjectType {
                        { "candidates", Any(Any::ArrayType{}) }
                    });
                }
                thiz->queued_messages_[caller].GetAt("candidates").AsArray()->push_back(msg_data);
            }
        };

        if (msg_type == "sessionData") {
            ProcessSessionData(msg_data.GetAt("sessionData"));
        } else if (msg_type == "roomData") {
            ProcessRoomData(msg_data.GetAt("roomData"));
        } else if (msg_type == "iceConfig") {
            ProcessIceConfig(msg_data.GetAt("iceConfig"));
        } else if (msg_type == "forwardToUrl") {
            printf("forward to url: %s\n", msg_data.ToJsonString().c_str());
        } else if (msg_type == "offer") {
            process_offer(*caller, msg_data);
        } else if (msg_type == "reject") {
            process_reject(*caller);
        } else if (msg_type == "answer") {
            process_answer(*caller, msg_data);
        } else if (msg_type == "candidate") {
            process_candidate_queue(*caller, msg_data);
        } else if (msg_type == "hangup") {
            OnRemoteHangup(*caller);
            ClearQueuedMessages(*caller);
        } else if (msg_type == "error") {
            ShowError(msg_data.GetAt("errorCode").AsString().value(),
                      msg_data.GetAt("errorText").AsString().value());
        } else {
            printf("received unknown message type from server; msg=%s\n", msg.ToJsonString().c_str());
            return;
        }
        
        if (ack_acceptor_fn) {
            ack_acceptor_fn(ack_message_);
        }
    }
    
    void Easyrtc::ConnectToWSServer(const std::function<void(const std::string &)> & success_callback,
                                    const std::function<void(const std::string &,
                                                             const std::string &)> & error_callback)
    {
        auto thiz = shared_from_this();

        if (preallocated_socket_io_) {
            websocket_ = preallocated_socket_io_;
        }
        else if (!websocket_) {
            websocket_ = sio0::Io::Connect(server_path_, connection_options_);
            if (!websocket_) {
                Fatal("sio::Io failed");
            }
        }
        else {
            for (const auto & entry : websocket_listeners_) {
                websocket_->emitter()->Off(entry.event, entry.handler);
            }
        }
        
        websocket_listeners_.clear();
        
        auto add_socket_listener = [thiz](const std::string & event, const AnyEventListener & handler) {
            thiz->websocket_->emitter()->On(event, handler);
            thiz->websocket_listeners_.push_back({ event, handler });
        };
        
        add_socket_listener("close", AnyEventListenerMake([](const Any & event){
            printf("the web socket closed\n");
        }));
        add_socket_listener("error", AnyEventListenerMake([thiz, error_callback](const Any & event){
            
            if (thiz->my_easyrtcid_) {
                //
                // socket.io version 1 got rid of the socket member, moving everything up one level.
                //
                if (thiz->IsSocketConnected(thiz->websocket_)) {
                    thiz->ShowError(thiz->err_codes_SIGNAL_ERROR_,
                                    thiz->GetConstantString("miscSignalError"));
                }
                else {
                    /* socket server went down. this will generate a 'disconnect' event as well, so skip this event */
                    error_callback(thiz->err_codes_CONNECT_ERR_,
                                   thiz->GetConstantString("noServer"));
                }
            }
            else {
                error_callback(thiz->err_codes_CONNECT_ERR_,
                               thiz->GetConstantString("noServer"));
            }
            
        }));
        
        auto connect_handler = [thiz, success_callback, error_callback](const Any & event) {
            thiz->websocket_connected_ = true;
            if (!thiz->websocket_) {
                thiz->ShowError(thiz->err_codes_CONNECT_ERR_,
                                thiz->GetConstantString("badsocket"));
            }

            FuncCall(thiz->debug_printer_, "saw socket-server connect event");
            
            if (thiz->websocket_connected_) {
                thiz->SendAuthenticate(success_callback, error_callback);
            }
            else {
                error_callback(thiz->err_codes_SIGNAL_ERROR_,
                               thiz->GetConstantString("icf"));
            }
        };
        
        if (IsSocketConnected(preallocated_socket_io_)) {
            connect_handler(nullptr);
        }
        else {
            add_socket_listener("connect", AnyEventListenerMake(connect_handler));
        }
        add_socket_listener("easyrtcMsg", AnyEventListenerMake([thiz](const Any & event, const Any & arg_ack){
            std::function<void(const Any &)> ack = nullptr;
            
            Optional<AnyFuncPtr> ack_func_opt = arg_ack.AsFunction();
            if (ack_func_opt) {
                ack = [ack_func_opt](const Any & x){
                    (*ack_func_opt)->Call({ x });
                };
            }
            
            thiz->OnChannelMsg(event, ack);
        }));
        add_socket_listener("easyrtcCmd", AnyEventListenerMake([thiz](const Any & event, const Any & arg_ack){
            std::function<void(const Any &)> ack = nullptr;
            
            Optional<AnyFuncPtr> ack_func_opt = arg_ack.AsFunction();
            if (ack_func_opt) {
                ack = [ack_func_opt](const Any & x){
                    (*ack_func_opt)->Call({ x });
                };
            }
            
            thiz->OnChannelCmd(event, ack);
        }));
        
        add_socket_listener("disconnect", AnyEventListenerMake([thiz](const Any & event){
            thiz->websocket_connected_ = false;
            thiz->update_configuration_info_ = [](){
                
            }; // dummy update function
            thiz->old_config_ = Any(Any::ObjectType{});
            thiz->DisconnectBody();
            FuncCall(thiz->disconnect_listener_);
        }));
    }
    
    Any Easyrtc::BuildDeltaRecord(const Any & added, const Any & deleted) {
        auto object_not_empty = [](const Any & obj){
            return obj.keys().size() > 0;
        };
        
        Any result(Any::ObjectType{});
        
        if (object_not_empty(added)) {
            result.SetAt("added", added);
        }

        if (object_not_empty(deleted)) {
            result.SetAt("deleted", deleted);
        }
        
        if (object_not_empty(result)) {
            return result;
        }
        else {
            return nullptr;
        }
    }
    
    Any Easyrtc::FindDeltas(const Any & old_version, const Any & new_version) {
        Any added = Any(Any::ObjectType{});
        Any deleted = Any(Any::ObjectType{});
//        var subPart;
        for (const std::string & i : new_version.keys()) {
            if (!old_version || !old_version.GetAt(i)) {
                added.SetAt(i, new_version.GetAt(i));
            }
            else if (new_version.GetAt(i).type() == Any::Type::Object) {
                Any sub_part = FindDeltas(old_version.GetAt(i), new_version.GetAt(i));
                if (sub_part) {
                    added.SetAt(i, new_version.GetAt(i));
                }
            }
            else if (new_version.GetAt(i) != old_version.GetAt(i)) {
                added.SetAt(i, new_version.GetAt(i));
            }
        }
        
        for (const std::string & i : old_version.keys()) {
            if (!new_version.GetAt(i)) {
                deleted.SetAt(i, old_version.GetAt(i));
            }
        }
        
        return BuildDeltaRecord(added, deleted);
    }
    
    Any Easyrtc::CollectConfigurationInfo(bool forAuthentication) {
        Any p2p_list(Any::ObjectType{});
        
        for (const std::string & i : Keys(peer_conns_)) {
            Any connect_time_json;
            if (peer_conns_[i]->connect_time()) {
                connect_time_json = Any(ToString(peer_conns_[i]->connect_time().value()));
            }
            
            p2p_list.SetAt(i, Any(Any::ObjectType{
                { "connectTime", connect_time_json },
                { "isInitiator", Any(peer_conns_[i]->is_initiator()) }
            }));
        }

#warning todo sizes
#warning todo language
        Any new_config(Any::ObjectType {
            { "userSettings", Any(Any::ObjectType{
                { "sharingAudio", Any(have_audio_) },
                { "sharingVideo", Any(have_video_) },
                { "sharingData", Any(data_enabled_) },
                { "nativeVideoWidth", Any(640) },
                { "nativeVideoHeight", Any(480) },
                { "windowWidth", Any(320) },
                { "windowHeight", Any(640) },
                { "screenWidth", Any(320) },
                { "screenHeight", Any(640) },
                { "cookieEnabled", Any(false) },
                { "os", Any("iOS") },
                { "language", Any("ja") },
            }) }
        });
        
        if (!IsEmptyObj(p2p_list)) {
            new_config.SetAt("p2pList", p2p_list);
        }

        return new_config;
    }
    
    void Easyrtc::UpdateConfiguration() {
        auto thiz = shared_from_this();
        std::weak_ptr<Easyrtc> whiz = shared_from_this();
        
        auto new_config = CollectConfigurationInfo(false);
        //
        // we need to give the getStats calls a chance to fish out the data.
        // The longest I've seen it take is 5 milliseconds so 100 should be overkill.
        //
        auto send_deltas = [thiz, new_config](){
            auto altered_data = thiz->FindDeltas(thiz->old_config_, new_config);
            //
            // send all the configuration information that changes during the session
            //
            if (altered_data) {
                FuncCall(thiz->debug_printer_, std::string("cfg=") + altered_data.ToJsonString());
                
                if (thiz->websocket_) {
                    thiz->SendSignaling(None(), "setUserCfg",
                                        Any(Any::ObjectType
                                            {
                                                { "setUserCfg", altered_data.GetAt("added") }
                                            }),
                                        nullptr, nullptr);
                }
            }
            
            thiz->old_config_ = new_config;
        };
        
        if (old_config_.count() == 0) {
            send_deltas();
        }
        else {
            Timer::Create(TimeDuration(0.1), [send_deltas]{
                send_deltas();
            });
        }
    }
    
    void Easyrtc::UpdateConfigurationInfo() {
        FuncCall(update_configuration_info_);
    }

    void Easyrtc::UpdatePresence(const std::string & state, const std::string & status_text) {
        presence_show_ = Some(state);
        presence_status_ = Some(status_text);
        if (websocket_connected_) {
            SendSignaling(None(), "setPresence", Any(Any::ObjectType
                                                     {
                                                         { "show", Any(state) },
                                                         { "status", Any(status_text) }
                                                     }),
                          nullptr, nullptr);
        }
    }
    
    std::map<std::string, Any> Easyrtc::GetSessionFields() {
        return session_fields_;
    }
    
    Any Easyrtc::GetSessionField(const std::string & name) {
        if (HasKey(session_fields_, name)) {
            return session_fields_[name].GetAt("fieldValue");
        }
        else {
            return nullptr;
        }
    }
    
    void Easyrtc::ProcessSessionData(const Any & session_data) {
        if (session_data) {
            if (session_data.GetAt("easyrtcsid")) {
                easyrtcsid_ = Some(session_data.GetAt("easyrtcsid").AsString().value());
            }
            if (session_data.GetAt("field")) {
                session_fields_ = session_data.GetAt("field").AsObject().value();
            }
        }
    }
    
    void Easyrtc::ProcessRoomData(const Any & room_data) {
        room_data_ = room_data.AsObject().value();
        
        for (const auto & room_name : Keys(room_data_)) {
            if (room_data_[room_name].GetAt("roomStatus").AsString() == Some(std::string("join"))) {
                if (!HasKey(room_join_, room_name)) {
#warning change js ref copy to cpp val copy; it may cause bugs.
                    
                    room_join_[room_name] = room_data_[room_name];
                }
                
                auto media_ids = BuildMediaIds();
                if (media_ids.size() != 0) {
                    SetRoomApiField(room_name, "mediaIds", Any(media_ids));
                }
            }
            else if (room_data_[room_name].GetAt("roomStatus").AsString() == Some(std::string("leave"))) {
                FuncCall(room_entry_listener_, false, room_name);
                room_join_.erase(room_name);
                last_logged_in_list_.erase(room_name);
                continue;
            }
            
            if (room_data_[room_name].GetAt("clientList")) {
                last_logged_in_list_[room_name] = room_data_[room_name].GetAt("clientList");
            }
            else if (room_data_[room_name].GetAt("clientListDelta")) {
                auto stuff_to_add = room_data_[room_name].GetAt("clientListDelta").GetAt("updateClient");
                if (stuff_to_add) {
                    for (const std::string & id : stuff_to_add.keys()) {
                        if (!HasKey(last_logged_in_list_, room_name)) {
                            last_logged_in_list_[room_name] = Any(Any::ObjectType{});
                        }
                        if( !last_logged_in_list_[room_name].HasKey(id) ) {
                            last_logged_in_list_[room_name].SetAt(id, stuff_to_add.GetAt(id));
                        }
                        for (const std::string & k : stuff_to_add.GetAt(id).keys()) {
                            if( k == "apiField" || k == "presence") {
                                last_logged_in_list_[room_name].GetAt(id).SetAt(k, stuff_to_add.GetAt(id).GetAt(k));
                            }
                        }
                    }
                }
                auto stuff_to_remove = room_data_[room_name].GetAt("clientListDelta").GetAt("removeClient");
                if (stuff_to_remove && HasKey(last_logged_in_list_, room_name)) {
                    for (const std::string & remove_id : stuff_to_remove.keys()) {
                        last_logged_in_list_[room_name].RemoveAt(remove_id);
                    }
                }
            }
            if (HasKey(room_join_, room_name) && room_data_[room_name].GetAt("field")) {
                fields_.rooms[room_name] = room_data_[room_name].GetAt("field").AsObject().value();
            }
            if (room_data_[room_name].GetAt("roomStatus").AsString() == Some(std::string("join"))) {
                FuncCall(room_entry_listener_, true, room_name);
            }
            ProcessOccupantList(room_name, last_logged_in_list_[room_name].AsObject().value());
        }
        EmitEvent("roomOccupant", Any(last_logged_in_list_));
    }
    
    std::vector<std::string> Easyrtc::GetRoomOccupantsAsArray(const std::string & room_name) {
        if (!HasKey(last_logged_in_list_, room_name)) {
            return {};
        }
        else {
            return last_logged_in_list_[room_name].keys();
        }
    }
    
    std::map<std::string, Any> Easyrtc::GetRoomOccupantsAsMap(const std::string & room_name) {
        return last_logged_in_list_[room_name].AsObject().value();
    }
    
    bool Easyrtc::IsTurnServer(const std::string & ip_address) {
        return turn_servers_[ip_address];
    }
    
    void Easyrtc::ProcessIceConfig(const Any & arg_ice_config) {
        Any ice_config = arg_ice_config;
        
        pc_config_ = std::make_shared<webrtc::PeerConnectionInterface::RTCConfiguration>();
        turn_servers_.clear();
        
        if (!ice_config || !ice_config.HasKey("iceServers") ||
            ice_config.GetAt("iceServers").type() != Any::Type::Array)
        {
            ShowError(err_codes_DEVELOPER_ERR_, "iceConfig received from server didn't have an array called iceServers, ignoring it");
            ice_config = Any(Any::ObjectType{
                { "iceServers", Any(Any::ArrayType{}) }
            });
        }
        
        for (int i = 0; i < ice_config.GetAt("iceServers").count(); i++) {
            auto item = ice_config.GetAt("iceServers").GetAt(i);
            Any fixed_item;
            
//            printf("ice server [%d] %s\n", i, item.ToJsonString().c_str());
            
            if (IndexOf(item.GetAt("url").AsString().value(), "turn:") == 0) {
                if (item.HasKey("username")) {
                    fixed_item = CreateIceServer(item.GetAt("url").AsString().value(),
                                                 item.GetAt("username").AsString().value(),
                                                 item.GetAt("credential").AsString().value());
                }
                else {
                    ShowError(err_codes_DEVELOPER_ERR_, std::string("TURN server entry doesn't have a username: ") + item.ToJsonString());
                }
                std::string url_str = item.GetAt("url").AsString().value();
                std::string ip_address = Split(url_str, std::regex("[@:&]"))[1];
                turn_servers_[ip_address] = true;
            }
            else { // is stun server entry
                fixed_item = item;
            }
            if (fixed_item) {
                webrtc::PeerConnectionInterface::IceServer entry;
                entry.uri = fixed_item.GetAt("url").AsString().value();
                entry.username = fixed_item.GetAt("username").AsString() || std::string();
                entry.password = fixed_item.GetAt("credential").AsString() || std::string();
                pc_config_->servers.push_back(entry);
            }
        }
    }
    
    void Easyrtc::GetFreshIceConfig(const std::function<void (bool)> & arg_callback) {
        auto thiz = shared_from_this();
        auto callback = arg_callback;
        Any data_to_ship(Any::ObjectType{
            { "msgType", Any("getIceConfig") },
            { "msgData", Any(Any::ObjectType{})}
        });
        if (!callback) {
            callback = [](bool v){};
        }
        websocket_->JsonEmit("easyrtcCmd", {
            data_to_ship,
            AnyFuncMake([thiz, callback](const Any & ack_msg) {
                if (ack_msg.GetAt("msgType").AsString() == Some(std::string("iceConfig"))) {
                    thiz->ProcessIceConfig(ack_msg.GetAt("msgData").GetAt("iceConfig"));
                    callback(true);
                }
                else {
                    thiz->ShowError(ack_msg.GetAt("msgData").GetAt("errorCode").AsString() || std::string(),
                                    ack_msg.GetAt("msgData").GetAt("errorText").AsString() || std::string());
                    callback(false);
                }
            })
        });
    }
    
    void Easyrtc::ProcessToken(const Any & msg) {
        FuncCall(debug_printer_, "entered process token");
        
        auto msg_data = msg.GetAt("msgData");
        if (msg_data.HasKey("easyrtcid")) {
            my_easyrtcid_ = Some(msg_data.GetAt("easyrtcid").AsString().value());
        }
        if (msg_data.HasKey("field")) {
            fields_.connection = msg_data.GetAt("field").AsObject().value();
        }
        if (msg_data.HasKey("iceConfig")) {
            ProcessIceConfig(msg_data.GetAt("iceConfig"));
        }
        if (msg_data.HasKey("sessionData")) {
            ProcessSessionData(msg_data.GetAt("sessionData"));
        }
        if (msg_data.HasKey("roomData")) {
            ProcessRoomData(msg_data.GetAt("roomData"));
        }
        if (msg_data.GetAt("application").HasKey("field")) {
            fields_.application = msg_data.GetAt("application").GetAt("field").AsObject().value();
        }
    }
    
    void Easyrtc::SendAuthenticate(const std::function<void (const std::string &)> & success_callback,
                                   const std::function<void (const std::string &,
                                                             const std::string &)> & error_callback)
    {
        auto thiz = shared_from_this();
        //
        // find our easyrtcsid
        //
#warning todo! cookie handling emulation
        easyrtcsid_ = None();

        Any msg_data(Any::ObjectType{
            { "apiVersion", Any(api_version_) },
            { "applicationName", Any(application_name_) },
            { "setUserCfg", CollectConfigurationInfo(true) }
        });
        
        if (presence_show_) {
            msg_data.SetAt("setPresence", Any(Any::ObjectType
                                              {
                                                  { "show", Any(presence_show_.value()) },
                                                  { "status", Any(presence_status_.value()) }
                                              }));
        }
        if (username_) {
            msg_data.SetAt("username", Any(username_.value()));
        }
        if (room_join_.size() != 0) {
            msg_data.SetAt("roomJoin", Any(room_join_));
        }
        if (easyrtcsid_) {
            msg_data.SetAt("easyrtcsid", Any(easyrtcsid_.value()));
        }
        if (credential_) {
            msg_data.SetAt("credential", Any(credential_.value()));
        }
        
        websocket_->JsonEmit("easyrtcAuth",
                         {
                             Any(Any::ObjectType
                               {
                                   { "msgType", Any("authenticate") },
                                   { "msgData", msg_data }
                               }),
                             AnyFuncMake([thiz, success_callback, error_callback](const Any & msg) {
                                 if (msg.GetAt("msgType").AsString() == Some(std::string("error"))) {
                                     
                                     error_callback(msg.GetAt("msgData").GetAt("errorCode").AsString() || std::string(),
                                                    msg.GetAt("msgData").GetAt("errorText").AsString() || std::string());
                                     thiz->room_join_.clear();
                                 }
                                 else {
                                     thiz->ProcessToken(msg);
                                     
                                     for (const std::string & room : Keys(thiz->room_api_fields_)) {
                                         thiz->EnqueueSendRoomApi(room);
                                     }
                                     
                                     FuncCall(success_callback, thiz->my_easyrtcid_.value());
                                 }
                             })
                         });
    }
    
    std::map<std::string, bool> Easyrtc::GetRoomsJoined() {
        std::map<std::string, bool> rooms_in;
        for (const std::string & key : Keys(room_join_)) {
            rooms_in[key] = true;
        }
        return rooms_in;
    }
    
    std::map<std::string, Any> Easyrtc::GetRoomFields(const std::string & room_name) {
        return fields_.rooms[room_name];
    }
    
    std::map<std::string, Any> Easyrtc::GetApplicationFields() {
        return fields_.application;
    }
    
    std::map<std::string, Any> Easyrtc::GetConnectionFields() {
        return fields_.connection;
    }
    
    void Easyrtc::UseThisSocketConnection(const std::shared_ptr<sio0::Socket> & already_allocated_socket_io) {
        preallocated_socket_io_ = already_allocated_socket_io;
    }
    
    void Easyrtc::Connect(const std::string & application_name,
                          const std::function<void(const std::string &)> & success_callback,
                          const std::function<void(const std::string &,
                                                   const std::string &)> & arg_error_callback)
    {
        auto error_callback = arg_error_callback;
        
        if (!preallocated_socket_io_ && websocket_) {
            printf("Developer error: attempt to connect when already connected to socket server\n");
            return;
        }
        
        pc_config_ = std::make_shared<webrtc::PeerConnectionInterface::RTCConfiguration>();
        closed_channel_ = nullptr;
        old_config_ = Any(Any::ObjectType{}); // used internally by updateConfiguration
        queued_messages_.clear();
        application_name_ = application_name;
        fields_ = Fields();
        FuncCall(debug_printer_,
                 std::string("attempt to connect to WebRTC signalling server with application name=") + application_name);

        if (!error_callback) {
            error_callback = [](const std::string & code, const std::string & text) {
                printf("easyrtc.connect: %s\n", text.c_str());
            };
        }
        
        ConnectToWSServer(success_callback, error_callback);
    }
    
    void Easyrtc::DontAddCloseButtons() {
        auto_add_close_buttons_ = false;
    }
    
    void Easyrtc::EasyAppBody(const Optional<ElementId> & monitor_video_id,
                              const std::vector<ElementId> & video_ids)
    {
        auto thiz = shared_from_this();
        
        int num_people = static_cast<int>(video_ids.size());
//        var videoIdsP = videoIds;
//        var refreshPane = 0;
//        var onCall = null, onHangup = null;
//        var videoIdToCallerMap = {};
//        if (!videoIdsP) {
//            videoIdsP = [];
//        }
        
        AddEventListener("roomOccupants",
                         AnyEventListenerMake([thiz, num_people]
                                              (const Any & event_name, const Any & event_data){
                                                  for (int i = 0; i < num_people; i++) {
                                                      
                                                      auto video_opt = thiz->GetIthVideo(i);
                                            
                                                      if (video_opt && !thiz->VideoIsFree(video_opt.value())) {
                                                          auto video = video_opt.value();
                                                          if (!thiz->IsPeerInAnyRoom(thiz->GetCallerOfVideo(video).value())) {
                                                              FuncCall(thiz->on_hangup_, thiz->GetCallerOfVideo(video).value(), i);
                                                              thiz->SetCallerOfVideo(video, None());
                                                          }
                                                      }
                                                  }
                                              }));
        
        if (monitor_video_id && !ValidateVideoIds(monitor_video_id.value(), video_ids)) {
            Fatal("bad video element id");
        }
        
        if (monitor_video_id) {
#warning todo mute monitor
//            document.getElementById(monitorVideoId).muted = "muted";
        }
        
        set_on_stream_closed([thiz, num_people]
                             (const std::string & caller,
                              const std::shared_ptr<MediaStream> & stream,
                              const std::string & stream_name)
                             {
                                 for (int i = 0; i < num_people; i++) {
                                     auto video_opt = thiz->GetIthVideo(i);
                                     
                                     if (video_opt && thiz->GetCallerOfVideo(video_opt.value()) == Some(caller)) {
                                         auto video = video_opt.value();
                                         thiz->HideVideo(video);
                                         thiz->SetCallerOfVideo(video, None());
                                         FuncCall(thiz->on_hangup_, caller, i);
                                     }
                                 }
                             });
        
        set_accept_checker([thiz, num_people]
                           (const std::string & caller,
                            const std::function<void (bool,
                                                      const Optional<std::vector<std::string> > &)> & helper)
                           {
                               for (int i = 0; i < num_people; i++) {
                                   auto video = thiz->GetIthVideo(i);
                                   if (video && thiz->VideoIsFree(video.value())) {
                                       helper(true, None());
                                       return;
                                   }
                               }
                               helper(false, None());
                           });
        
        set_stream_acceptor([thiz, num_people]
                            (const std::string & caller,
                             const std::shared_ptr<MediaStream> & stream,
                             const std::string & stream_name)
                            {
                                FuncCall(thiz->debug_printer_, "stream acceptor called");
                                
#warning track original bug fix
//                                if (thiz->refresh_pane_ && thiz->VideoIsFree(thiz->refresh_pane_.value())) {
//                                    thiz->ShowVideo(thiz->refresh_pane_.value(), stream);
////                                    FuncCall(thiz->on_call_, caller, thiz->refresh_pane_.value());
//                                    thiz->refresh_pane_ = None();
//                                    return;
//                                }
                                
                                Optional<ObjcPointer> video_opt;
                                for (int i = 0; i < num_people; i++) {
                                    video_opt = thiz->GetIthVideo(i);
                                    if (video_opt && thiz->GetCallerOfVideo(video_opt.value()) == Some(caller)) {
                                        thiz->ShowVideo(video_opt.value(), stream);
                                        FuncCall(thiz->on_call_, caller, i);
                                        return;
                                    }
                                }
 
                                //
                                // no empty slots, so drop whatever caller we have in the first slot and use that one.
                                //
                                video_opt = thiz->GetIthVideo(0);
                                
                                if (video_opt) {
                                    thiz->Hangup(thiz->GetCallerOfVideo(video_opt.value()).value());
                                    thiz->ShowVideo(video_opt.value(), stream);
                                    FuncCall(thiz->on_call_, caller, 0);
                                }
                                thiz->SetCallerOfVideo(video_opt.value(), Some(caller));

                            });
        
        if (auto_add_close_buttons_) {
//            addControls = function(video) {
//                parentDiv = video.parentNode;
//                setCallerOfVideo(video, "");
//                closeButton = document.createElement("div");
//                closeButton.className = "easyrtc_closeButton";
//                closeButton.onclick = function() {
//                    if (getCallerOfVideo(video)) {
//                        easyrtc.hangup(getCallerOfVideo(video));
//                        hideVideo(video);
//                        setCallerOfVideo(video, "");
//                    }
//                };
//                parentDiv.appendChild(closeButton);
//            };
//            for (i = 0; i < numPEOPLE; i++) {
//                addControls(getIthVideo(i));
//            }
        }
        
        if (video_enabled_ && monitor_video_id) {
#warning open issue about mute control
            auto monitor_video = GetElementById(monitor_video_id.value());
            if (!monitor_video) {
                printf("Programmer error: no object called %d\n", monitor_video_id.value());
                return;
            }
//            monitorVideo.muted = "muted";
//            monitorVideo.defaultMuted = true;
        }
        
    }
    
    ObjcPointer Easyrtc::GetElementById(ElementId id) {
        UIView * work_view = (UIView *)ObjcPointerGet(work_view_);
        for (UIView * view in work_view.subviews) {
            if (view.tag == id) {
                return ObjcPointerMake(view);
            }
        }
        return nullptr;
    }
    
    bool Easyrtc::ValidateVideoIds(const ElementId & monitor_video_id,
                                   const std::vector<ElementId> & video_ids)
    {
        if (!GetElementById(monitor_video_id)) {
            ShowError(err_codes_DEVELOPER_ERR_,
                      nwr::Format("The monitor video id passed to easyApp was bad, saw %d", monitor_video_id));
            return false;
        }
        
        for (const auto & i : video_ids) {
            if (!GetElementById(i)) {
                ShowError(err_codes_DEVELOPER_ERR_,
                          nwr::Format("The caller video id %d passed to easyApp was bad.", i));
                return false;
            }
        }
        
        return true;
    }
    
    Optional<std::string> Easyrtc::GetCallerOfVideo(const ObjcPointer & video_object) {
        if (HasKey(video_id_to_caller_map_, video_object)) {
            return video_id_to_caller_map_[video_object];
        } else {
            return None();
        }
    }
    void Easyrtc::SetCallerOfVideo(const ObjcPointer & video_object,
                                   const Optional<std::string> & caller_easyrtcid) {
        video_id_to_caller_map_[video_object] = caller_easyrtcid;
    }
    
    bool Easyrtc::VideoIsFree(const ObjcPointer & obj) {
        auto caller = GetCallerOfVideo(obj);
        return !caller;
    }
    
    void Easyrtc::set_on_call(const std::function<void(const std::string &, const int &)> & cb) {
        on_call_ = cb;
    }
    
    void Easyrtc::set_on_hangup(const std::function<void(const std::string &, int)> & cb) {
        on_hangup_ = cb;
    }
    
    Optional<ObjcPointer> Easyrtc::GetIthVideo(int i) {
        if (0 <= i && i < video_ids_.size()) {
            return Some(GetElementById(video_ids_[i]));
        }
        return None();
    }
    
    Optional<std::string> Easyrtc::GetIthCaller(int i) {
        if (i < 0 || i >= video_ids_.size()) {
            return None();
        }
        auto vid = GetIthVideo(i);
        if (!vid) { return None(); }
        return GetCallerOfVideo(vid.value());
    }
    
    int Easyrtc::GetSlotOfCaller(const std::string & easyrtcid) {
        for (int i = 0; i < video_ids_.size(); i++) {
            if (GetIthCaller(i) == Some(easyrtcid)) {
                return i;
            }
        }
        return -1; // caller not connected
    }
    
    void Easyrtc::HideVideo(const ObjcPointer & video) {
        NWRHtmlMediaElementView * video_o = (NWRHtmlMediaElementView *)ObjcPointerGet(video);
        SetVideoObjectSrc(video, nullptr);
        video_o.hidden = YES;
    }
    
    void Easyrtc::ShowVideo(const ObjcPointer & video, const std::shared_ptr<MediaStream> & stream) {
        NWRHtmlMediaElementView * video_o = (NWRHtmlMediaElementView *)ObjcPointerGet(video);
        SetVideoObjectSrc(video, stream);
        video_o.hidden = NO;
    }
    
    void Easyrtc::EasyApp(const std::string & application_name,
                          const Optional<ElementId> & monitor_video_id,
                          const std::vector<ElementId> & video_ids,
                          const std::function<void(const std::string &)> & on_ready,
                          const std::function<void(const std::string &,
                                                   const std::string &)> & on_failure)
    {
        auto thiz = shared_from_this();
        
//        var gotMediaCallback = null,
//        gotConnectionCallback = null;
        EasyAppBody(monitor_video_id, video_ids);
        
        //
        // try to restablish broken connections that weren't caused by a hangup
        //
        
        set_peer_closed_listener([thiz](const std::string & easyrtcid){
            Timer::Create(TimeDuration(1.0), [thiz, easyrtcid](){
                if (thiz->GetSlotOfCaller(easyrtcid) >= 0 && thiz->IsPeerInAnyRoom(easyrtcid)) {
                    thiz->Call(easyrtcid,
                               None(),
                               nullptr,
                               nullptr,
                               nullptr);
                }
            });
        });
        
        
        /** Sets an event handler that gets called when a connection to the signaling
         * server has or has not been made. Can only be called after calling easyrtc.easyApp.
         * @param {Function} gotConnectionCB has the signature (gotConnection, errorText)
         * @example
         */
        
        auto next_initialization_step = [thiz, on_ready](const std::string & id){
            FuncCall(thiz->got_connection_callback_, true, None());
            on_ready(thiz->my_easyrtcid_.value());
        };
        
        auto post_get_user_media = [thiz, application_name, monitor_video_id, on_failure,
                                    next_initialization_step]
        (){
            FuncCall(thiz->got_media_callback_, true, None());

            if (monitor_video_id) {
                thiz->SetVideoObjectSrc(thiz->GetElementById(monitor_video_id.value()), thiz->GetLocalStream(None()));
            }
            
            auto connect_error = [thiz, application_name,  on_failure](const std::string & error_code, const std::string & error_text) {
                if (thiz->got_connection_callback_) {
                    thiz->got_connection_callback_(false, Some(error_text));
                }
                else if (on_failure) {
                    on_failure(thiz->err_codes_CONNECT_ERR_, error_text);
                }
                else {
                    thiz->ShowError(thiz->err_codes_CONNECT_ERR_, error_text);
                }
            };
            
            thiz->Connect(application_name,
                          next_initialization_step,
                          connect_error);
        };
        
        auto stream = GetLocalStream(None());
        if (stream) {
            post_get_user_media();
        }
        else {
            InitMediaSource(None(),
                            [post_get_user_media](const std::shared_ptr<MediaStream> &)
                            {
                                post_get_user_media();
                            },
                            [thiz, on_failure](const std::string & error_code, const std::string & error_text)
                            {
                                if (thiz->got_media_callback_) {
                                    thiz->got_media_callback_(false, Some(error_text));
                                }
                                else if (on_failure) {
                                    on_failure(thiz->err_codes_MEDIA_ERR_, error_text);
                                }
                                else {
                                    thiz->ShowError(thiz->err_codes_MEDIA_ERR_, error_text);
                                }
                            });
        }
    }
    
    void Easyrtc::set_got_media(const std::function<void(bool, const Optional<std::string> &)> & callback) {
        got_media_callback_ = callback;
    }
    
    void Easyrtc::set_got_connection(const std::function<void(bool, const Optional<std::string> &)> & callback) {
        got_connection_callback_ = callback;
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
    
    void Easyrtc::SetPeerConn(const std::string & other_user, const std::shared_ptr<PeerConn> & peer_conn) {
        if (HasKey(peer_conns_, other_user)) {
            printf("[warning] peer conn %s override\n", other_user.c_str());
            DeletePeerConn(other_user);
        }
        peer_conns_[other_user] = peer_conn;
    }
    
    void Easyrtc::DeletePeerConn(const std::string & other_user) {
        if (HasKey(peer_conns_, other_user)) {
            peer_conns_[other_user]->Close();
            peer_conns_.erase(other_user);
        }
    }
    
}
}
