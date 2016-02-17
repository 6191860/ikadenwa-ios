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
