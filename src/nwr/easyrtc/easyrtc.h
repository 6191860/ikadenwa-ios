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

#include <webrtc/base/scoped_ptr.h>
#include <webrtc/base/thread.h>
#include <talk/app/webrtc/peerconnectioninterface.h>

#include <nwr/base/string.h>
#include <nwr/base/array.h>
#include <nwr/base/optional.h>
#include <nwr/base/json.h>
#include <nwr/base/any.h>
#include <nwr/base/any_emitter.h>
#include <nwr/socketio/socket.h>

#include "media_constraints.h"

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
        void StopStream(int stream);
        void set_sdp_filters(int local_filter, int remote_filter);
        void set_peer_closed_listener(const std::shared_ptr<std::function<void()>> & handler);
        void set_peer_failing_listener(const std::shared_ptr<std::function<void()>> & failing_handler,
                                       const std::shared_ptr<std::function<void()>> & recovered_handler);
        void set_ice_candidate_filter(int filter);
        void set_auto_init_user_media(bool flag);
    public:
        std::string Format(const std::string & format, const std::vector<std::string> & args);
    private:
        bool IsSocketConnected(const std::shared_ptr<sio::Socket> & socket);
        std::string GetConstantString(const std::string & key);
        void CheckEvent(const std::string & event_name, const std::string & calling_function);
        void AddEventListener(const std::string & event_name, const AnyEventListener & event_listener);
        void RemoveEventListener(const std::string & event_name, const AnyEventListener & event_listener);
        void EmitEvent(const std::string & event_name, const Any & event_data);
        void EnableAudioReceive(bool value);
        void EnableVideoReceive(bool value);
        //  GetSourcesList
        //  GetAudioSourceList
        //  GetVideoSourceList
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
        //  set_video_source
        void set_video_dims(int width, int height, const Optional<double> & frame_rate);
        //  set_screen_capture
        VideoAudioMediaConstraints GetUserMediaConstraints();
        void set_application_name(const std::string & application_name);
        void EnableDebug(bool enable);
        
        
        void ProcessRoomData(const Any & room_data);
        void ShowError(const std::string & message_code, const std::string & message);
        void SendSignalling(const Optional<std::string> & dest_user,
                            const std::string & msg_type,
                            const Any & msg_data,
                            const std::function<void (const std::string &, const Any &)> & success_callback,
                            const std::function<void (const std::string &, const std::string &)> & error_callback);
        void UpdatePresence(const std::string & state, const std::string & status_text);
        bool SupportsGetUserMedia();
        bool SupportsPeerConnections();
        
        rtc::scoped_refptr<webrtc::PeerConnectionInterface>
        CreateRtcPeerConnection(const PeerConnectionInterface::RTCConfiguration & configuration,
                                const MediaConstraintsInterface * constraints,
                                PeerConnectionObserver * observer)
        
        //  GetDataChannelConstraints
        void Disconnect();
        
        
        bool closed_;
        rtc::scoped_ptr<rtc::Thread> rtc_signaling_thread_;
        rtc::scoped_ptr<rtc::Thread> rtc_worker_thread_;
        rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory_;

        

        bool auto_init_user_media_;
        int sdp_local_filter_;
        int sdp_remote_filter_;
        int ice_candidate_filter_;
        int connection_option_timeout_;
        bool connection_option_force_new_connection_;
        std::shared_ptr<std::function<void()>> on_peer_closed_;
        std::shared_ptr<std::function<void()>> on_peer_failing_;
        std::shared_ptr<std::function<void()>> on_peer_recovered_;
        bool have_audio_;
        bool have_video_;
        std::vector<std::string> allowed_events_;
        std::map<std::string, std::vector<AnyEventListener>> event_listeners_;

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
        std::map<std::string, int> named_local_media_streams_;
        std::vector<std::string> session_fields_;
        Any received_media_constraints_;
        bool audio_enabled_;
        bool video_enabled_;
        std::string data_channel_name_;
        std::function<void (const std::string &)> debug_printer_;
        std::string my_easyrtcid_;
        int old_config_;
        int offers_pending_;
        int native_video_height_;
        int max_p2p_message_length_;
        int native_video_width_;
        int credential_;
        std::map<std::string, Any> room_join_;
        Any desired_video_properties_;
        Optional<VideoAudioMediaConstraints> preset_media_constraints_;
        std::string application_name_;
        std::string presence_show_;
        std::string presence_status_;
        bool data_enabled_;
        std::string server_path_;
        int room_occupant_listener_;
        int on_data_channel_open_;
        int on_data_channel_close_;
        int last_loggged_in_list_;
        int receive_peer_;
        int receive_server_cb_;
        std::function<void ()> update_configuration_info_;
        std::map<std::string, PeerConn> peer_conns_;
        std::vector<std::string> acceptance_pending_;
        std::function<void ()> accept_check_;
        std::function<void ()> stream_acceptor_;
        std::function<void ()> on_stream_closed_;
        std::function<void ()> call_cancelled_;

        
        std::shared_ptr<sio::Socket> web_socket_;
        std::shared_ptr<std::function<void(const Any &)>> on_error_;
        
        
        
        static std::map<std::string, std::string> constant_strings_;
    };
}
}
