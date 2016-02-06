//
//  websocket.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/16.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "websocket.h"

#include "string.h"
#include "url.h"
#include "task_queue.h"
#include "websocket_impl.h"

namespace nwr {
    
    Websocket::Message::Message(const std::string & text):
    Message(Mode::Text,
            std::make_shared<Data>(ToData(text))){}

    Websocket::Message::Message(const Data & binary):
    Message(Mode::Binary,
            std::make_shared<Data>(binary)){}
    
    Websocket::Message::Message():
    Message(Mode::Binary){}
    
    Websocket::Message::Message(Mode mode):
    Message(mode, std::make_shared<Data>()){}
    
    Websocket::Message::Message(Mode mode, const DataPtr & data):
    mode(mode), data(data){}
    
    std::shared_ptr<Websocket> Websocket::Create(const std::string & url,
                                                 const std::string & origin)
    {
        return Create(url, origin, nullptr);
    }
    std::shared_ptr<Websocket> Websocket::Create(const std::string & url,
                                                 const std::string & origin,
                                                 const std::shared_ptr<std::string> & protocol)
    {
        auto thiz = std::shared_ptr<Websocket>(new Websocket());
        thiz->impl_->Connect(url, origin, protocol);
        return thiz;
    }

    Websocket::~Websocket() {
    }
    
    Websocket::OnCloseFunc Websocket::on_close() {
        return impl_->on_close_;
    }
    void Websocket::set_on_close(const OnCloseFunc & value) {
        impl_->on_close_ = value;
    }
    Websocket::OnErrorFunc Websocket::on_error() {
        return impl_->on_error_;
    }
    void Websocket::set_on_error(const OnErrorFunc & value) {
        impl_->on_error_ = value;
    }
    Websocket::OnMessageFunc Websocket::on_message() {
        return impl_->on_message_;
    }
    void Websocket::set_on_message(const OnMessageFunc & value) {
        impl_->on_message_ = value;
    }
    Websocket::OnOpenFunc Websocket::on_open() {
        return impl_->on_open_;
    }
    void Websocket::set_on_open(const OnOpenFunc & value) {
        impl_->on_open_ = value;
    }
    std::string Websocket::protocol() {
        return impl_->protocol_;
    }
    Websocket::ReadyState Websocket::ready_state() {
        return impl_->ready_state_;
    }
    std::string Websocket::url() {
        return impl_->url_;
    }

    void Websocket::Close() {
        impl_->Close();
    }
    
    void Websocket::Send(const std::string & message) {
        Send(Message(message));
    }
    void Websocket::Send(const Data & message) {
        Send(Message(message));
    }
    void Websocket::Send(const Message & message) {
        impl_->Send(message);
    }
    Websocket::Websocket() {
        impl_ = std::make_shared<WebsocketImpl>(this);
    }
}
