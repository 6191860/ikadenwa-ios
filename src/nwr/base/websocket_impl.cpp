//
//  websocket_impl.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/20.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "websocket_impl.h"

#include "env.h"
#include "string.h"
#include "url.h"
#include "task_queue.h"

namespace nwr {
    WebsocketImpl::WebsocketImpl(Websocket * owner) {
        owner_ = owner;
        queue_ = TaskQueue::current_queue();
        ready_state_ = Websocket::ReadyState::Connecting;
        context_ = nullptr;
    }
    WebsocketImpl::~WebsocketImpl() {
        if (!is_closed()) {
            Fatal("not closed before destructor");
        }
    }
    
    void WebsocketImpl::Connect(const std::string & url,
                                const std::string & origin,
                                const std::shared_ptr<std::string> & protocol)
    {
        auto url_parts = ParseUrl(url);
        auto scheme = url_parts.scheme;
        if (scheme == "") {
            scheme = "ws";
        }
        bool is_ssl = false;
        if (scheme == "wss" || scheme == "https") {
            is_ssl = true;
        }
        if (!url_parts.port) {
            if (is_ssl) {
                url_parts.port = Some(443);
            } else {
                url_parts.port = Some(80);
            }
        }
        url_ = url;
        
        std::string path = url_parts.path;
        if (url_parts.query.size() > 0) {
            path += "?" + QueryStringEncode(url_parts.query);
        }
        
        context_ = new WebsocketContext(protocol, &WebsocketImpl::LwsCallbackHandlerStatic);
        
        context_->thread()->PostTask([this, url_parts, is_ssl, origin, path]{
            lws_client_connect_info info = { 0 };
            info.context = context_->context();
            info.address = url_parts.hostname.c_str();
            info.port = url_parts.port || -1;
            info.ssl_connection = is_ssl ? 2 : 0;
            info.path = path.c_str();
            info.host = url_parts.hostname.c_str();
            info.origin = origin.c_str();
            if (context_->protocol()) {
                info.protocol = context_->protocol()->c_str();
            } else {
                info.protocol = nullptr;
            }
            info.ietf_version_or_minus_one = -1;
            info.userdata = this;
            
            ws_client_ = lws_client_connect_via_info(&info);
        });
    }
    
    void WebsocketImpl::Close() {
        if (is_closed()) { return; }
        
        if (context_) {
            delete context_;
            context_ = nullptr;
        }
        
        ready_state_ = Websocket::ReadyState::Closed;
        FuncCall(on_close_);
    }
    
    void WebsocketImpl::Send(const Websocket::Message & message) {
        if (ready_state_ != Websocket::ReadyState::Open) {
            Fatal(Format("ready_state(%d) != Open", ready_state_));
        }
        
        {
            std::lock_guard<std::mutex> lk(mutex_);
            sending_queue_.push_back(message);
            context_->thread()->PostTask([this] {
                lws_callback_on_writable(this->ws_client_);
            });
        }
    }
    
    int WebsocketImpl::LwsCallbackHandlerStatic(struct lws * wsi,
                                                enum lws_callback_reasons reason,
                                                void * user, void * in, size_t len)
    {
        if (!user) {
            switch (reason) {
                default:
                    //                    printf("lws callback, user data is null; reason = %d\n", reason);
                    break;
            }
            return 0;
        }
        auto * thiz = static_cast<WebsocketImpl *>(user);
        return thiz->LwsCallbackHandler(wsi, reason, user, in, len);
    }
    
