//
//  parser.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/19.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

//  https://github.com/socketio/engine.io-parser

#pragma once

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <algorithm>

#include <nwr/base/env.h>
#include <nwr/base/data.h>
#include <nwr/base/string.h>
#include <nwr/base/base64.h>
#include <nwr/base/websocket.h>

namespace nwr {
namespace eio {
    
    int parser_protocol();
    
    enum class PacketType {
        Open = 0,   // non-ws
        Close = 1,  // non-ws,
        Ping = 2,
        Pong = 3,
        Message = 4,
        Upgrade = 5,
        Noop = 6,
        Error = 7
    };
        
    struct Packet {
        PacketType type;
        DataPtr data;
    };
    
    Packet MakeParserErrorPacket(const std::string & error);
    
    std::string ToString(const PacketType & type);
    bool IsValidPacketType(uint8_t type);

    Websocket::Message EncodePacket(const Packet & packet);
    Packet DecodePacket(const Websocket::Message & message);
    Packet DecodeBase64Packet(const Websocket::Message & message);
    
}
}


