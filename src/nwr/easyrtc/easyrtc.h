//
//  easyrtc.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/12.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <vector>
#include <string>
#include <map>
#include <regex>
#include <memory>
#include <functional>

#include <nwr/base/string.h>
#include <nwr/base/array.h>
#include <nwr/base/map.h>
#include <nwr/base/optional.h>
#include <nwr/base/timer.h>
#include <nwr/base/json.h>
#include <nwr/base/func.h>
#include <nwr/base/any.h>
#include <nwr/base/any_emitter.h>
#include <nwr/socketio/socket.h>
#include <nwr/base/lib_webrtc.h>

#include "media_constraints.h"
#include "media_stream_track.h"
#include "peer_conn.h"

namespace nwr {
namespace ert {
    class Easyrtc: public std::enable_shared_from_this<Easyrtc> {
    public:
    private:
        Easyrtc();
        void Init();
    public:
        ~Easyrtc();
        void Close();
    private:
        bool auto_init_user_media_;
        int sdp_local_filter_;
        int sdp_remote_filter_;
        int ice_candidate_filter_;
        int connection_option_timeout_;
        bool connection_option_force_new_connection_;
        void StopStream(webrtc::MediaStreamInterface * stream);
        void set_sdp_filters(int local_filter, int remote_filter);
        Func<void()> on_peer_closed_;
        void set_peer_closed_listener(const Func<void()> & handler);
        Func<void()> on_peer_failing_;
        Func<void()> on_peer_recovered_;
        void set_peer_failing_listener(const Func<void()> & failing_handler,
                                       const Func<void()> & recovered_handler);
        void set_ice_candidate_filter(int filter);
        void set_auto_init_user_media(bool flag);
    public:
        std::string Format(const std::string & format, const std::vector<std::string> & args);
    private:
        bool IsSocketConnected(const std::shared_ptr<sio::Socket> & socket);
        bool have_audio_;
        bool have_video_;
        std::string GetConstantString(const std::string & key);
        std::vector<std::string> allowed_events_;
        std::map<std::string, std::vector<AnyEventListener>> event_listeners_;
        void CheckEvent(const std::string & event_name, const std::string & calling_function);
        void AddEventListener(const std::string & event_name, const AnyEventListener & event_listener);
        void RemoveEventListener(const std::string & event_name, const AnyEventListener & event_listener);
        void EmitEvent(const std::string & event_name, const Any & event_data);
        static std::string err_codes_BAD_NAME_;
        static std::string err_codes_CALL_ERR_;
        static std::string err_codes_DEVELOPER_ERR_;
        static std::string err_codes_SYSTEM_ERR_;
        static std::string err_codes_CONNECT_ERR_;
        static std::string err_codes_MEDIA_ERR_;
        static std::string err_codes_MEDIA_WARNING_;
        static std::string err_codes_INTERNAL_ERR_;
        static std::string err_codes_PEER_GONE_;
        static std::string err_codes_ALREADY_CONNECTED_;
        static std::string err_codes_BAD_CREDENTIAL_;
        static std::string err_codes_ICECANDIDATE_ERROR_;
        std::string api_version_;
        Any ack_message_;
        std::regex username_regexp_;
        std::string cookie_id_;
        std::string username_;
        bool logging_out_;
        bool disconnecting_;
        std::vector<std::string> session_fields_;
        Any received_media_constraints_;
        void EnableAudioReceive(bool value);
        void EnableVideoReceive(bool value);
        //  GetSourcesList
        //  GetAudioSourceList
        //  GetVideoSourceList
        std::string data_channel_name_;
        Func<void (const std::string &)> debug_printer_;
        std::string my_easyrtcid_;
        int old_config_;
        int offers_pending_;
        int native_video_height_;
        int native_video_width_;
        int credential_;
        std::map<std::string, Any> room_join_;
        bool IsNameValid(const std::string & name);
        void set_cookie_id(const std::string & cookie_id);
        void JoinRoom(const std::string & room_name,
                      const Any & room_parameters,
                      const std::function<void(const std::string &)> & success_cb,
                      const std::function<void(const std::string &,
                                               const std::string &,
                                               const std::string &)> & failure_cb);
        void LeaveRoom(const std::string & room_name,
                       const std::function<void(const std::string &)> & success_callback,
                       const std::function<void(const std::string &,
                                                const std::string &,
                                                const std::string &)> & failure_callback);
        Any desired_video_properties_;
        //  set_video_source
        void set_video_dims(int width, int height, const Optional<double> & frame_rate);
        //  set_screen_capture
        Optional<VideoAudioMediaConstraints> preset_media_constraints_;
        VideoAudioMediaConstraints GetUserMediaConstraints();
        std::string application_name_;
        void set_application_name(const std::string & application_name);
        void EnableDebug(bool enable);
        std::string presence_show_;
        std::string presence_status_;
        void UpdatePresence(const std::string & state, const std::string & status_text);
        bool SupportsGetUserMedia();
        bool SupportsPeerConnections();
        
