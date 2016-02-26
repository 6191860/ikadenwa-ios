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
#include <nwr/base/time.h>
#include <nwr/base/map.h>
#include <nwr/base/optional.h>
#include <nwr/base/timer.h>
#include <nwr/base/json.h>
#include <nwr/base/func.h>
#include <nwr/base/any.h>
#include <nwr/base/any_func.h>
#include <nwr/base/any_emitter.h>
#include <nwr/base/objc_pointer.h>
#include <nwr/base/lib_webrtc.h>
#include <nwr/engineio/packet.h>
#include <nwr/socketio0/io.h>
#include <nwr/jsrtc/media_track_constraints.h>
#include <nwr/jsrtc/media_stream_constraints.h>
#include <nwr/jsrtc/media_stream.h>
#include <nwr/jsrtc/media_stream_track.h>
#include <nwr/jsrtc/rtc_session_description.h>
#include <nwr/jsrtc/rtc_ice_candidate.h>
#include <nwr/jsrtc/rtc_peer_connection.h>
#include <nwr/jsrtc/rtc_peer_connection_factory.h>

#include "peer_conn.h"
#include "receive_peer.h"
#include "aggregating_timer.h"
#include "websocket_listener_entry.h"
#include "fields.h"

namespace nwr {
namespace ert {
    using namespace jsrtc;
    
    class Easyrtc: public std::enable_shared_from_this<Easyrtc> {
    public:
        friend PeerConn;
        //  work_view: UIView
        static std::shared_ptr<Easyrtc> Create(const std::string & server_path,
                                               const ObjcPointer & work_view);
        ~Easyrtc();
        
    private:
        Easyrtc();
        void Init(const std::string & server_path,
                  const ObjcPointer & work_view);
#warning todo clear all vars , especially check remove all retain cycles.
        void Close();
    private:
        Any CreateIceServer(const std::string & url, const std::string & username, const std::string & credential);
        bool auto_init_user_media_;
        std::function<std::string(const std::string &)> sdp_local_filter_;
        std::function<std::string(const std::string &)> sdp_remote_filter_;
        int connection_option_timeout_;
        bool connection_option_force_new_connection_;
        void StopStream(const std::shared_ptr<MediaStream> & stream);
        void set_sdp_filters(const std::function<std::string(const std::string &)> & local_filter,
                             const std::function<std::string(const std::string &)> & remote_filter);
        std::function<void(const std::string &)> on_peer_closed_;
        void set_peer_closed_listener(const std::function<void(const std::string &)> & handler);
        std::function<void(const std::string &)> on_peer_failing_;
        std::function<void(const std::string &,
                           const std::chrono::system_clock::time_point &,
                           const std::chrono::system_clock::time_point &)> on_peer_recovered_;
        void set_peer_failing_listener(const std::function<void(const std::string &)> & failing_handler,
                                       const std::function<void(const std::string &,
                                                                const std::chrono::system_clock::time_point &,
                                                                const std::chrono::system_clock::time_point &)> & recovered_handler);
        std::function<Any(const Any &, bool)> ice_candidate_filter_;
        void set_ice_candidate_filter(const std::function<Any(const Any &, bool)> & filter);
        void set_auto_init_user_media(bool flag);
    public:
        std::string Format(const std::string & format, const std::vector<std::string> & args);
    private:
        bool IsSocketConnected(const std::shared_ptr<const sio0::Socket> & socket);
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
        static std::string err_codes_NOVIABLEICE_;
        static std::string err_codes_SIGNAL_ERROR_;

