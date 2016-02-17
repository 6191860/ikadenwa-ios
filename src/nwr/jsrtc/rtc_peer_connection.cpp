//
//  rtc_peer_connection.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/16.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "rtc_peer_connection.h"

#include "media_stream.h"
#include "rtc_session_description.h"

namespace nwr {
namespace jsrtc {
    
    void RtcPeerConnection::
    CreateOffer(const webrtc::MediaConstraintsInterface * options,
                const std::function<void(const std::shared_ptr<RtcSessionDescription> &)> & success,
                const std::function<void(const std::string &)> & failure)
    {
        rtc::scoped_refptr<CreateSessionDescriptionObserver>
        observer(new CreateSessionDescriptionObserver(shared_from_this(),
                                                      success,
                                                      failure));
        
        inner_connection_->CreateOffer(observer.get(), options);
    }
    
    void RtcPeerConnection::
    CreateAnswer(const webrtc::MediaConstraintsInterface * options,
                 const std::function<void(const std::shared_ptr<RtcSessionDescription> &)> & success,
                 const std::function<void(const std::string &)> & failure)
    {
        rtc::scoped_refptr<CreateSessionDescriptionObserver>
        observer(new CreateSessionDescriptionObserver(shared_from_this(),
                                                      success,
                                                      failure));
        
        inner_connection_->CreateAnswer(observer.get(), options);
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
    
    void RtcPeerConnection::AddIceCandidate(const webrtc::IceCandidateInterface * candidate) {
        bool ok = inner_connection_->AddIceCandidate(candidate);
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
    
    void RtcPeerConnection::Close() {
        if (closed_) { return; }
        
        inner_connection_->Close();
        inner_connection_ = nullptr;
        inner_observer_ = nullptr;
        
        closed_ = true;
    }
    
    void RtcPeerConnection::
    set_on_negotiation_needed(const std::function<void()> & value) {
        on_negotiation_needed_ = value;
    }
    
    void RtcPeerConnection::
    set_on_ice_candidate(const std::function<void(const std::shared_ptr<webrtc::IceCandidateInterface> &)> & value) {
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

    
    RtcPeerConnection::ObserverBase::
    ObserverBase(const std::shared_ptr<RtcPeerConnection> & owner):
    owner(owner)
    {}
    
    void RtcPeerConnection::ObserverBase::
    Post(const std::function<void(const std::shared_ptr<RtcPeerConnection> &)> & task) {
        auto owner = this->owner;
        owner->queue_->PostTask([owner, task](){
            if (owner->closed_) { return; }
            task(owner);
        });
    }
    
    RtcPeerConnection::InnerObserver::
    InnerObserver(const std::shared_ptr<RtcPeerConnection> & owner):
    ObserverBase(owner)
    {}
    
    void RtcPeerConnection::InnerObserver::
    OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state)
    {
        Post([new_state](const std::shared_ptr<RtcPeerConnection> & owner) {
            FuncCall(owner->on_signaling_state_change_, new_state);
        });
    }
    void RtcPeerConnection::InnerObserver::
    OnStateChange(StateType state_changed)
    {
        
    }
    void RtcPeerConnection::InnerObserver::
    OnAddStream(webrtc::MediaStreamInterface* inner_stream)
    {
        auto stream = std::make_shared<MediaStream>(*inner_stream);
        Post([stream](const std::shared_ptr<RtcPeerConnection> & owner){
            owner->remote_streams_.push_back(stream);
            FuncCall(owner->on_add_stream_, stream);
        });
    }
    void RtcPeerConnection::InnerObserver::
    OnRemoveStream(webrtc::MediaStreamInterface* arg_inner_stream)
    {
        rtc::scoped_refptr<webrtc::MediaStreamInterface> inner_stream(arg_inner_stream);
        Post([inner_stream](const std::shared_ptr<RtcPeerConnection> & owner) {
            auto stream = owner->FindRemoteStreamByInnerStream(inner_stream.get());
            Remove(owner->remote_streams_, stream);
            FuncCall(owner->on_remove_stream_, stream);
        });
    }
    void RtcPeerConnection::InnerObserver::
    OnDataChannel(webrtc::DataChannelInterface* data_channel)
    {
        
    }
    void RtcPeerConnection::InnerObserver::
    OnRenegotiationNeeded()
    {
        Post([](const std::shared_ptr<RtcPeerConnection> & owner) {
            FuncCall(owner->on_negotiation_needed_);
        });
    }
    void RtcPeerConnection::InnerObserver::
    OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state)
    {
        Post([new_state](const std::shared_ptr<RtcPeerConnection> & owner) {
            FuncCall(owner->on_ice_connection_state_change_, new_state);
        });
    }
    void RtcPeerConnection::InnerObserver::
    OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state)
    {
        Post([new_state](const std::shared_ptr<RtcPeerConnection> & owner) {
            FuncCall(owner->on_ice_gathering_state_change_, new_state);
        });
    }
    void RtcPeerConnection::InnerObserver::
    OnIceCandidate(const webrtc::IceCandidateInterface* arg_candidate)
    {
        std::string sdp;
        bool ok = arg_candidate->ToString(&sdp);
        if (!ok) { Fatal("candidate ToString failed"); }
        
        std::shared_ptr<webrtc::IceCandidateInterface>
        candidate(webrtc::CreateIceCandidate(arg_candidate->sdp_mid(),
                                             arg_candidate->sdp_mline_index(),
                                             sdp,
                                             nullptr));
        
        Post([candidate](const std::shared_ptr<RtcPeerConnection> & owner) {
            FuncCall(owner->on_ice_candidate_, candidate);
        });
    }
    void RtcPeerConnection::InnerObserver::OnIceComplete()
    {
        
    }
    void RtcPeerConnection::InnerObserver::OnIceConnectionReceivingChange(bool receiving)
    {
        
    }
    
