//
//  rtc_peer_connection.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/16.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "rtc_peer_connection.h"

#include "media_track_constraints.h"
#include "media_stream.h"
#include "rtc_data_channel.h"
#include "rtc_session_description.h"
#include "rtc_ice_candidate.h"

namespace nwr {
namespace jsrtc {
    
    RtcPeerConnection::~RtcPeerConnection() {
        Close();
    }
    
    void RtcPeerConnection::
    CreateOffer(const std::shared_ptr<MediaTrackConstraints> & options,
                const std::function<void(const std::shared_ptr<RtcSessionDescription> &)> & success,
                const std::function<void(const std::string &)> & failure)
    {
        rtc::scoped_refptr<CreateSessionDescriptionObserver>
        observer(new CreateSessionDescriptionObserver(shared_from_this(),
                                                      success,
                                                      failure));
        
        inner_connection_->CreateOffer(observer.get(),
                                       options ? &options->inner_constraints() : nullptr);
    }
    
    void RtcPeerConnection::
    CreateAnswer(const std::shared_ptr<MediaTrackConstraints> & options,
                 const std::function<void(const std::shared_ptr<RtcSessionDescription> &)> & success,
                 const std::function<void(const std::string &)> & failure)
    {
        rtc::scoped_refptr<CreateSessionDescriptionObserver>
        observer(new CreateSessionDescriptionObserver(shared_from_this(),
                                                      success,
                                                      failure));
        
        inner_connection_->CreateAnswer(observer.get(),
                                        options ? &options->inner_constraints() : nullptr);
    }
    
    void RtcPeerConnection::
    SetLocalDescription(const std::shared_ptr<const RtcSessionDescription> & description,
                        const std::function<void()> & success,
                        const std::function<void(const std::string &)> & failure)
    {
        auto thiz = shared_from_this();
        
        pending_local_description_ = description;
        
        auto wdesc = description->CreateWebrtc();
        
        rtc::scoped_refptr<SetSessionDescriptionObserver>
        observer(new SetSessionDescriptionObserver(shared_from_this(),
                                                   [thiz, description, success]() {
                                                       thiz->pending_local_description_ = nullptr;
                                                       thiz->current_local_description_ = description;
                                                       FuncCall(success);
                                                   },
                                                   [thiz, failure](const std::string & err){
                                                       thiz->pending_local_description_ = nullptr;
                                                       FuncCall(failure, err);
                                                   }));
        
        inner_connection_->SetLocalDescription(observer.get(), wdesc);
    }
    
    std::shared_ptr<const RtcSessionDescription> RtcPeerConnection::
    local_description() {
        auto desc = pending_local_description();
        if (desc) { return desc; }
        return current_local_description();
    }
    
    std::shared_ptr<const RtcSessionDescription> RtcPeerConnection::current_local_description() {
        return current_local_description_;
    }
    std::shared_ptr<const RtcSessionDescription> RtcPeerConnection::pending_local_description() {
        return pending_local_description_;
    }
    
    void RtcPeerConnection::
    SetRemoteDescription(const std::shared_ptr<const RtcSessionDescription> & description,
                         const std::function<void()> & success,
                         const std::function<void(const std::string &)> & failure)
    {
        auto thiz = shared_from_this();
        
        pending_remote_description_ = description;
        
        auto wdesc = description->CreateWebrtc();
        
        rtc::scoped_refptr<SetSessionDescriptionObserver>
        observer(new SetSessionDescriptionObserver(shared_from_this(),
                                                   [thiz, description, success]() {
                                                       thiz->pending_remote_description_ = nullptr;
                                                       thiz->current_remote_description_ = description;
                                                       FuncCall(success);
                                                   },
                                                   [thiz, failure](const std::string & err) {
                                                       thiz->pending_remote_description_ = nullptr;
                                                       FuncCall(failure, err);
                                                   }));
        
        inner_connection_->SetRemoteDescription(observer.get(), wdesc);
    }
    