        std::string api_version_;
        Any ack_message_;
        std::regex username_regexp_;
        std::string cookie_id_;
        Optional<std::string> username_;
        bool logging_out_;
        bool disconnecting_;
        std::map<std::string, Any> session_fields_;
        MediaTrackConstraints received_media_constraints_;
        void EnableAudioReceive(bool value);
        void EnableVideoReceive(bool value);
        //  GetSourcesList
        //  GetAudioSourceList
        //  GetVideoSourceList
        std::string data_channel_name_;
        std::function<void (const std::string &)> debug_printer_;
        Optional<std::string> my_easyrtcid_;
        Any old_config_;
        std::map<std::string, Any> offers_pending_;
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
        Optional<MediaStreamConstraints> preset_media_constraints_;
        MediaStreamConstraints GetUserMediaConstraints();
        std::string application_name_;
        void set_application_name(const std::string & application_name);
    public:
        void EnableDebug(bool enable);
    private:
        Optional<std::string> presence_show_;
        Optional<std::string> presence_status_;
        bool SupportsGetUserMedia();
        bool SupportsPeerConnections();
        std::shared_ptr<RtcPeerConnection>
        CreateRtcPeerConnection(const webrtc::PeerConnectionInterface::RTCConfiguration & configuration,
                                const MediaTrackConstraints * constraints);
        webrtc::DataChannelInit GetDataChannelConstraints();
        std::string server_path_;
        std::map<std::string, Any> last_logged_in_list_;
        ReceivePeer receive_peer_;
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
    public:
        void ShowError(const std::string & message_code, const std::string & message);
    private:
        //  CreateObjectURL
        std::string CleanId(const std::string & id_string);
        std::function<void(bool, const std::string &)> room_entry_listener_;
        void set_room_entry_listener(const std::function<void(bool, const std::string &)> & handler);
        std::function<void(const Optional<std::string> &,
                           const std::map<std::string, Any> &,
                           const Any &)> room_occupant_listener_;
    public:
        void set_room_occupant_listener(const std::function<void(const Optional<std::string> &,
                                                                 const std::map<std::string, Any> &,
                                                                 const Any &)> & listener);
    private:
        std::function<void(const std::string &, bool)> on_data_channel_open_;
        void set_data_channel_open_listener(const std::function<void(const std::string &, bool)> & listener);
        std::function<void(const std::string &)> on_data_channel_close_;
        void set_data_channel_close_listener(const std::function<void(const std::string &)> & listener);
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
                               const std::vector<std::shared_ptr<MediaStreamTrack>> & tracks);
        std::map<std::string, std::shared_ptr<MediaStream>> named_local_media_streams_;
        std::shared_ptr<MediaStream> GetLocalMediaStreamByName(const Optional<std::string> & stream_name);
        std::vector<std::string> GetLocalMediaIds();
        std::map<std::string, Any> BuildMediaIds();
        std::map<std::string, Any> room_data_;
        void RegisterLocalMediaStreamByName(const std::shared_ptr<MediaStream> & stream,
                                            const Optional<std::string> & stream_name);
        void Register3rdPartyLocalMediaStream(const std::shared_ptr<MediaStream> & stream,
                                              const Optional<std::string> & stream_name);
        Optional<std::string> GetNameOfRemoteStream(const std::string & easyrtcid,
                                                    const Optional<std::string> & webrtc_stream_id);
        void CloseLocalMediaStreamByName(const Optional<std::string> & stream_name);
        void EnableCamera(bool enable, const Optional<std::string> & stream_name);
        void EnableMicrophone(bool enable, const Optional<std::string> & stream_name);
        //  muteVideoObject (DOM)
        //  getLocalStreamAsUrl ; for <video>, <canvas>
        std::shared_ptr<MediaStream> GetLocalStream(const Optional<std::string> & stream_name);
        
        //  NWRHtmlMediaElementView
        void ClearMediaStream(const ObjcPointer & element);
        void SetVideoObjectSrc(const ObjcPointer & video_object, const std::shared_ptr<MediaStream> & stream);
        std::shared_ptr<MediaStream>
        BuildLocalMediaStream(const std::string & stream_name,
                              const std::vector<std::shared_ptr<MediaStreamTrack>> & audio_tracks,
                              const std::vector<std::shared_ptr<MediaStreamTrack>> & video_tracks);
        //  loadStylesheet
        std::string FormatError(const Any & error);
        
