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
#include <cmath>

#include <nwr/base/string.h>
#include <nwr/base/array.h>
#include <nwr/base/map.h>
#include <nwr/base/optional.h>
#include <nwr/base/timer.h>
#include <nwr/base/json.h>
#include <nwr/base/func.h>
#include <nwr/base/any.h>
#include <nwr/base/any_emitter.h>
#include <nwr/socketio/io.h>
#include <nwr/base/lib_webrtc.h>

#include "media_constraints.h"
#include "media_stream_track.h"
#include "peer_conn.h"
#include "receive_peer.h"
#include "logged_in_info.h"

namespace nwr {
namespace ert {
    class Easyrtc: public std::enable_shared_from_this<Easyrtc> {
    public:
        friend PeerConn;
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
        Optional<std::string> my_easyrtcid_;
        int old_config_;
        std::map<std::string, bool> offers_pending_;
        int native_video_height_;
        int native_video_width_;
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
        std::map<std::string, std::map<std::string, LoggedInInfo>> last_loggged_in_list_;
        ReceivePeer receive_peer_;
        Func<void ()> update_configuration_info_;
        std::map<std::string, std::shared_ptr<PeerConn>> peer_conns_;
        std::map<std::string, bool> acceptance_pending_;
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
        //  CreateObjectURL
        std::string CleanId(const std::string & id_string);
        Func<void()> room_entry_listener_;
        void set_room_entry_listener(const Func<void()> & handler);
        Func<void(const Optional<std::string> &,
                  const Any &,
                  bool)> room_occupant_listener_;
        void set_room_occupant_listener(const Func<void(const Optional<std::string> &,
                                                        const Any &,
                                                        bool)> & listener);
        Func<void()> on_data_channel_open_;
        void set_data_channel_open_listener(const Func<void()> & listener);
        Func<void()> on_data_channel_close_;
        void set_data_channel_close_listener(const Func<void()> & listener);
        int connection_count();
        int max_p2p_message_length_;
        void set_max_p2p_message_length(int max_length);
        bool audio_enabled_;
        void EnableAudio(bool enabled);
        bool video_enabled_;
        void EnableVideo(bool enabled);
        bool data_enabled_;
        void EnableDataChannels(bool enabled);
        void EnableMediaTracks(bool enable,
                               const MediaStreamTrackVector & tracks);
        std::map<std::string, rtc::scoped_refptr<webrtc::MediaStreamInterface>> named_local_media_streams_;
        rtc::scoped_refptr<webrtc::MediaStreamInterface> GetLocalMediaStreamByName(const Optional<std::string> & stream_name);
        std::vector<std::string> GetLocalMediaIds();
        Any BuildMediaIds();
        std::map<std::string, int> room_data_;
        void RegisterLocalMediaStreamByName(webrtc::MediaStreamInterface & stream,
                                            const Optional<std::string> & stream_name);
        //  register3rdPartyLocalMediaStream
        Optional<std::string> GetNameOfRemoteStream(const std::string & easyrtcid,
                                                    const Optional<std::string> & webrtc_stream_id);
        void CloseLocalMediaStreamByName(const Optional<std::string> & stream_name);
        void EnableCamera(bool enable, const Optional<std::string> & stream_name);
        void EnableMicrophone(bool enable, const Optional<std::string> & stream_name);
        //  muteVideoObject (DOM)
        //  getLocalStreamAsUrl ; for <video>, <canvas>
        //  clearMediaStream(DOM)
        //  setVideoObjectSrc <video>
        //  loadStylesheet
        std::string FormatError(const Any & error);
        void InitMediaSource(const std::function<void(webrtc::MediaStreamInterface &)> & success_callback,
                             const std::function<void(const std::string &,
                                                      const std::string &)> & arg_error_callback,
                             const Optional<std::string> & stream_name);
        Func<void ()> accept_check_;
        void set_accept_checker(const Func<void()> & accept_check);
        Func<void ()> stream_acceptor_;
        void set_stream_acceptor(const Func<void()> & acceptor);
        Func<void(const Any &)> on_error_;
        void set_on_error(const Func<void(const Any &)> & err_listener);
        Func<void ()> call_cancelled_;
        void set_call_canceled(const Func<void()> & call_canceled);
        Func<void ()> on_stream_closed_;
        void set_on_stream_closed(const Func<void()> & on_stream_closed);
        //  setVideoBandwidth ; deprecated in original
        bool SupportsDataChannels();
        void set_peer_listener(const ReceivePeerCallback & listener,
                               const Optional<std::string> & msg_type,
                               const Optional<std::string> & source);
        void ReceivePeerDistribute(const std::string & easyrtcid,
                                   const Any & msg,
                                   const Any & targeting);
        Func<void()> receive_server_cb_;
        void set_server_listener(const Func<void()> & listener);
        eio::Socket::ConstructorParams connection_options_;
        void set_socket_url(const std::string & socket_url,
                            const Optional<eio::Socket::ConstructorParams> & options);
        bool set_user_name(const std::string & username);
        std::vector<std::tuple<std::string, std::string>> UsernameToIds(const std::string & username,
                                                                        const Optional<std::string> & room);
        Any GetRoomApiField(const std::string & room_name,
                            const std::string & easyrtcid,
                            const std::string & field_name);
        std::string credential_;
        void set_credential(const Any & credential_param);
        Func<void()> disconnect_listener_;
        void set_disconnect_listener(const Func<void()> & disconnect_listener);
        std::string IdToName(const std::string & easyrtcid);
        std::shared_ptr<sio::Socket> websocket_;
        MediaConstraints pc_config_;
        //  MediaConstraints pc_config_to_use_;
        bool use_fresh_ice_each_peer_;
        void set_use_fresh_ice_each_peer_connection(bool value);
        MediaConstraints server_ice();
        //  setIceUsedInCalls
        std::shared_ptr<sio::Socket> closed_channel_;
        bool HaveTracks(const Optional<std::string> & easyrtcid,
                        bool check_audio,
                        const Optional<std::string> & stream_name);
        bool HaveAudioTrack(const Optional<std::string> & easyrtcid, const Optional<std::string> & stream_name);
        bool HaveVideoTrack(const Optional<std::string> & easyrtcid, const Optional<std::string> & stream_name);
        Any GetRoomField(const std::string & room_name, const std::string & field_name);
        bool SupportsStatistics();
        Any fields_;
        bool IsEmptyObj(const Any & obj);
        std::shared_ptr<sio::Socket> preallocated_socket_io_;
        void DisconnectBody();
        void Disconnect();
        void SendSignaling(const Optional<std::string> & dest_user,
                           const std::string & msg_type,
                           const Any & msg_data,
                           const std::function<void (const std::string &,
                                                     const Any &)> & success_callback,
                           const std::function<void (const std::string &,
                                                     const std::string &)> & error_callback);
        int send_by_chunk_uid_counter_;
        void SendByChunkHelper(const std::string & dest_user, const std::string & msg_data);
        void SendDataP2P(const std::string & dest_user,
                         const std::string & msg_type,
                         const Any & msg_data);
        void SendDataWS(const Any & destination,
                        const std::string & msg_type,
                        const Any & msg_data,
                        const std::function<void(const Any &)> & arg_ack_handler);
        void SendData(const std::string & dest_user,
                      const std::string & msg_type,
                      const Any & msg_data,
                      const std::function<void(const Any &)> & ack_handler);
        void SendPeerMessage(const Any & destination,
                             const std::string & msg_type,
                             const Any & msg_data,
                             const std::function<void(const std::string &,
                                                      const Any &)> & success_cb,
                             const std::function<void(const std::string &,
                                                      const std::string &)> & failure_cb);
        void SendServerMessage(const std::string & msg_type,
                               const Any & msg_data,
                               const std::function<void()> & success_cb,
                               const std::function<void(const std::string &,
                                                        const std::string &)> & failure_cb);
        void GetRoomList(const std::function<void(const Any &)> & callback,
                         const std::function<void(const std::string &,
                                                  const std::string &)> & error_callback);
        enum class ConnectStatus {
            NotConnected,
            BecomingConnected,
            IsConnected
        };
        ConnectStatus GetConnectStatus(const std::string & other_user);
        MediaConstraints BuildPeerConstraints();
        void Call(const std::string & other_user,
                  const std::function<void()> & call_success_cb,
                  const std::function<void(const std::string &,
                                           const std::string &)> & call_failure_cb,
                  const std::function<void(bool)> & was_accepted_cb,
                  const Optional<std::vector<std::string>> & stream_names);
        
        
        rtc::scoped_refptr<webrtc::PeerConnectionInterface>
        BuildPeerConnection(const std::string & other_user,
                            bool b,
                            const std::function<void(const std::string &,
                                                     const std::string &)> & call_failure_cb,
                            const Optional<std::vector<std::string>> & stream_names);
        
        void GetFreshIceConfig(const std::function<void(bool)> & cb);
        void DoAnswer(const std::string & other_user, bool a,
                      const Optional<std::vector<std::string>> & stream_names);
        void CallCanceled(const std::string & other_user, bool a);
        
        void HangupAll();
                
        webrtc::MediaStreamInterface * GetLocalStream(const Optional<std::string> & stream_name);
        
        
        
        
        
        void ProcessRoomData(const Any & room_data);
        
        
        Any GetRoomFields(const std::string & room_name);

        static std::map<std::string, std::string> constant_strings_;

        
        
        bool closed_;
        rtc::scoped_ptr<rtc::Thread> rtc_signaling_thread_;
        rtc::scoped_ptr<rtc::Thread> rtc_worker_thread_;
        rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory_;

        

        
        
    };
}
}
