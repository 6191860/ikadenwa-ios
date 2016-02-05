//
//  socket_manager.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/24.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <string>

#include "time.h"

namespace nwr {
namespace sio {
    
    //  TODO: reconnection
    
    class SocketManager {
    public:
        struct ConstructorParams {
            ConstructorParams();
            
            std::string path;
            bool reconnection;
            int reconnection_attempts;
            TimeDuration reconnection_delay;
            TimeDuration reconnection_delay_max;
            double randomization_factor;
            TimeDuration timeout;
            bool auto_connect;
        };
        enum class ReadyState {
            Closed
            
        };
        SocketManager(const std::string & uri, const ConstructorParams & params);
    private:
        bool reconnection();
        void set_reconnection(bool value);
        
        int reconnection_attempts();
        void set_reconnection_attempts(int value);
        
        TimeDuration reconnection_delay();
        void set_reconnection_delay(const TimeDuration & value);
        
        TimeDuration reconnection_delay_max();
        void set_reconnection_delay_max(const TimeDuration & value);
        
        double randomization_factor();
        void set_randomization_factor(double value);
        
        TimeDuration timeout();
        void set_timeout(const TimeDuration & value);
        
        void EmitAll();
        
        void Open();
        
        int nsps_;
        int subs_;
        ConstructorParams params_;
        bool reconnection_;
        int reconnection_attempts_;
        TimeDuration reconnection_delay_;
        TimeDuration reconnection_delay_max_;
        double randomization_factor_;
        int backoff_; //TODO
        TimeDuration timeout_;
        ReadyState ready_state_;
        std::string uri_;
        int connecting_;
        int last_ping_;
        bool encoding_;
        int packet_buffer_;
        int encoder_;
        int decoder_;
        bool auto_connect_;
    };
}
}