        void InitMediaSource(const Optional<std::string> & stream_name,
                             const std::function<void(const std::shared_ptr<MediaStream> &)> & success_callback,
                             const std::function<void(const std::string &,
                                                      const std::string &)> & error_callback);
        std::function<void (const std::string &,
                            const std::function<void (bool,
                                                      const Optional<std::vector<std::string>> &)> &)> accept_check_;
        void set_accept_checker(const std::function<void (const std::string &,
                                                          const std::function<void (bool,
                                                                                    const Optional<std::vector<std::string>> &)> &)> & accept_check);
        std::function<void(const std::string &,
                           const std::shared_ptr<MediaStream> &,
                           const std::string &)> stream_acceptor_;
        void set_stream_acceptor(const std::function<void(const std::string &,
                                                          const std::shared_ptr<MediaStream> &,
                                                          const std::string &)> & acceptor);
        std::function<void(const Any &)> on_error_;
        void set_on_error(const std::function<void(const Any &)> & err_listener);
        std::function<void (const std::string &, bool)> call_canceled_;
        void set_call_canceled(const std::function<void(const std::string &, bool)> & call_canceled);
        std::function<void (const std::string &,
                            const std::shared_ptr<MediaStream> &,
                            const std::string &)> on_stream_closed_;
        void set_on_stream_closed(const std::function<void(const std::string &,
                                                           const std::shared_ptr<MediaStream> &,
                                                           const std::string &)> & on_stream_closed);
        //  setVideoBandwidth ; deprecated in original
        bool SupportsDataChannels();
    public:
        void SetPeerListener(const Optional<std::string> & msg_type,
                             const Optional<std::string> & source,
                             const ReceivePeerCallback & listener);
    private:
        void ReceivePeerDistribute(const std::string & easyrtcid,
                                   const Any & msg,
                                   const Any & targeting);
        std::function<void(const std::string &,
                           const Any &,
                           const Any &)> receive_server_cb_;
        void set_server_listener(const std::function<void(const std::string &,
                                                          const Any &,
                                                          const Any &)> & listener);
        sio0::SocketOptions connection_options_;
        void set_socket_url(const std::string & socket_url,
                            const Optional<sio0::SocketOptions> & options);
        bool set_user_name(const std::string & username);
        std::vector<std::tuple<std::string, std::string>> UsernameToIds(const std::string & username,
                                                                        const Optional<std::string> & room);
        Any GetRoomApiField(const std::string & room_name,
                            const std::string & easyrtcid,
                            const std::string & field_name);
        Optional<std::string> credential_;
        void set_credential(const Any & credential_param);
        std::function<void()> disconnect_listener_;
        void set_disconnect_listener(const std::function<void()> & disconnect_listener);
        std::string IdToName(const std::string & easyrtcid);
        std::shared_ptr<sio0::Socket> websocket_;
        std::shared_ptr<webrtc::PeerConnectionInterface::RTCConfiguration> pc_config_;
        std::shared_ptr<webrtc::PeerConnectionInterface::RTCConfiguration> pc_config_to_use_;
        bool use_fresh_ice_each_peer_;
        void set_use_fresh_ice_each_peer_connection(bool value);
        std::shared_ptr<webrtc::PeerConnectionInterface::RTCConfiguration> server_ice();
        //  setIceUsedInCalls
        std::shared_ptr<sio0::Socket> closed_channel_;
        bool HaveTracks(const Optional<std::string> & easyrtcid,
                        bool check_audio,
                        const Optional<std::string> & stream_name);
        bool HaveAudioTrack(const Optional<std::string> & easyrtcid, const Optional<std::string> & stream_name);
        bool HaveVideoTrack(const Optional<std::string> & easyrtcid, const Optional<std::string> & stream_name);
        Any GetRoomField(const std::string & room_name, const std::string & field_name);
        bool SupportsStatistics();
        Fields fields_;
        bool IsEmptyObj(const Any & obj);
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
                               const std::function<void(const std::string &,
                                                        const Any &)> & success_cb,
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
        MediaTrackConstraints BuildPeerConstraints();
        void Call(const std::string & other_user,
                  const std::function<void(const std::string &,
                                           const std::string &)> & call_success_cb,
                  const std::function<void(const std::string &,
                                           const std::string &)> & call_failure_cb,
                  const std::function<void(bool, const std::string &)> & was_accepted_cb,
                  const Optional<std::vector<std::string>> & stream_names);
        void CallBody(const std::string & other_user,
                      const std::function<void(const std::string &,
                                               const std::string &)> & call_success_cb,
                      const std::function<void(const std::string &,
                                               const std::string &)> & call_failure_cb,
                      const std::function<void(bool, const std::string &)> & was_accepted_cb,
                      const Optional<std::vector<std::string>> & stream_names);
        void HangupBody(const std::string & other_user);
        void Hangup(const std::string & other_user);
        void HangupAll();
        bool DoesDataChannelWork(const std::string & other_user);
        std::shared_ptr<MediaStream>
        GetRemoteStream(const std::string & easyrtcid,
                        const std::string & remote_stream_name);
        void MakeLocalStreamFromRemoteStream(const std::string & easyrtcid,
                                             const std::string & remote_stream_name,
                                             const std::string & local_stream_name);
        void AddStreamToCall(const std::string & easyrtcid,
                             const Optional<std::string> & arg_stream_name,
                             const std::function<void(const std::string &,
                                                      const std::string &)> & receipt_handler);
        void SetupPeerListener1();
        void SetupPeerListener2();
        void SetupPeerListener3();
        void OnRemoveStreamHelper(const std::string & easyrtcid,
                                  const std::shared_ptr<MediaStream> & stream);
        void DumpPeerConnectionInfo();
        std::map<std::string, bool> turn_servers_;
        std::shared_ptr<RtcPeerConnection>
        BuildPeerConnection(const std::string & other_user,
                            bool is_initiator,
                            const std::function<void(const std::string &,
                                                     const std::string &)> & failure_cb,
                            const Optional<std::vector<std::string>> & stream_names);
        void DoAnswer(const std::string & caller,
                      const Any & msg_data,
                      const Optional<std::vector<std::string> > &stream_names);
        void DoAnswerBody(const std::string & caller, const Any & msg_data,
                          const Optional<std::vector<std::string>> & stream_names);
        void EmitOnStreamClosed(const std::string & easyrtcid,
                                const std::shared_ptr<MediaStream> &stream);
        void OnRemoteHangup(const std::string & caller);
        std::map<std::string, Any> queued_messages_;
        void ClearQueuedMessages(const std::string & caller);
        bool IsPeerInAnyRoom(const std::string & id);
        void ProcessLostPeers(const std::map<std::string, Any> & peers_in_room);
        std::map<std::string, AggregatingTimer> aggregating_timers_;
        void AddAggregatingTimer(const std::string & key,
                                 const std::function<void()> & callback,
                                 const Optional<TimeDuration> & arg_period);
        void ProcessOccupantList(const std::string & room_name,
                                 std::map<std::string, Any> & occupant_list);
        void SendQueuedCandidates(const std::string & peer,
                                  const std::function<void (const std::string &,
                                                            const Any &)> & on_signal_success,
                                  const std::function<void (const std::string &,
                                                            const std::string &)> & on_signal_failure);
        void OnChannelMsg(const Any & msg,
                          const std::function<void(const Any &)> & ack_acceptor_func);
        void OnChannelCmd(const Any & msg,
                          const std::function<void(const Any &)> & ack_acceptor_fn);
        std::vector<WebsocketListenerEntry> websocket_listeners_;
        void ConnectToWSServer(const std::function<void(const std::string &)> & success_callback,
                               const std::function<void(const std::string &,
                                                        const std::string &)> & error_callback);
        Any BuildDeltaRecord(const Any & added, const Any & deleted);
        Any FindDeltas(const Any & old_version, const Any & new_version);
        Any CollectConfigurationInfo(bool forAuthentication);
        void UpdateConfiguration();
        void UpdateConfigurationInfo();
        std::function<void()> update_configuration_info_;
        void UpdatePresence(const std::string & state, const std::string & status_text);
        std::map<std::string, Any> GetSessionFields();
        Any GetSessionField(const std::string & name);
        Optional<std::string> easyrtcsid_;
        void ProcessSessionData(const Any & session_data);
        void ProcessRoomData(const Any & room_data);
        std::vector<std::string> GetRoomOccupantsAsArray(const std::string & room_name);
        std::map<std::string, Any> GetRoomOccupantsAsMap(const std::string & room_name);
        bool IsTurnServer(const std::string & ip_address);
        void ProcessIceConfig(const Any & arg_ice_config);
        void GetFreshIceConfig(const std::function<void(bool)> & callback);
        void ProcessToken(const Any & msg);
        void SendAuthenticate(const std::function<void (const std::string &)> & success_callback,
                              const std::function<void (const std::string &,
                                                        const std::string &)> & error_callback);
        std::map<std::string, bool> GetRoomsJoined();
        std::map<std::string, Any> GetRoomFields(const std::string & room_name);
        std::map<std::string, Any> GetApplicationFields();
        std::map<std::string, Any> GetConnectionFields();
        std::shared_ptr<sio0::Socket> preallocated_socket_io_;
        void UseThisSocketConnection(const std::shared_ptr<sio0::Socket> & already_allocated_socket_io);
    public:
        void Connect(const std::string & application_name,
                     const std::function<void(const std::string &)> & success_callback,
                     const std::function<void(const std::string &,
                                              const std::string &)> & arg_error_callback);
    private:
        bool auto_add_close_buttons_;
        void DontAddCloseButtons();
        
