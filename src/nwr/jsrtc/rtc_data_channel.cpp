//
//  rtc_data_channel.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/18.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "rtc_data_channel.h"

namespace nwr {
namespace jsrtc {
    RtcDataChannel::RtcDataChannel(webrtc::DataChannelInterface & inner_channel):
    closed_(false),
    inner_channel_(&inner_channel)
    {
        inner_observer_ = std::make_shared<InnerObserver>(*this);
        inner_channel_->RegisterObserver(inner_observer_.get());
        
        ready_state_ = ComputeReadyState(inner_channel_->state());
    }
    
    webrtc::DataChannelInterface & RtcDataChannel::inner_channel() {
        return *inner_channel_;
    }
    
    std::string RtcDataChannel::label() const {
        return inner_channel_->label();
    }
    bool RtcDataChannel::ordered() const {
        return inner_channel_->ordered();
    }
    Optional<int> RtcDataChannel::max_packet_life_time() const {
        int value = inner_channel_->maxRetransmitTime();
        if (value == -1) { return None(); }
        return Some(value);
    }
    Optional<int> RtcDataChannel::max_retransmits() const {
        int value = inner_channel_->maxRetransmits();
        if (value == -1) { return None(); }
        return Some(value);
    }
    std::string RtcDataChannel::protocol() const {
        return inner_channel_->protocol();
    }
    bool RtcDataChannel::negotiated() const {
        return inner_channel_->negotiated();
    }
    int RtcDataChannel::id() const {
        return inner_channel_->id();
    }
    RtcDataChannelState RtcDataChannel::ready_state() const {
        return ready_state_;
    }
    int RtcDataChannel::buffered_amount() const {
        return static_cast<int>(inner_channel_->buffered_amount());
    }
    
    void RtcDataChannel::set_on_open(const std::function<void()> & value) {
        on_open_ = value;
    }
    void RtcDataChannel::set_on_close(const std::function<void()> & value) {
        on_close_ = value;
    }
    
    void RtcDataChannel::Close() {
        if (closed_) { return; }
        
        inner_set_ready_state(RtcDataChannelState::Closed);
        
        inner_channel_->Close();
        inner_channel_ = nullptr;
        inner_observer_ = nullptr;
        
        ClosePostTarget();
        closed_ = true;
    }
    
    void RtcDataChannel::set_on_message(const std::function<void(const eio::PacketData &)> & value) {
        on_message_ = value;
    }
    
    void RtcDataChannel::Send(const eio::PacketData & data) {
        webrtc::DataBuffer wdata(rtc::Buffer(data.ptr(), data.size()), data.binary != nullptr);
        inner_channel_->Send(wdata);
    }
    
    RtcDataChannel::InnerObserver::InnerObserver(RtcDataChannel & owner):
    owner(owner)
    {}
    
    void RtcDataChannel::InnerObserver::OnStateChange() {
        auto new_state = owner.ComputeReadyState(owner.inner_channel_->state());
        owner.Post([new_state](RtcDataChannel & owner){
            owner.inner_set_ready_state(new_state);
        });
    }
    void RtcDataChannel::InnerObserver::OnMessage(const webrtc::DataBuffer & buffer) {
        eio::PacketData data(buffer.data.data(),
                             static_cast<int>(buffer.data.size()), buffer.binary);
        
        owner.Post([data](RtcDataChannel & owner){
            FuncCall(owner.on_message_, data);
        });
    }
    void RtcDataChannel::InnerObserver::OnBufferedAmountChange(uint64_t previous_amount) {
        
    }
    
    RtcDataChannelState RtcDataChannel::ComputeReadyState(webrtc::DataChannelInterface::DataState state) {
        switch (state) {
            case webrtc::DataChannelInterface::kConnecting:
                return RtcDataChannelState::Connecting;
            case webrtc::DataChannelInterface::kOpen:
                return RtcDataChannelState::Open;
            case webrtc::DataChannelInterface::kClosing:
                return RtcDataChannelState::Closing;
            case webrtc::DataChannelInterface::kClosed:
                return RtcDataChannelState::Closed;
        }
    }
    
    void RtcDataChannel::inner_set_ready_state(RtcDataChannelState value) {
        if (ready_state_ != value) {
            ready_state_ = value;
            if (value == RtcDataChannelState::Open) {
                FuncCall(on_open_);
            } else if (value == RtcDataChannelState::Closed) {
                FuncCall(on_close_);
            }
        }
    }
}
}