    int WebsocketImpl::LwsCallbackHandler(struct lws * wsi,
                                          enum lws_callback_reasons reason,
                                          void * user, void * in, size_t len)
    {
//        printf("[WebsocketImpl::LwsCallbackHandler] reason=%d\n", reason);
        switch (reason) {
            case LWS_CALLBACK_CLIENT_ESTABLISHED: {
                protocol_ = lws_get_protocol(wsi)->name;
                
                {
                    auto thiz = shared_from_this();
                    queue_->PostTask([thiz]{
                        thiz->HandleConnected();
                    });
                }
                break;
            }
            case LWS_CALLBACK_CLIENT_CONNECTION_ERROR: {
                std::string message("connection error");
                if (in) {
                    message = Format("%s: %.*s", message.c_str(), len, (char *)in);
                }
                
                {
                    auto thiz = shared_from_this();
                    queue_->PostTask([thiz, message]{
                        thiz->HandleError(message);
                    });
                }
                
                break;
            }
            case LWS_CALLBACK_CLOSED: {
                {
                    auto thiz = shared_from_this();
                    queue_->PostTask([thiz] {
                        thiz->HandleClosed();
                    });
                }
                
                break;
            }
            case LWS_CALLBACK_CLIENT_RECEIVE: {
                uint8_t * data = static_cast<uint8_t *>(in);
                const int read_len = static_cast<int>(len);
                const int rest_len = static_cast<int>(lws_remaining_packet_payload(ws_client_));
                
                if (!receiving_message_) {
                    Websocket::Message::Mode mode;
                    if (lws_frame_is_binary(ws_client_)) {
                        mode = Websocket::Message::Mode::Binary;
                    } else {
                        mode = Websocket::Message::Mode::Text;
                    }
                    
                    receiving_message_ = std::make_shared<Websocket::Message>(mode, std::make_shared<Data>());
                }
                
                receiving_message_->data->insert(receiving_message_->data->end(),
                                                 data, data + read_len);
                
                if (rest_len > 0) {
                    break;
                }
                
                auto message = receiving_message_;
                receiving_message_ = nullptr;
                
                {
                    auto thiz = shared_from_this();
                    queue_->PostTask([thiz, message]{
                        thiz->HandleMessage(*message);
                    });
                }
                
                break;
            }
            case LWS_CALLBACK_CLIENT_WRITEABLE: {
                Websocket::Message message;
                
                {
                    std::lock_guard<std::mutex> lk(mutex_);
                    if (sending_queue_.size() == 0) {
                        break;
                    }
                    message = sending_queue_.front();
                    sending_queue_.pop_front();
                }
                
                const int data_len = static_cast<int>(message.data->size());
                const int buf_len = LWS_SEND_BUFFER_PRE_PADDING + data_len + LWS_SEND_BUFFER_POST_PADDING;
                Data buf(buf_len);
                std::copy(message.data->begin(), message.data->begin() + data_len,
                          buf.begin() + LWS_SEND_BUFFER_PRE_PADDING);
                
                lws_write_protocol write_protocol;
                switch (message.mode) {
                    case Websocket::Message::Mode::Text:
                        write_protocol = LWS_WRITE_TEXT;
                        break;
                    case Websocket::Message::Mode::Binary:
                        write_protocol = LWS_WRITE_BINARY;
                        break;
                    default:
                        break;
                }
                
                const int wrote_len = lws_write(ws_client_,
                                                &buf[0] + LWS_SEND_BUFFER_PRE_PADDING,
                                                data_len,
                                                write_protocol);
                if (wrote_len == -1) {
                    auto thiz = shared_from_this();
                    queue_->PostTask([thiz]{
                        thiz->HandleError(Format("lws_write failed"));
                    });
                    break;
                }
                
                if (wrote_len < data_len) {
                    Fatal(Format("lws_write failed: buf=%d, wrote=%d", buf_len, wrote_len));
                }
                lws_callback_on_writable(ws_client_);
                
                break;
            }
            default:
                break;
        }
        return 0;
    }
    
    void WebsocketImpl::HandleError(const std::string & message) {
        if (is_closed()) { return; }
        
        FuncCall(on_error_, message);
        
        Close();
    }
    
    void WebsocketImpl::HandleClosed() {
        Close();
    }
    
    void WebsocketImpl::HandleConnected() {
        if (is_closed()) { return; }
        
        ready_state_ = Websocket::ReadyState::Open;
        FuncCall(on_open_);
    }
    
    void WebsocketImpl::HandleMessage(const Websocket::Message & message) {
        if (is_closed()) { return; }
        
        FuncCall(on_message_, message);
    }
    
    
    
    WebsocketContext::WebsocketContext(const std::shared_ptr<std::string> & protocol,
                                       lws_callback_function callback)
    {
        protocol_ = protocol;
        
        if (protocol) {
            inner_protocol_ = *protocol;
        } else {
            inner_protocol_ = "default";
        }
        
        memset(context_protocols_, 0, sizeof(context_protocols_));
        
        context_protocols_[0] = {
            inner_protocol_.c_str(),
            callback,
            0,
            16 * 1024,
            0,
            nullptr
        };
        
        lws_context_creation_info info = { 0 };
        info.port = CONTEXT_PORT_NO_LISTEN;
        info.protocols = context_protocols_;
        info.gid = -1;
        info.uid = -1;
        
        context_ = lws_create_context(&info);
        if (!context_) {
            Fatal("lws_create_context failed");
        }
        
        thread_ = new WebsocketThread(context_);
    }
    
    WebsocketContext::~WebsocketContext() {
        thread_->Quit();
        delete thread_;
        
        lws_context_destroy(context_);
    }
    
    
    
    WebsocketThread::WebsocketThread(lws_context * context):
    context_(context){
        thread_ = std::thread(std::bind(&WebsocketThread::ThreadMain, this));
        do_quit_ = false;
    }
    void WebsocketThread::PostTask(const Task & task) {
        PushTask(task);
        lws_cancel_service(context_);
    }
    void WebsocketThread::Quit() {
        PushTask([this] {
            this->do_quit_ = true;
        });
        thread_.join();
    }
    void WebsocketThread::ThreadMain() {
        while (!do_quit_) {
            lws_service(context_, 1000);
            
            while (true) {
                Task task;
                if (!PopTask(task)) {
                    break;
                }
                task();
            }
        }
    }
    void WebsocketThread::PushTask(const Task & task) {
        std::lock_guard<std::mutex> lk(mutex_);
        tasks_.push_back(task);
    }
    bool WebsocketThread::PopTask(Task & dest_task) {
        std::lock_guard<std::mutex> lk(mutex_);
        if (tasks_.size() == 0) {
            return false;
        }
        dest_task = tasks_.front();
        tasks_.pop_front();
        return true;
    }
}