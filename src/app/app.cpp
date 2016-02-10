//
//  App.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/16.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "app.h"



namespace app {

    void App::Test1() {
   
//        auto params = nwr::eio::Socket::ConstructorParams();
//        params.origin = "https://ikadenwa.ink";
//        eio_sock_ = std::shared_ptr<nwr::eio::Socket>(new nwr::eio::Socket("", params));
        
//        ws_ = nwr::Websocket::Create("ws://192.168.1.5:7681",
//                                     "192.168.11.96",
//                                     std::make_shared<std::string>("lws-mirror-protocol"));
//        ws_->set_on_open([]{
//            printf("on_open\n");
//        });
//        ws_->set_on_close([]{
//            printf("close\n");
//        });
//        ws_->set_on_error([](const std::string & message) {
//            printf("error %s\n", message.c_str());
//        });
//        ws_->set_on_message([](const nwr::Websocket::Message & message) {
//            printf("data %d[%.*s]\n", (int)message.data->size(), (int)message.data->size(), &message.data->at(0));
//        });
//
//        timer_ = nwr::Timer::Create(std::chrono::seconds(3), std::chrono::seconds(1), [this] {
//            if (ws_->ready_state() == nwr::Websocket::ReadyState::Open) {
//                ws_->Send(nwr::Format("hello world %d", timer_->repeat_count()));
//            }
//            
//            
//            if (timer_->repeat_count() >= 10) {
//                timer_->Cancel();
//                timer_ = nullptr;
//            }
//        });
        
//        eio_test_ = std::make_shared<EioTest>();
//        eio_test_->Run();
        
        any_test_ = std::make_shared<AnyTest>();
        any_test_->Run();
    }
}