    std::shared_ptr<const RtcSessionDescription> RtcPeerConnection::
    remote_description() {
        auto desc = pending_remote_description();
        if (desc) { return desc; }
        return current_remote_description();
    }
    
    std::shared_ptr<const RtcSessionDescription> RtcPeerConnection::current_remote_description() {
        return current_remote_description_;
    }
    
    std::shared_ptr<const RtcSessionDescription> RtcPeerConnection::pending_remote_description() {
        return pending_remote_description_;
    }
    
    void RtcPeerConnection::AddIceCandidate(const std::shared_ptr<const RtcIceCandidate> & candidate) {
        rtc::scoped_ptr<webrtc::IceCandidateInterface> wcand(candidate->CreateWebrtc());
        
        bool ok = inner_connection_->AddIceCandidate(wcand.get());
        if (!ok) { Fatal("AddIceCandidate failed"); }
    }
    
    webrtc::PeerConnectionInterface::SignalingState RtcPeerConnection::signaling_state() {
        return inner_connection_->signaling_state();
    }
    webrtc::PeerConnectionInterface::IceGatheringState RtcPeerConnection::ice_gathering_state() {
        return inner_connection_->ice_gathering_state();
    }
    webrtc::PeerConnectionInterface::IceConnectionState RtcPeerConnection::ice_connection_state() {
        return inner_connection_->ice_connection_state();
    }
    
    void RtcPeerConnection::OnClose() {
        if (inner_connection_) {
            inner_connection_->Close();
            inner_connection_ = nullptr;
        }
        inner_observer_ = nullptr;
        
        current_local_description_ = nullptr;
        pending_local_description_ = nullptr;
        current_remote_description_ = nullptr;
        pending_remote_description_ = nullptr;
        on_negotiation_needed_ = nullptr;
        on_ice_candidate_ = nullptr;
        on_signaling_state_change_ = nullptr;
        on_ice_connection_state_change_ = nullptr;
        on_ice_gathering_state_change_ = nullptr;
        on_data_channel_ = nullptr;
        
        local_streams_.clear();
        remote_streams_.clear();
        on_add_stream_ = nullptr;
        on_remove_stream_ = nullptr;
    }
    
    void RtcPeerConnection::
    set_on_negotiation_needed(const std::function<void()> & value) {
        on_negotiation_needed_ = value;
    }
    
    void RtcPeerConnection::
    set_on_ice_candidate(const std::function<void(const std::shared_ptr<RtcIceCandidate> &)> & value) {
        on_ice_candidate_ = value;
    }
    
    void RtcPeerConnection::
    set_on_signaling_state_change(const std::function<void(webrtc::PeerConnectionInterface::SignalingState)> & value) {
        on_signaling_state_change_ = value;
    }
    
    void RtcPeerConnection::
    set_on_ice_connection_state_change(const std::function<void(webrtc::PeerConnectionInterface::IceConnectionState)> & value) {
        on_ice_connection_state_change_ = value;
    }
    
    void RtcPeerConnection::
    set_on_ice_gathering_state_change(const std::function<void(webrtc::PeerConnectionInterface::IceGatheringState)> & value) {
        on_ice_gathering_state_change_ = value;
    }
    
    std::vector<rtc::scoped_refptr<webrtc::RtpSenderInterface>>
    RtcPeerConnection::senders() {
        return inner_connection_->GetSenders();
    }
    std::vector<rtc::scoped_refptr<webrtc::RtpReceiverInterface>> RtcPeerConnection::receivers() {
        return inner_connection_->GetReceivers();
    }
    rtc::scoped_refptr<webrtc::RtpSenderInterface>
    RtcPeerConnection::AddTrack(webrtc::MediaStreamTrackInterface * track,
                                std::vector<webrtc::MediaStreamInterface*> streams)
    {
        return inner_connection_->AddTrack(track, streams);
    }
    bool RtcPeerConnection::RemoveTrack(webrtc::RtpSenderInterface* sender) {
        return inner_connection_->RemoveTrack(sender);
    }
    
