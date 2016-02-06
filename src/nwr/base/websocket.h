//
//  websocket.h
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/16.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <string>
#include <memory>
#include <functional>

#include "data.h"

namespace nwr {    
    class WebsocketImpl;
    
    class Websocket {
    public:
        enum class ReadyState {
            Connecting = 0,
            Open = 1,
            Closing = 2,
            Closed = 3
        };
        struct Message {
            enum class Mode {
                Text = 1,
                Binary = 2
            };
            Mode mode;
            DataPtr data;
            Message(const std::string & text);
            Message(const Data & binary);
            Message();
            Message(Mode mode);
            Message(Mode mode, const DataPtr & data);
        };
        
        using OnCloseFunc = std::function<void()>;
        using OnErrorFunc = std::function<void(const std::string &)>;
        using OnMessageFunc = std::function<void(const Message &)>;
        using OnOpenFunc = std::function<void()>;
        
        static std::shared_ptr<Websocket> Create(const std::string & url,
                                                 const std::string & origin);
        static std::shared_ptr<Websocket> Create(const std::string & url,
                                                 const std::string & origin,
                                                 const std::shared_ptr<std::string> & protocol);
        ~Websocket();
        
        OnCloseFunc on_close();
        void set_on_close(const OnCloseFunc & value);
        OnErrorFunc on_error();
        void set_on_error(const OnErrorFunc & value);
        OnMessageFunc on_message();
        void set_on_message(const OnMessageFunc & value);
        OnOpenFunc on_open();
        void set_on_open(const OnOpenFunc & value);
        std::string protocol();
        ReadyState ready_state();
        std::string url();
        
        void Close();
        void Send(const std::string & message);
        void Send(const Data & message);
        void Send(const Message & message);
    private:
        Websocket();
        
        std::shared_ptr<WebsocketImpl> impl_;
    };
}
