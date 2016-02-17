//
//  packet.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/18.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "packet.h"

namespace nwr {
namespace eio {
    char PacketTypeToChar(const PacketType & type) {
        return '0' + static_cast<int>(type);
    }
    std::string ToString(const PacketType & type) {
        switch (type) {
            case PacketType::Open:
                return "Open";
            case PacketType::Close:
                return "Close";
            case PacketType::Ping:
                return "Ping";
            case PacketType::Pong:
                return "Pong";
            case PacketType::Message:
                return "Message";
            case PacketType::Upgrade:
                return "Upgrade";
            case PacketType::Noop:
                return "Noop";
            case PacketType::Error:
                return "Error";
            default:
                Fatal(Format("invalid packet type: %d", type));
        }
    }
    bool IsValidPacketType(uint8_t type) {
        return 0 <= type && type <= static_cast<int>(PacketType::Noop);
    }
    
    PacketData::PacketData(){}
    
    PacketData::PacketData(const Data & data):
    PacketData(std::make_shared<Data>(data)){}
    
    PacketData::PacketData(const DataPtr & data): binary(data){}
    
    PacketData::PacketData(const std::string & data):
    PacketData(std::make_shared<std::string>(data)){}
    
    PacketData::PacketData(const std::shared_ptr<std::string> & data): text(data){}
    
    PacketData::PacketData(const uint8_t * ptr, int size, bool binary) {
        if (binary) {
            this->binary = std::make_shared<Data>(ptr, ptr + size);
        } else {
            this->text = std::make_shared<std::string>(reinterpret_cast<const char *>(ptr), size);
        }
    }
    
    const uint8_t * PacketData::ptr() const {
        if (binary) {
            return &(*binary)[0];
        } else {
            return reinterpret_cast<const uint8_t *>(&(*text)[0]);
        }
    }
    const char * PacketData::char_ptr() const {
        if (binary) {
            return reinterpret_cast<const char *>(&(*binary)[0]);
        } else {
            return &(*text)[0];
        }
    }
    int PacketData::size() const {
        if (binary) {
            return static_cast<int>(binary->size());
        } else {
            return static_cast<int>(text->size());
        }
    }

}
}