    std::shared_ptr<RtcDataChannel> RtcPeerConnection::CreateDataChannel(const std::string & label,
                                                                         const webrtc::DataChannelInit * config)
    {
        auto inner_channel = inner_connection_->CreateDataChannel(label, config);
        return std::make_shared<RtcDataChannel>(*inner_channel);
    }
    void RtcPeerConnection::set_on_data_channel(const std::function<void(const std::shared_ptr<RtcDataChannel> &)> & value) {
        on_data_channel_ = value;
    }
    
    
    std::vector<std::shared_ptr<MediaStream>> RtcPeerConnection::local_streams() {
        return local_streams_;
    }
    std::vector<std::shared_ptr<MediaStream>> RtcPeerConnection::remote_streams() {
        return remote_streams_;
    }
    void RtcPeerConnection::AddStream(const std::shared_ptr<MediaStream> & stream) {
        bool ok = inner_connection_->AddStream(&stream->inner_stream());
        if (!ok) { Fatal("AddStream failed"); }
        local_streams_.push_back(stream);
    }
    void RtcPeerConnection::RemoveStream(const std::shared_ptr<MediaStream> & stream) {
        inner_connection_->RemoveStream(&stream->inner_stream());
        Remove(local_streams_, stream);
    }
    void RtcPeerConnection::set_on_add_stream(const std::function<void(const std::shared_ptr<MediaStream> &)> & value) {
        on_add_stream_ = value;
    }
    void RtcPeerConnection::set_on_remove_stream(const std::function<void(const std::shared_ptr<MediaStream> &)> & value) {
        on_remove_stream_ = value;
    }
    
    RtcPeerConnection::InnerObserver::
    InnerObserver(const std::shared_ptr<RtcPeerConnection> & owner):
    owner(owner)
    {}
    
    void RtcPeerConnection::InnerObserver::
    OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state)
    {
        owner->Post([new_state](const RtcPeerConnection & owner) {
            FuncCall(owner.on_signaling_state_change_, new_state);
        });
    }
    
