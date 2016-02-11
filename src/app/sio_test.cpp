//
//  sio_test.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/11.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "sio_test.h"

namespace app {
    using namespace nwr;
    
    void SioTest::Run() {
        
        eio::Socket::ConstructorParams params;
        auto socket = sio::Io("ws://192.168.1.5:3000", params);
        socket->emitter()->On("connect", AnyEventListenerMake([socket](){
            printf("socket on connect\n");
            
            Timer::Create(TimeDuration(5.0), [socket]{
                socket->Emit("hello", { Any(3) });
            });
            
        }));
        socket->emitter()->On("event", AnyEventListenerMake([](const Any & data){
            printf("socket on event %s\n",
                   JsonFormat(*data.ToJson()).c_str());
        }));
        socket->emitter()->On("echo", AnyEventListenerMake([](const Any & data) {
            printf("socket on echo %s\n", JsonFormat(*data.ToJson()).c_str());
        }));
        socket->emitter()->On("disconnect", AnyEventListenerMake([](){
            printf("socket on disconnect\n");
        }));
        

    }
}