    RtcPeerConnection::CreateSessionDescriptionObserver::
    CreateSessionDescriptionObserver(const std::shared_ptr<RtcPeerConnection> & owner,
                                     const std::function<void(const std::shared_ptr<RtcSessionDescription> &)> &  success,
                                     const std::function<void(const std::string &)> & failure):
    ObserverBase(owner),
    success(success),
    failure(failure)
    {}

    void RtcPeerConnection::CreateSessionDescriptionObserver::
    OnSuccess(webrtc::SessionDescriptionInterface* arg_desc) {
        rtc::scoped_refptr<CreateSessionDescriptionObserver> thiz(this);
        std::shared_ptr<RtcSessionDescription> desc = RtcSessionDescription::FromWebrtc(*arg_desc);
        
        Post([thiz, desc](const std::shared_ptr<RtcPeerConnection> & owner) {
            FuncCall(thiz->success, desc);
        });
    }
    void RtcPeerConnection::CreateSessionDescriptionObserver::
    OnFailure(const std::string& error) {
        rtc::scoped_refptr<CreateSessionDescriptionObserver> thiz(this);

        Post([thiz, error](const std::shared_ptr<RtcPeerConnection> & owner) {
            FuncCall(thiz->failure, error);
        });
    }
    
    RtcPeerConnection::SetSessionDescriptionObserver::
    SetSessionDescriptionObserver(const std::shared_ptr<RtcPeerConnection> & owner,
                                  const std::function<void()> & success,
                                  const std::function<void(const std::string &)> & failure):
    ObserverBase(owner),
    success(success),
    failure(failure)
    {}
    
    void RtcPeerConnection::SetSessionDescriptionObserver::
    OnSuccess() {
        rtc::scoped_refptr<SetSessionDescriptionObserver> thiz(this);
        
        Post([thiz](const std::shared_ptr<RtcPeerConnection> & owner) {
            FuncCall(thiz->success);
        });
    }
    void RtcPeerConnection::SetSessionDescriptionObserver::
    OnFailure(const std::string& error) {
        rtc::scoped_refptr<SetSessionDescriptionObserver> thiz(this);
        
        Post([thiz, error](const std::shared_ptr<RtcPeerConnection> & owner) {
            FuncCall(thiz->failure, error);
        });
    }
    
    
    RtcPeerConnection::RtcPeerConnection():
    queue_(TaskQueue::system_current_queue()),
    closed_(false)
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