    void RtcPeerConnection::InnerObserver::
    OnAddStream(webrtc::MediaStreamInterface* inner_stream)
    {
        auto stream = std::make_shared<MediaStream>(*inner_stream);
        owner->Post([stream](RtcPeerConnection & owner){
            owner.remote_streams_.push_back(stream);
            FuncCall(owner.on_add_stream_, stream);
        });
    }
    void RtcPeerConnection::InnerObserver::
    OnRemoveStream(webrtc::MediaStreamInterface* arg_inner_stream)
    {
        rtc::scoped_refptr<webrtc::MediaStreamInterface> inner_stream(arg_inner_stream);
        owner->Post([inner_stream](RtcPeerConnection & owner) {
            auto stream = owner.FindRemoteStreamByInnerStream(inner_stream.get());
            Remove(owner.remote_streams_, stream);
            FuncCall(owner.on_remove_stream_, stream);
        });
    }
    void RtcPeerConnection::InnerObserver::
    OnDataChannel(webrtc::DataChannelInterface* inner_channel)
    {
        auto channel = std::make_shared<RtcDataChannel>(*inner_channel);
        owner->Post([channel](RtcPeerConnection & owner) {
            FuncCall(owner.on_data_channel_, channel);
        });
    }
    void RtcPeerConnection::InnerObserver::
    OnRenegotiationNeeded()
    {
        owner->Post([](RtcPeerConnection & owner) {
            FuncCall(owner.on_negotiation_needed_);
        });
    }
    void RtcPeerConnection::InnerObserver::
    OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state)
    {
        owner->Post([new_state](RtcPeerConnection & owner) {
            FuncCall(owner.on_ice_connection_state_change_, new_state);
        });
    }
    void RtcPeerConnection::InnerObserver::
    OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state)
    {
        owner->Post([new_state](RtcPeerConnection & owner) {
            FuncCall(owner.on_ice_gathering_state_change_, new_state);
        });
    }
    void RtcPeerConnection::InnerObserver::
    OnIceCandidate(const webrtc::IceCandidateInterface* arg_candidate)
    {
        std::shared_ptr<RtcIceCandidate> candidate = RtcIceCandidate::FromWebrtc(*arg_candidate);
        if (!candidate) { Fatal("invalid"); }
        
        owner->Post([candidate](RtcPeerConnection & owner) {
            FuncCall(owner.on_ice_candidate_, candidate);
        });
    }
    
    void RtcPeerConnection::InnerObserver::OnIceConnectionReceivingChange(bool receiving)
    {
        
    }
    
    RtcPeerConnection::CreateSessionDescriptionObserver::
    CreateSessionDescriptionObserver(const std::shared_ptr<RtcPeerConnection> & owner,
                                     const std::function<void(const std::shared_ptr<RtcSessionDescription> &)> &  success,
                                     const std::function<void(const std::string &)> & failure):
    owner(owner),
    success(success),
    failure(failure)
    {}

    void RtcPeerConnection::CreateSessionDescriptionObserver::
    OnSuccess(webrtc::SessionDescriptionInterface* arg_desc) {
        rtc::scoped_refptr<CreateSessionDescriptionObserver> thiz(this);
        std::shared_ptr<RtcSessionDescription> desc = RtcSessionDescription::FromWebrtc(*arg_desc);
        if (!desc) { Fatal("invalid"); }
        
        owner->Post([thiz, desc](RtcPeerConnection & owner) {
            FuncCall(thiz->success, desc);
        });
    }
    void RtcPeerConnection::CreateSessionDescriptionObserver::
    OnFailure(const std::string& error) {
        rtc::scoped_refptr<CreateSessionDescriptionObserver> thiz(this);

        owner->Post([thiz, error](RtcPeerConnection & owner) {
            FuncCall(thiz->failure, error);
        });
    }
    
    RtcPeerConnection::SetSessionDescriptionObserver::
    SetSessionDescriptionObserver(const std::shared_ptr<RtcPeerConnection> & owner,
                                  const std::function<void()> & success,
                                  const std::function<void(const std::string &)> & failure):
    owner(owner),
    success(success),
    failure(failure)
    {}
    
    void RtcPeerConnection::SetSessionDescriptionObserver::
    OnSuccess() {
        rtc::scoped_refptr<SetSessionDescriptionObserver> thiz(this);
        
        owner->Post([thiz](RtcPeerConnection & owner) {
            FuncCall(thiz->success);
        });
    }
    void RtcPeerConnection::SetSessionDescriptionObserver::
    OnFailure(const std::string& error) {
        rtc::scoped_refptr<SetSessionDescriptionObserver> thiz(this);
        
        owner->Post([thiz, error](RtcPeerConnection & owner) {
            FuncCall(thiz->failure, error);
        });
    }
    
    
    RtcPeerConnection::RtcPeerConnection()
    {}
    
    void RtcPeerConnection::Init(webrtc::PeerConnectionInterface & inner_connection,
                                 const std::shared_ptr<InnerObserver> & inner_observer)
    {
        inner_connection_ = rtc::scoped_refptr<webrtc::PeerConnectionInterface>(&inner_connection);
        inner_observer_ = inner_observer;
    }
    
    std::shared_ptr<MediaStream> RtcPeerConnection::
    FindRemoteStreamByInnerStream(webrtc::MediaStreamInterface * inner_stream)
    {
        int index = IndexOfIf(remote_streams_, [inner_stream](const std::shared_ptr<MediaStream> & stream) {
            return &stream->inner_stream() == inner_stream;
        });
        if (index == -1) { return nullptr; }
        return remote_streams_[index];
    }
    

}
}