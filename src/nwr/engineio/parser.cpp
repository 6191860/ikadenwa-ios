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

    Packet MakeParserErrorPacket(const std::string & error) {
        auto data = ToData(error);
        return {
            PacketType::Error,
            std::make_shared<Data>(std::move(data))
        };
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
    
    Websocket::Message EncodePacket(const Packet & packet)
    {
        auto blob = Data();
        char type_char = '0' + static_cast<int>(packet.type);
        blob.insert(blob.end(), static_cast<uint8_t>(type_char));
        blob.insert(blob.end(), packet.data->begin(), packet.data->end());
        return Websocket::Message(blob);
    }
    
    Packet DecodePacket(const Websocket::Message & message)
    {
        auto & data = *message.data;
        
        if (data.size() == 0) {
            return MakeParserErrorPacket("message size is empty");
        }
        
        if (message.mode == Websocket::Message::Mode::Text) {
        
            uint8_t type = static_cast<char>(data[0]) - '0';
            
            if (type == static_cast<uint8_t>('b')){
                return DecodeBase64Packet(Websocket::Message(Data(data.begin() + 1, data.end())));
            }
            
            if (!IsValidPacketType(type)) {
                return MakeParserErrorPacket(Format("invalid packet type: %d", type));
            }
            
            return {
                static_cast<PacketType>(type),
                std::make_shared<Data>(data.begin() + 1, data.end())
            };
            
        } else {
            uint8_t type = data[0];
            
            if (!IsValidPacketType(type)) {
                return MakeParserErrorPacket(Format("invalid packet type: %d", type));
            }
            
            return {
                static_cast<PacketType>(type),
                std::make_shared<Data>(data.begin() + 1, data.end())
            };
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
        
        return {
            static_cast<PacketType>(type),
            std::make_shared<Data>(std::move(data2))
        };
    }

}
}
