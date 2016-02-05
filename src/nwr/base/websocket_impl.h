//
//  websocket_impl.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/20.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <string>
#include <thread>
#include <memory>
#include <deque>
#include <algorithm>
#include <mutex>

extern "C" {
#include <libwebsockets.h>
}

#include "data.h"
#include "looper.h"

#include "websocket.h"

//  TODO: Agent

namespace nwr {
    class TaskQueue;
    class WebsocketContext;
    class WebsocketThread;
    
    class WebsocketImpl: public std::enable_shared_from_this<WebsocketImpl> {
    public:
        WebsocketImpl(Websocket * owner);
        ~WebsocketImpl();
        
        bool is_closed() { return ready_state_ == Websocket::ReadyState::Closed; }
        
        void Connect(const std::string & url,
                     const std::string & origin,
                     const std::shared_ptr<std::string> & protocol);
        void Close();
        void Send(const Websocket::Message & message);
        
        static int LwsCallbackHandlerStatic(struct lws * wsi,
                                            enum lws_callback_reasons reason,
                                            void * user, void * in, size_t len);
        int LwsCallbackHandler(struct lws * wsi,
                               enum lws_callback_reasons reason,
                               void * user, void * in, size_t len);
        void HandleError(const std::string & message);
        void HandleClosed();
        void HandleConnected();
        void HandleMessage(const Websocket::Message & message);
        
        Websocket * owner_;
        
        Websocket::OnCloseFunc on_close_;
        Websocket::OnErrorFunc on_error_;
        Websocket::OnMessageFunc on_message_;
        Websocket::OnOpenFunc on_open_;
        
        std::string protocol_;
        Websocket::ReadyState ready_state_;
        std::string url_;
        
        std::shared_ptr<TaskQueue> queue_;
        
        WebsocketContext * context_;
        lws * ws_client_;
        
        std::mutex mutex_;
        std::deque<Websocket::Message> sending_queue_;
        
        std::shared_ptr<Websocket::Message> receiving_message_;
    };
    
    class WebsocketContext {
    public:
        WebsocketContext(const std::shared_ptr<std::string> & protocol,
                         lws_callback_function callback);
        ~WebsocketContext();
        
        lws_context * context() { return context_; }
        WebsocketThread * thread() { return thread_; }
        std::shared_ptr<std::string> protocol() { return protocol_; }
        
    private:
        std::shared_ptr<std::string> protocol_;
        std::string inner_protocol_;
        lws_protocols context_protocols_[2];
        
        lws_context * context_;
        
        WebsocketThread * thread_;
    };
    
    class WebsocketThread: public Looper {
    public:
        WebsocketThread(lws_context * context);
        virtual ~WebsocketThread() {}
        
        lws_context * context() { return context_; }
        
        virtual void PostTask(const Task & task);
        
        virtual void Quit();
    private:
        void ThreadMain();
        void PushTask(const Task & task);
        bool PopTask(Task & dest_task);
        lws_context * context_;
        
        std::thread thread_;
        std::mutex mutex_;
        std::deque<Task> tasks_;
        
        bool do_quit_;
    };
}
