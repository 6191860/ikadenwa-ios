//
//  eio_test.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/05.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "eio_test.h"

namespace app {
    using namespace nwr;
    
    void EioTest::Run() {
        eio::Socket::ConstructorParams params;
//        params.origin = "192.168.1.5";
        
        
        auto socket = eio::Socket::Create("ws://192.168.1.5:8080", params);
        socket->open_emitter()->On([socket](None _){
            printf("[EioTest] on open\n");
            
            socket->Send(ToData("abcde"));
            
            socket->message_emitter()->On([socket](const Data & data){
                printf("[EioTest] on message\n");
            });
            socket->close_emitter()->On([](None _){
                printf("[EioTest] on close\n");
            });
        });
    }
}