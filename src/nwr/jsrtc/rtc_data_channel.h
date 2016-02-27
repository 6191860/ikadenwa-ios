//
//  rtc_data_channel.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/18.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <memory>
#include <functional>
#include <nwr/base/data.h>
#include <nwr/base/func.h>
#include <nwr/base/optional.h>
#include <nwr/base/task_queue.h>
#include <nwr/base/lib_webrtc.h>
#include <nwr/engineio/packet.h>

#include "post_target.h"

namespace nwr {
namespace jsrtc {
    
    enum class RtcDataChannelState {
        Connecting,
        Open,
        Closing,
        Closed
    };
        
    class RtcDataChannel : public PostTarget<RtcDataChannel> {
    public:
        RtcDataChannel(webrtc::DataChannelInterface & inner_channel);
        virtual ~RtcDataChannel();
        
        webrtc::DataChannelInterface & inner_channel();
        
        std::string label() const;
        bool ordered() const;
        Optional<int> max_packet_life_time() const;
        Optional<int> max_retransmits() const;
        std::string protocol() const;
        bool negotiated() const;
        int id() const;
        RtcDataChannelState ready_state() const;
        int buffered_amount() const;
        
//        attribute unsigned long       bufferedAmountLowThreshold;
        void set_on_open(const std::function<void()> & value);
        //        attribute EventHandler        onbufferedamountlow;
        //        attribute EventHandler        onerror;
        void set_on_close(const std::function<void()> & value);

        
        void set_on_message(const std::function<void(const eio::PacketData &)> & value);

//        attribute DOMString           binaryType;
        void Send(const eio::PacketData & data);
    protected:
        void OnClose() override;
    private:
        struct InnerObserver : public webrtc::DataChannelObserver {
            InnerObserver(RtcDataChannel & owner);
                        
            void OnStateChange() override;
            void OnMessage(const webrtc::DataBuffer & buffer) override;
            void OnBufferedAmountChange(uint64_t previous_amount) override;
            
            RtcDataChannel & owner;
        };
        
        RtcDataChannelState ComputeReadyState(webrtc::DataChannelInterface::DataState state);
        void inner_set_ready_state(RtcDataChannelState value);
        
        bool closed_;
        rtc::scoped_refptr<webrtc::DataChannelInterface> inner_channel_;
        std::shared_ptr<InnerObserver> inner_observer_;
        
        RtcDataChannelState ready_state_;
        std::function<void()> on_open_;
        std::function<void()> on_close_;
        std::function<void(const eio::PacketData &)> on_message_;
       
    };
}
}
