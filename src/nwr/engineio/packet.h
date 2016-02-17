//
//  packet.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/18.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <string>
#include <memory>

#include <nwr/base/env.h>
#include <nwr/base/data.h>
#include <nwr/base/string.h>

namespace nwr {
namespace eio {
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
    char PacketTypeToChar(const PacketType & type);
    std::string ToString(const PacketType & type);
    bool IsValidPacketType(uint8_t type);
    
    struct PacketData {
        PacketData();
        explicit PacketData(const Data & data);
        explicit PacketData(const DataPtr & data);
        explicit PacketData(const std::string & data);
        explicit PacketData(const std::shared_ptr<std::string> & data);
        PacketData(const uint8_t * ptr, int size, bool binary);
        
        DataPtr binary;
        std::shared_ptr<std::string> text;
        
        const uint8_t * ptr() const;
        const char * char_ptr() const;
        int size() const;
    };
    
    struct Packet {
        PacketType type;
        PacketData data;
    };
    
    
}
}