        using ElementId = int;
        void EasyAppBody(const Optional<ElementId> & monitor_video_id,
                         const std::vector<ElementId> & video_ids);
        // search view in work view not recursively
        ObjcPointer GetElementById(ElementId id);
        std::vector<ElementId> video_ids_;
        std::map<ObjcPointer, Optional<std::string>> video_id_to_caller_map_;
        bool ValidateVideoIds(const ElementId & monitor_video_id,
                              const std::vector<ElementId> & video_ids);
        Optional<std::string> GetCallerOfVideo(const ObjcPointer & video_object);
        void SetCallerOfVideo(const ObjcPointer & video_object, const Optional<std::string> & caller_easyrtcid);
        bool VideoIsFree(const ObjcPointer & obj);
        std::function<void(const std::string &, const int &)> on_call_;
        void set_on_call(const std::function<void(const std::string &, const int &)> & cb);
        std::function<void(const std::string &, int)> on_hangup_;
        void set_on_hangup(const std::function<void(const std::string &, int)> & cb);
        Optional<ObjcPointer> GetIthVideo(int i);
        Optional<std::string> GetIthCaller(int i);
        int GetSlotOfCaller(const std::string & easyrtcid);
        void HideVideo(const ObjcPointer & video);
        void ShowVideo(const ObjcPointer & video, const std::shared_ptr<MediaStream> & stream);
        Optional<ObjcPointer> refresh_pane_;
        void EasyApp(const std::string & application_name,
                     const Optional<ElementId> & monitor_video_id,
                     const std::vector<ElementId> & video_ids,
                     const std::function<void(const std::string &)> & on_ready,
                     const std::function<void(const std::string &,
                                              const std::string &)> & on_failure);
        std::function<void(bool, const Optional<std::string> &)> got_media_callback_;
        void set_got_media(const std::function<void(bool, const Optional<std::string> &)> & callback);
        std::function<void(bool, const Optional<std::string> &)> got_connection_callback_;
        void set_got_connection(const std::function<void(bool, const Optional<std::string> &)> & callback);
        static std::map<std::string, std::string> constant_strings_;

        // ---
        bool closed_;
        std::shared_ptr<RtcPeerConnectionFactory> peer_connection_factory_;
        ObjcPointer work_view_;
        void SetPeerConn(const std::string & other_user, const std::shared_ptr<PeerConn> & peer_conn);
        void DeletePeerConn(const std::string & other_user);
    };
}
}
