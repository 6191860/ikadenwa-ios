//
//  packet.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/06.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "packet.h"

namespace nwr {
namespace sio {
 
    std::string PacketTypeToString(const PacketType & type) {
        const char c = static_cast<char>(type) + '0';
        return std::string(&c, 1);
    }
    Optional<PacketType> PacketTypeFromString(const std::string & str) {
        if (str.length() <= 0) {
            return Optional<PacketType>();
        }
        int value = str[0] - '0';
        if (!IsValidPacketTypeValue(value)) {
            return Optional<PacketType>();
        }
        return OptionalSome(static_cast<PacketType>(value));
    }
    
    bool IsValidPacketTypeValue(int value) {
        return (static_cast<int>(PacketType::Connect) <= value &&
                value <= static_cast<int>(PacketType::BinaryAck));
    }
    
    Packet::Packet()
    {}
}
}