//
//  parser.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/19.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "parser.h"

namespace nwr {
namespace eio {
    
    int parser_protocol() { return 3; }
    
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
    
    const uint8_t * PacketData::ptr() const {
        if (binary) {
            return &binary->at(0);
        } else {
            return reinterpret_cast<const uint8_t *>(&text->at(0));
        }
    }
    const char * PacketData::char_ptr() const {
        if (binary) {
            return reinterpret_cast<const char *>(&binary->at(0));
        } else {
            return &text->at(0);
        }
    }
    int PacketData::size() const {
        if (binary) {
            return static_cast<int>(binary->size());
        } else {
            return static_cast<int>(text->size());
        }
    }

    Packet MakeParserErrorPacket(const std::string & error) {
        return { PacketType::Error, PacketData(std::make_shared<std::string>(error)) };
    }
    
    Websocket::Message EncodePacket(const Packet & packet)
    {
        if (packet.data.binary) {
            return EncodeBuffer(packet);
        }
        
        std::string encoded;
        encoded += PacketTypeToChar(packet.type);
        encoded += *packet.data.text;
        return Websocket::Message(encoded);
    }
    
    Websocket::Message EncodeBuffer(const Packet & packet) {
        auto data = Data();
        data.insert(data.end(), static_cast<uint8_t>(packet.type));
        data.insert(data.end(), packet.data.binary->begin(), packet.data.binary->end());
        return Websocket::Message(data);
    }
    
    Packet DecodePacket(const Websocket::Message & message)
    {
        auto & data = *message.data;
        
        if (data.size() == 0) {
            return MakeParserErrorPacket("message size is empty");
        }
        
        if (message.mode == Websocket::Message::Mode::Text) {
            char type_char = data[0];
            
            if (type_char == 'b'){
                return DecodeBase64Packet(Websocket::Message(Data(data.begin() + 1, data.end())));
            }
            
            uint8_t type = type_char - '0';
            
            if (!IsValidPacketType(type)) {
                return MakeParserErrorPacket(Format("invalid packet type: %d", type));
            }
            
            auto packet_data = std::make_shared<std::string>(reinterpret_cast<const char *>(&data[1]),
                                                             data.size() - 1);
            return { static_cast<PacketType>(type), PacketData(packet_data) };
        } else {
            uint8_t type = data[0];
            
            if (!IsValidPacketType(type)) {
                return MakeParserErrorPacket(Format("invalid packet type: %d", type));
            }
            
            return { static_cast<PacketType>(type),
                PacketData(std::make_shared<Data>(data.begin() + 1, data.end())) };
        }
    }
    
    Packet DecodeBase64Packet(const Websocket::Message & message) {
        auto & data = *message.data;
        if (data.size() == 0) {
            return MakeParserErrorPacket("message size is empty");
        }
        uint8_t type = static_cast<char>(data[0]) - '0';
        
        if (!IsValidPacketType(type)) {
            return MakeParserErrorPacket(Format("invalid packet type: %d", type));
        }
        
        Data data2;
        Base64Decode(Data(data.begin() + 1, data.end()), data2);
        
        return { static_cast<PacketType>(type),
            PacketData(std::make_shared<Data>(std::move(data2))) };
    }

}
}
