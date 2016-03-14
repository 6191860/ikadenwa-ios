//
//  sio_test.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/11.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "nwr_test_set.h"

namespace app {
    using namespace nwr;
    
    int test_index = 0;
#define ASSERT(x) assert_body(x, #x)
    
    void assert_body(bool x, const char * s) {
        if(x) {
            printf("ok %d - %s\n", test_index, s);
        } else {
            printf("not ok %d - %s\n", test_index, s);
        }
        test_index += 1;
    }
    
    void NwrTestSet::TestAnyType() {
        Any a(std::map<std::string, Any> {
            { "a", Any(3) },
            { "b", Any("bbb") },
            { "c", Any(std::vector<Any> {
                Any(11),
                Any(22),
                Any(33)
            }) }
        });
        
        ASSERT(a.GetAt("a").AsInt() == Some(3));
        ASSERT(a.GetAt("b").AsString() == Some(std::string("bbb")));
        ASSERT(a.GetAt("c").GetAt(0).AsInt() == Some(11));
        ASSERT(a.GetAt("c").GetAt(1).AsInt() == Some(22));
        ASSERT(a.GetAt("c").GetAt(2).AsInt() == Some(33));
        
        //  ref copy
        Any b = a;
        b.SetAt("a", Any("aaa"));
        ASSERT(b.GetAt("a").AsString() == Some(std::string("aaa")));
        ASSERT(a.GetAt("a").AsString() == Some(std::string("aaa")));
        
        //  value copy
        Any c = b.GetAt("b");
        c = Any(3);
        ASSERT(c.AsInt() == Some(3));
        ASSERT(b.GetAt("b").AsString() == Some(std::string("bbb")));
        
        Any d = Any(Any::ObjectType {
            { "aa", Any(Any::ObjectType {
                { "bb", Any(1) }
            }) }
        });
        ASSERT(d.GetAt("aa").GetAt("bb").AsInt() == Some(1));
        d.GetAt("aa").SetAt("bb", Any(2));
        ASSERT(d.GetAt("aa").GetAt("bb").AsInt() == Some(2));
    }
    
    void NwrTestSet::TestEio() {
        eio::Socket::ConstructorParams params;
        //        params.origin = "192.168.1.5";
        
        
        auto socket = eio::Socket::Create("ws://192.168.1.5:8080", params);
        socket->open_emitter()->On([socket](None _){
            printf("[EioTest] on open\n");
            
            socket->Send(eio::PacketData("abcde"));
            
            socket->message_emitter()->On([socket](const eio::PacketData & data){
                printf("[EioTest] on message: %s\n",
                       DataFormat(data.ptr(), data.size()).c_str());
            });
            socket->close_emitter()->On([](None _){
                printf("[EioTest] on close\n");
            });
        });
    }
    void NwrTestSet::TestSio() {
        
        eio::Socket::ConstructorParams params;
        auto socket = sio::Io("ws://192.168.1.5:3000", params);
        socket->emitter()->On("connect", AnyEventListenerMake([socket](){
            printf("socket on connect\n");
            
            Timer::Create(TimeDuration(5.0), [socket]{
                socket->Emit("hello", { Any(3) });
            });
            
        }));
        socket->emitter()->On("echo", AnyEventListenerMake([](const Any & data) {
            printf("socket on echo %s\n", data.ToJsonString().c_str());
        }));
        socket->emitter()->On("disconnect", AnyEventListenerMake([](){
            printf("socket on disconnect\n");
        }));
    }
    
    void NwrTestSet::TestSio0() {
        sio0::SocketOptions options;
        auto socket = sio0::Io::Connect("http://192.168.1.6:3000", options);
        
        socket->emitter()->On("connect", AnyEventListenerMake([socket](const Any & arg){
            printf("socket on connect\n");
            
            Timer::Create(TimeDuration(5.0), [socket](){
                socket->Emit("my other event",
                             {
                                 Any("abc"),
                                 AnyFuncMake([socket](const Any & a)
                                             {
                                                 printf("ack %s\n", a.ToJsonString().c_str());
                                             })
                             });
            });
        }));
        
        socket->emitter()->On("news", AnyEventListenerMake([](const Any & params){
            printf("news %s\n", params.ToJsonString().c_str());
        }));
    }
}
