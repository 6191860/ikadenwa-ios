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
        
        connection_options_.timeout = TimeDuration(10.0);
        connection_options_.force_new = true;
        
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
        old_config_ = 0;
        offers_pending_ = 0;
        native_video_height_ = 0;
        max_p2p_message_length_ = 1000;
        native_video_width_ = 0;
        desired_video_properties_ = Any(Any::ObjectType {
        });
        application_name_ = "";
        data_enabled_ = false;
        on_error_ = FuncMake([this](const Any & error) {
            FuncCall(debug_printer_,
                     "saw error" + (error.GetAt("errorText").AsString() || std::string("")));
            printf("[Easyrtc::on_error_] %s\n", error.ToJsonString().c_str());
        });
        use_fresh_ice_each_peer_ = false;
        
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
                                 thiz->ShowError(ack_msg.GetAt("msgData").GetAt("errorCode").AsString() || std::string(),
                                                 ack_msg.GetAt("msgData").GetAt("errorText").AsString() || std::string());
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
    
    void Easyrtc::set_room_occupant_listener(const Func<void(const Optional<std::string> &,
                                                             const Any &,
                                                             bool)> & listener) {
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
    
    Any Easyrtc::BuildMediaIds() {
        std::map<std::string, Any> media_map;
        for (auto iter : named_local_media_streams_) {
            auto id = iter.second->label();
            media_map[iter.first] = Any(std::string(id != "" ? id : "default"));
        }
        return Any(media_map);
    }
    
    void Easyrtc::RegisterLocalMediaStreamByName(webrtc::MediaStreamInterface & stream,
                                                 const Optional<std::string> & arg_stream_name)
    {
        std::string stream_name = arg_stream_name || std::string("default");
//        stream.streamName = streamName;
        named_local_media_streams_[stream_name] = rtc::scoped_refptr<webrtc::MediaStreamInterface>(&stream);
        
        if (stream_name != "default") {
            auto media_ids = BuildMediaIds();
            for (const auto & i : room_data_) {
                SetRoomApiField(i.first, "mediaIds", media_ids);
            }
        }
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
        
        rtc::scoped_refptr<webrtc::MediaStreamInterface> stream = GetLocalStream(stream_name);
        if (!stream) {
            return;
        }
        
        std::string stream_id = stream->label();
        
        if (HasKey(named_local_media_streams_, stream_name)) {
            for (const auto & id : Keys(peer_conns_)) {
                peer_conns_[id]->pc()->RemoveStream(stream.get());
                SendPeerMessage(Any(id), "__closingMediaStream", Any(Any::ObjectType {
                    { "streamId", Any(stream_id) },
                    { "streamName", Any(stream_name) }
                }), nullptr, nullptr);
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
    
    void Easyrtc::EnableCamera(bool enable, const Optional<std::string> & stream_name) {
        auto stream = GetLocalMediaStreamByName(stream_name);
        if (stream) {
            EnableMediaTracks(enable, ToMediaStreamTrackVector(stream->GetVideoTracks()));
        }
    }
    
    void Easyrtc::EnableMicrophone(bool enable, const Optional<std::string> & stream_name) {
        auto stream = GetLocalMediaStreamByName(stream_name);
        if (stream) {
            EnableMediaTracks(enable, ToMediaStreamTrackVector(stream->GetAudioTracks()));
        }
    };
    
  
    //  how to clone track?
#if 0
    rtc::scoped_refptr<webrtc::MediaStreamInterface>
    Easyrtc::BuildLocalMediaStream(const std::string & stream_name,
                                   const webrtc::AudioTrackVector & audio_tracks,
                                   const webrtc::VideoTrackVector & video_tracks)
    {
        webrtc::MediaStreamInterface * stream_to_clone_ = nullptr;
        for (const auto & key : Keys(named_local_media_streams_)) {
            stream_to_clone_ = named_local_media_streams_[key].get();
            if (stream_to_clone_) { break; }
        }
        
        if (!stream_to_clone_) {
            for (const auto & key : Keys(peer_conns_)) {
                auto remote_streams = peer_conns_[key].pc->remote_streams();
                // bug? : if( remoteStreams && remoteStreams.length > 1 ) {
                if (remote_streams->count() > 0) {
                    stream_to_clone_ = remote_streams->at(0);
                }
            }
        }
        
        if (!stream_to_clone_) {
            ShowError(err_codes_DEVELOPER_ERR_,
                      "Attempt to create a mediastream without one to clone from");
            return nullptr;
        }
        
        //
        // clone whatever mediastream we found, and remove any of it's
        // tracks.
        //
        
        auto media_clone = webrtc::MediaStream::Create(stream_name);

        for (auto track : audio_tracks) {
            media_clone->AddTrack(track.get());
        }
        
        for (auto track : video_tracks) {
            media_clone->AddTrack(track.get());
        }
        
        RegisterLocalMediaStreamByName(media_clone, stream_name);
        return media_clone;
    }
#endif
    
    std::string Easyrtc::FormatError(const Any & error) {
        return error.ToJsonString();
    }
    
    void Easyrtc::InitMediaSource(const std::function<void(webrtc::MediaStreamInterface &)> & success_callback,
                                  const std::function<void(const std::string &,
                                                           const std::string &)> & arg_error_callback,
                                  const Optional<std::string> & arg_stream_name)
    {
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
        
        auto on_user_media_success = FuncMake
        ([thiz, stream_name, success_callback]
         (webrtc::MediaStreamInterface & stream){
             FuncCall(thiz->debug_printer_, "getUserMedia success callback entered");
             FuncCall(thiz->debug_printer_, "successfully got local media");
             
             //            stream.streamName = streamName;
             thiz->RegisterLocalMediaStreamByName(stream, Some(stream_name));
             
             if (thiz->have_video_) {
                 //                videoObj = document.createElement('video');
                 //                videoObj.muted = true;
                 //                triesLeft = 30;
                 //                tryToGetSize = function() {
                 //                    if (videoObj.videoWidth > 0 || triesLeft < 0) {
                 //
                 //                        self.nativeVideoWidth = videoObj.videoWidth; [TODO]
                 //                        self.nativeVideoHeight = videoObj.videoHeight; [TODO]
                 //                        if (self._desiredVideoProperties.height &&
                 //                            (self.nativeVideoHeight !== self._desiredVideoProperties.height ||
                 //                             self.nativeVideoWidth !== self._desiredVideoProperties.width)) {
                 //                                [TODO]
                 //                                self.showError(self.errCodes.MEDIA_WARNING,
                 //                                               self.format(self.getConstantString("resolutionWarning"),
                 //                                                           self._desiredVideoProperties.width, self._desiredVideoProperties.height,
                 //                                                           self.nativeVideoWidth, self.nativeVideoHeight));
                 //                            }
                 //                        self.setVideoObjectSrc(videoObj, "");
                 //                        if (videoObj.removeNode) {
                 //                            videoObj.removeNode(true);
                 //                        }
                 //                        else {
                 //                            ele = document.createElement('div');
                 //                            ele.appendChild(videoObj);
                 //                            ele.removeChild(videoObj);
                 //                        }
                 //
                 //                        updateConfigurationInfo(); [TODO]
                 //                        if (successCallback) {
                 //                            successCallback(stream); [TODO]
                 //                        }
                 //                    }
                 //                    else {
                 //                        triesLeft -= 1;
                 //                        setTimeout(tryToGetSize, 300); [TODO]
                 //                    }
                 //                };
                 //                self.setVideoObjectSrc(videoObj, stream);
                 //                tryToGetSize();
                 
#warning TODO
                 
                 FuncCall(thiz->update_configuration_info_);
                 if (success_callback) {
                     success_callback(stream);
                 }
             }
             else {
                 FuncCall(thiz->update_configuration_info_);
                 if (success_callback) {
                     success_callback(stream);
                 }
             }
         });
        
#warning todo
//        var onUserMediaError = function(error) {
//            console.log("getusermedia failed");
//            if (self.debugPrinter) {
//                self.debugPrinter("failed to get local media");
//            }
//            var errText;
//            if (typeof error === 'string') {
//                errText = error;
//            }
//            else if (error.name) {
//                errText = error.name;
//            }
//            else {
//                errText = "Unknown";
//            }
//            if (errorCallback) {
//                console.log("invoking error callback", errText);
//                errorCallback(self.errCodes.MEDIA_ERR, self.format(self.getConstantString("gumFailed"), errText));
//            }
//            closeLocalMediaStreamByName(streamName);
//            haveAudioVideo = {
//            audio: false,
//            video: false
//            };
//            updateConfigurationInfo();
//        };
//        if (!self.audioEnabled && !self.videoEnabled) {
//            onUserMediaError(self.getConstantString("requireAudioOrVideo"));
//            return;
//        }
//        
//        function getCurrentTime() {
//            return (new Date()).getTime();
//        }
//        
//        var firstCallTime;
//        function tryAgain(error) {
//            var currentTime = getCurrentTime();
//            if (currentTime < firstCallTime + 1000) {
//                console.log("Trying getUserMedia a second time");
//                setTimeout(function() {
//                    getUserMedia(mode, onUserMediaSuccess, onUserMediaError);
//                }, 3000);
//            }
//            else {
//                onUserMediaError(error);
//            }
//        }
        
        if (video_enabled_ || audio_enabled_) {
            //
            // getUserMedia sometimes fails the first time I call it. I suspect it's a page loading
            // issue. So I'm going to try adding a 3 second delay to allow things to settle down first.
            // In addition, I'm going to try again after 3 seconds.
            //
            
//            
//            
//            setTimeout(function() {
//                try {
//                    firstCallTime = getCurrentTime();
//                    getUserMedia(mode, onUserMediaSuccess, tryAgain);[TODO]
//                } catch (e) {
//                    tryAgain(e);
//                }
//            }, 1000);
        }
        else {
            Fatal("bug?");
//            FuncCall(on_user_media_success, nullptr);
        }
    };
    
    void Easyrtc::set_accept_checker(const Func<void()> & accept_check) {
        accept_check_ = accept_check;
    }
    
    void Easyrtc::set_stream_acceptor(const Func<void()> & acceptor) {
        stream_acceptor_ = acceptor;
    }
    
    void Easyrtc::set_on_error(const Func<void(const Any &)> & err_listener) {
        on_error_ = err_listener;
    }
    
    void Easyrtc::set_call_canceled(const Func<void()> & call_canceled) {
        call_cancelled_ = call_canceled;
    }
    
    void Easyrtc::set_on_stream_closed(const Func<void()> & on_stream_closed) {
        on_stream_closed_ = on_stream_closed;
    }
    
    bool Easyrtc::SupportsDataChannels() {
        return true;
    }
    
    void Easyrtc::set_peer_listener(const ReceivePeerCallback & listener,
                                    const Optional<std::string> & msg_type,
                                    const Optional<std::string> & source)
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
                (*msg_entry.sources[easyrtcid].cb)(easyrtcid, msg_type, msg_data, targeting);
                return;
            }
            if (msg_entry.cb) {
                (*msg_entry.cb)(easyrtcid, msg_type, msg_data, targeting);
                return;
            }
        }
        if (receive_peer_.cb) {
            (*receive_peer_.cb)(easyrtcid, msg_type, msg_data, targeting);
        }
    }
    
    void Easyrtc::set_server_listener(const Func<void()> & listener) {
        receive_server_cb_ = listener;
    }
    
    void Easyrtc::set_socket_url(const std::string & socket_url,
                                 const Optional<eio::Socket::ConstructorParams> & options)
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
            username_ = username;
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
        
        for (const auto & room_name : Keys(last_loggged_in_list_)) {
            if (room && room_name != *room) {
                continue;
            }
            
            for (const std::string & id : Keys(last_loggged_in_list_[room_name])) {
                if (last_loggged_in_list_[room_name][id].username == Some(username)) {
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
        if (HasKey(last_loggged_in_list_, room_name) &&
            HasKey(last_loggged_in_list_[room_name], easyrtcid))
        {
            auto & info = last_loggged_in_list_[room_name][easyrtcid];
            if (HasKey(info.api_field, field_name)) {
                return info.api_field[field_name].GetAt("fieldValue");
            }
        }
        return nullptr;
    }
    
    void Easyrtc::set_credential(const Any & credential_param) {
        credential_ = credential_param.ToJsonString();
    }
    
    void Easyrtc::set_disconnect_listener(const Func<void()> & disconnect_listener) {
        disconnect_listener_ = disconnect_listener;
    }
    
    std::string Easyrtc::IdToName(const std::string & easyrtcid) {
        for (const std::string & room_name : Keys(last_loggged_in_list_)) {
            if (HasKey(last_loggged_in_list_[room_name], easyrtcid)) {
                auto & entry = last_loggged_in_list_[room_name][easyrtcid];
                if (entry.username) {
                    return *entry.username;
                }
            }
        }
        return easyrtcid;
    }
    
    void Easyrtc::set_use_fresh_ice_each_peer_connection(bool value) {
        use_fresh_ice_each_peer_ = value;
    }
    
    MediaConstraints Easyrtc::server_ice() {
        return pc_config_;
    }
    
    bool Easyrtc::HaveTracks(const Optional<std::string> & easyrtcid,
                             bool check_audio,
                             const Optional<std::string> & stream_name)
    {
        
        rtc::scoped_refptr<webrtc::MediaStreamInterface> stream;
        
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
        
        MediaStreamTrackVector tracks;
        if (check_audio) {
            tracks = ToMediaStreamTrackVector(stream->GetAudioTracks());
        }
        else {
            tracks = ToMediaStreamTrackVector(stream->GetVideoTracks());
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
        Any fields = GetRoomFields(room_name);
        return fields.GetAt(field_name).GetAt("fieldValue");
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
        offers_pending_ = 0;
        acceptance_pending_.clear();
        disconnecting_ = true;
        closed_channel_ = websocket_;
        if (websocket_connected_) {
            if (!preallocated_socket_io_) {
                websocket_->Close();
            }
            websocket_connected_ = false;
        }
        HangupAll();
        if (room_occupant_listener_) {
            for (const auto & key : Keys(last_loggged_in_list_)) {
                (*room_occupant_listener_)(Some(key), Any(Any::ObjectType{}), false);
            }
        }
        last_loggged_in_list_.clear();
        
        EmitEvent("roomOccupant", Any(Any::ObjectType{}));
        room_data_.clear();
        room_join_.clear();
        logging_out_ = false;
        my_easyrtcid_ = None();
        disconnecting_ = false;
        old_config_ = 0;
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
                thiz->websocket_->Close();
                
                thiz->closed_channel_ = thiz->websocket_;
                thiz->websocket_ = nullptr;
            }
            thiz->logging_out_ = false;
            thiz->disconnecting_ = false;
            
            if (thiz->room_occupant_listener_) {
                (*thiz->room_occupant_listener_)(None(), Any(Any::ObjectType{}), false);
            }
            
            thiz->EmitEvent("roomOccupant", Any(Any::ObjectType{}));
            thiz->old_config_ = 0;
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

            websocket_->Emit("easyrtcCmd", { data_to_ship },
                             [thiz, success_callback, error_callback]
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
        
        peer_conns_[dest_user]->data_channel_s()->Send(webrtc::DataBuffer(start_message.ToJsonString()));
        
        int pos = 0;
        int len = static_cast<int>(msg_data.length());
        for (; pos < len; pos += max_p2p_message_length_) {
            Any message(Any::ObjectType {
                { "transfer_id", Any(transfer_id) },
                { "data", Any(msg_data.substr(pos, max_p2p_message_length_)) },
                { "transfer", Any("chunk") }
            });

            peer_conns_[dest_user]->data_channel_s()->Send(webrtc::DataBuffer(message.ToJsonString()));
        }
        
        peer_conns_[dest_user]->data_channel_s()->Send(webrtc::DataBuffer(end_message.ToJsonString()));
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
                peer_conns_[dest_user]->data_channel_s()->Send(webrtc::DataBuffer(flattened_data));
            }
        }
    }
    
    void Easyrtc::SendDataWS(const Any & destination,
                             const std::string & msg_type,
                             const Any & msg_data,
                             const std::function<void(const Any &)> & arg_ack_handler)
    {
        auto thiz = shared_from_this();
        
        std::function<void(const Any &)> ack_handler = arg_ack_handler;
        
        FuncCall(debug_printer_,
                 std::string("sending client message via websockets to ") + destination.ToJsonString() +
                 " with data=" + msg_data.ToJsonString());
        
        if (!ack_handler) {
            ack_handler = [thiz](const Any & msg){
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
            websocket_->Emit("easyrtcMsg", { outgoing_message }, ack_handler);
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
                                    const std::function<void()> & success_cb,
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
    
    MediaConstraints Easyrtc::BuildPeerConstraints() {
        webrtc::MediaConstraintsInterface::Constraints options;
        
        //  TODO: google
        //        options.push({'DtlsSrtpKeyAgreement': 'true'}); // for interoperability
        
        return MediaConstraints(webrtc::MediaConstraintsInterface::Constraints(),
                                options);
    }
    
    void Easyrtc::Call(const std::string & other_user,
                       const std::function<void()> & call_success_cb,
                       const std::function<void(const std::string &,
                                                const std::string &)> & call_failure_cb,
                       const std::function<void(bool)> & was_accepted_cb,
                       const Optional<std::vector<std::string>> & stream_names)
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
                
                InitMediaSource([thiz, other_user, call_success_cb, call_failure_cb, was_accepted_cb]
                                (webrtc::MediaStreamInterface & stream){
                                    thiz->Call(other_user,
                                               call_success_cb,
                                               call_failure_cb,
                                               was_accepted_cb,
                                               None());
                                },
                                call_failure_cb,
                                None());
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
            FuncCall(was_accepted_cb, true);
            DoAnswer(other_user, offers_pending_[other_user], stream_names);
            offers_pending_.erase(other_user);
            CallCanceled(other_user, false);
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
                           const std::function<void()> & call_success_cb,
                           const std::function<void(const std::string &,
                                                    const std::string &)> & call_failure_cb,
                           const std::function<void(bool)> & was_accepted_cb,
                           const Optional<std::vector<std::string>> & stream_names)
    {
        acceptance_pending_[other_user] = true;
        
        auto pc = BuildPeerConnection(other_user, true, call_failure_cb, stream_names);

        if (!pc) {
            std::string message("buildPeerConnection failed, call not completed");
            FuncCall(debug_printer_, message);
            Fatal(message);
        }
        
        peer_conns_[other_user]->set_call_success_cb(call_success_cb);
        
        peerConns[otherUser].callSuccessCB = callSuccessCB;
        peerConns[otherUser].callFailureCB = callFailureCB;
        peerConns[otherUser].wasAcceptedCB = wasAcceptedCB;
        var peerConnObj = peerConns[otherUser];
        var setLocalAndSendMessage0 = function(sessionDescription) {
            if (peerConnObj.cancelled) {
                return;
            }
            var sendOffer = function() {
                
                sendSignalling(otherUser, "offer", sessionDescription, null, callFailureCB);
            };
            if (sdpLocalFilter) {
                sessionDescription.sdp = sdpLocalFilter(sessionDescription.sdp);
            }
            pc.setLocalDescription(sessionDescription, sendOffer,
                                   function(errorText) {
                                       callFailureCB(self.errCodes.CALL_ERR, errorText);
                                   });
        };
        setTimeout(function() {
            //
            // if the call was cancelled, we don't want to continue getting the offer.
            // we can tell the call was cancelled because there won't be a peerConn object
            // for it.
            //
            if( !peerConns[otherUser]) {
                return;
            }
            pc.createOffer(setLocalAndSendMessage0, function(errorObj) {
                callFailureCB(self.errCodes.CALL_ERR, JSON.stringify(errorObj));
            },
                           receivedMediaConstraints);
        }, 100);
    }
    
    
    
    
    
    
    
    Any Easyrtc::GetRoomFields(const std::string & room_name) {
        return fields_.GetAt("rooms").GetAt(room_name);
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