        rtc::scoped_refptr<webrtc::PeerConnectionInterface>
        CreateRtcPeerConnection(const webrtc::PeerConnectionInterface::RTCConfiguration & configuration,
                                const webrtc::MediaConstraintsInterface * constraints,
                                webrtc::PeerConnectionObserver * observer);
        
        //  GetDataChannelConstraints
        std::string server_path_;
        int last_loggged_in_list_;
        int receive_peer_;
        int receive_server_cb_;
        Func<void ()> update_configuration_info_;
        std::map<std::string, PeerConn> peer_conns_;
        std::vector<std::string> acceptance_pending_;
        void Disconnect();
        Func<void ()> accept_check_;
        Func<void ()> stream_acceptor_;
        Func<void ()> on_stream_closed_;
        Func<void ()> call_cancelled_;
        //  GetPeerStatistics
        std::map<std::string, std::map<std::string, Any>> room_api_fields_;
        bool websocket_connected_;
        void SetRoomApiField(const std::string & room_name);
        void SetRoomApiField(const std::string & room_name,
                             const std::string & field_name);
        void SetRoomApiField(const std::string & room_name,
                             const std::string & field_name,
                             const Any & field_value);
        TimerPtr room_api_field_timer_;
        void EnqueueSendRoomApi(const std::string & room_name);
        void SendRoomApiFields(const std::string & roomName,
                               const std::map<std::string, Any> & fields);
        void ShowError(const std::string & message_code, const std::string & message);
        Func<void(const Any &)> on_error_;
        //  CreateObjectURL
        std::string CleanId(const std::string & id_string);
        Func<void()> room_entry_listener_;
        void set_room_entry_listener(const Func<void()> & handler);
        Func<void()> room_occupant_listener_;
        void set_room_occupant_listener(const Func<void()> & listener);
        Func<void()> on_data_channel_open_;
        void set_data_channel_open_listener(const Func<void()> & listener);
        Func<void()> on_data_channel_close_;
        void set_data_channel_close_listener(const Func<void()> & listener);
        int connection_count();
        
        enum class ConnectStatus {
            NotConnected,
            BecomingConnected,
            IsConnected
        };
        ConnectStatus GetConnectStatus(const std::string & other_user);
        
        int max_p2p_message_length_;
        void set_max_p2p_message_length(int max_length);
        bool audio_enabled_;
        void EnableAudio(bool enabled);
        bool video_enabled_;
        void EnableVideo(bool enabled);
        bool data_enabled_;
        void EnableDataChannels(bool enabled);
        void EnableMediaTracks(bool enable,
                               const std::vector<rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>> & tracks);
        std::map<std::string, rtc::scoped_refptr<webrtc::MediaStreamInterface>> named_local_media_streams_;
        rtc::scoped_refptr<webrtc::MediaStreamInterface> GetLocalMediaStreamByName();
        rtc::scoped_refptr<webrtc::MediaStreamInterface> GetLocalMediaStreamByName(const std::string & stream_name);
        std::vector<std::string> GetLocalMediaIds();
        Any BuildMediaIds();
        std::map<std::string, int> room_data_;
        void RegisterLocalMediaStreamByName(const rtc::scoped_refptr<webrtc::MediaStreamInterface> & stream);
        void RegisterLocalMediaStreamByName(const rtc::scoped_refptr<webrtc::MediaStreamInterface> & stream,
                                            const std::string & stream_name);
        //  register3rdPartyLocalMediaStream
        Optional<std::string> GetNameOfRemoteStream(const std::string & easyrtcid);
        Optional<std::string> GetNameOfRemoteStream(const std::string & easyrtcid, const std::string & webrtc_stream_id);
        void CloseLocalMediaStreamByName();
        void CloseLocalMediaStreamByName(const std::string & stream_name);
        void EnableCamera(bool enable, const std::string & stream_name);
        void EnableMicrophone(bool enable, const std::string & stream_name);
        //  muteVideoObject (DOM)
        //  getLocalStreamAsUrl ; for <video>, <canvas>
        //  clearMediaStream(DOM)
        //  setVideoObjectSrc <video>
        
        
        void SendPeerMessage(const std::string & id, const std::string key, const Any & data);
        
        rtc::scoped_refptr<webrtc::MediaStreamInterface> GetLocalStream(const std::string & stream_name);
        Any GetRoomApiField(const std::string & room_name, const std::string & field_name, const std::string & name);
        
        void SendSignaling(const Optional<std::string> & dest_user,
                           const std::string & msg_type,
                           const Any & msg_data,
                           const std::function<void (const std::string &, const Any &)> & success_callback,
                           const std::function<void (const std::string &, const std::string &)> & error_callback);
        
        void ProcessRoomData(const Any & room_data);
        
        bool closed_;
        rtc::scoped_ptr<rtc::Thread> rtc_signaling_thread_;
        rtc::scoped_ptr<rtc::Thread> rtc_worker_thread_;
        rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory_;

        

        
        std::shared_ptr<sio::Socket> websocket_;
        
        
        
        static std::map<std::string, std::string> constant_strings_;
    };
}
}
