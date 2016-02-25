//
//  parser.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/25.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <vector>
#include <string>
#include <memory>
#include <regex>
#include <nwr/base/optional.h>
#include <nwr/base/array.h>
#include <nwr/base/string.h>
#include <nwr/base/any.h>

namespace nwr {
namespace sio0 {
    enum class PacketType {
        Disconnect = 0,
        Connect = 1,
        Heartbeat = 2,
        Message = 3,
        Json = 4,
        Event = 5,
        Ack = 6,
        Error = 7,
        Noop = 8
    };
    std::string PacketTypeToString(PacketType type);
    
    struct Packet {
        Packet();
        
        PacketType type;
        Optional<int> id;
        std::string endpoint;
        Optional<std::string> ack;
        Optional<std::string> reason;
        Optional<std::string> advice;
        Any data;
        std::string name;
        Any args;
        Optional<std::string> qs;
        int ack_id;
    };
    
    std::string EncodePacket(const Packet & packet);
    Packet DecodePacket(const std::string & data);
    
    class Parser {
    public:
        static std::vector<std::string> reasons_;
        static std::vector<std::string> advice_;
        static std::regex regexp_;
    };
    

    

}
}
