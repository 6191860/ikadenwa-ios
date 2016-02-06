//
//  packet.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/06.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <string>
#include <nwr/base/optional.h>
#include <nwr/base/data.h>
#include <nwr/base/json.h>

namespace nwr {
namespace sio {
    enum class PacketType {
        Connect = 0,
        Disconnect = 1,
        Event = 2,
        Ack = 3,
        Error = 4,
        BinaryEvent = 5,
        BinaryAck = 6
    };
    std::string PacketTypeToString(const PacketType & type);
    Optional<PacketType> PacketTypeFromString(const std::string & str);
    bool IsValidPacketTypeValue(int value);
    
    struct Packet {
        PacketType type;
        Optional<std::string> nsp;
        Optional<int> id;
        Optional<Json::Value> data;
        std::vector<DataPtr> attachments;
    };
}
}
