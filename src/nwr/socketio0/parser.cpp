//
//  parser.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/25.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "parser.h"

namespace nwr {
namespace sio0 {
    
    Packet::Packet():
    data(std::make_shared<std::string>()),
    ack_id(0)
    {}
    
    std::string PacketTypeToString(PacketType type) {
        switch (type) {
            case PacketType::Disconnect: return "disconnect";
            case PacketType::Connect: return "connect";
            case PacketType::Heartbeat: return "heartbeat";
            case PacketType::Message: return "message";
            case PacketType::Json: return "json";
            case PacketType::Event: return "event";
            case PacketType::Ack: return "ack";
            case PacketType::Error: return "error";
            case PacketType::Noop: return "noop";
        }
    }
    
    std::string EncodePacket(const Packet & packet) {
        int type = static_cast<int>(packet.type);
        std::string id = packet.id ? Format("%d", *packet.id) : std::string();
        std::string endpoint = packet.endpoint;
        std::string ack = packet.ack || std::string();
        Optional<std::string> data = None();
        
        switch (packet.type) {
            case PacketType::Error: {
                int reason = packet.reason ? IndexOf(Parser::reasons_, packet.reason.value()) : -1;
                int adv = packet.advice ? IndexOf(Parser::advice_, packet.advice.value()) : -1;

                if (reason != -1 || adv != -1) {
                    std::string data_str =
                    Format("%d", reason) +
                    (adv != -1 ? Format("+%d", adv) : "");
                    data = Some(data_str);
                }

                break;
            }
            case PacketType::Message: {
                std::string packet_data = packet.data.AsString().value();
                if (packet_data != "") {
                    data = Some(packet_data);
                }
                break;
            }
            case PacketType::Event: {
                Any ev(Any::ObjectType {
                    { "name", Any(packet.name) }
                });
                
                if (packet.args.type() == Any::Type::Array) {
                    ev.SetAt("args", packet.args);
                }
                
                data = Some(ev.ToJsonString());
                
                break;
            }
            case PacketType::Json: {
                data = Some(packet.data.ToJsonString());
                break;
            }
            case PacketType::Connect: {
                if (packet.qs) {
                    data = Some(packet.qs.value());
                }
                break;
            }
            case PacketType::Ack: {
                std::string data_str = Format("%d", packet.ack_id);
                
                if (packet.args.type() == Any::Type::Array) {
                    data_str += "+" + packet.args.ToJsonString();
                }

                data = Some(data_str);
                break;
            }
            default:
                break;
        }
        
        // construct packet with required fragments
        std::vector<std::string> encoded = {
            Format("%d", type),
            id + (ack == "data" ? "+" : ""),
            endpoint
        };
        
        // data fragment is optional
        if (data) {
            encoded.push_back(data.value());
        }
        
        return Join(encoded, ":");
    }
    
    Packet DecodePacket(const std::string & arg_data) {
        std::smatch pieces;
        std::regex_match(arg_data, pieces, Parser::regexp_);
        if (pieces.empty()) {
            return Packet();
        }
        
        std::string id = pieces[2].str();
        std::string data = pieces[5].str();
        Packet packet;
        packet.type = static_cast<PacketType>(atoi(pieces[1].str().c_str()));
        packet.endpoint = pieces[4].str();
        
        // whether we need to acknowledge the packet
        if (id != "") {
            packet.id = Some(atoi(id.c_str()));
            if (pieces[3].length() != 0) {
                packet.ack = Some(std::string("data"));
            } else {
                packet.ack = Some(std::string());
            }
        }
        
        // handle different packet types
        switch (packet.type) {
            case PacketType::Error: {
                std::vector<std::string> pieces = Split(data, "+");
                packet.reason = Some(Parser::reasons_[atoi(pieces[0].c_str())]);
                packet.advice = Some(Parser::advice_[atoi(pieces[1].c_str())]);
                break;
            }
            case PacketType::Message: {
                packet.data = Any(data);
                break;
            }
            case PacketType::Event: {
                Any opts = Any::FromJsonString(data);
                packet.name = opts.GetAt("name").AsString().value();
                packet.args = opts.GetAt("args");
                
                if (packet.args.type() != Any::Type::Array) {
                    packet.args = Any(Any::ArrayType {});
                }

                break;
            }
            case PacketType::Json: {
                packet.data = Any::FromJsonString(data);

                break;
            }
                
            case PacketType::Connect: {
                packet.qs = Some(data);
                break;
            }
            case PacketType::Ack: {
                std::regex re("([0-9]+)(\\+)?(.*)");
                std::smatch pieces;
                std::regex_match(data, pieces, re);
                if (pieces.size() != 0) {
                    packet.ack_id = atoi(pieces[1].str().c_str());
                    packet.args = Any(Any::ArrayType{});
                    
                    if (pieces[3].length() != 0) {
                        packet.args = Any::FromJsonString(pieces[3].str());
                    }
                }
                break;
            }
                
            case PacketType::Disconnect:
            case PacketType::Heartbeat:
                break;
            case PacketType::Noop:
                break;
        }
        
        
        return packet;
    }

    std::vector<std::string> Parser::reasons_ = {
        "transport not supported",
        "client not handshaken",
        "unauthorized"
    };
    
    std::vector<std::string> Parser::advice_ = {
        "reconnect"
    };
    
    std::regex Parser::regexp_ = std::regex("([^:]+):([0-9]+)?(\\+)?:([^:]+)?:?([\\s\\S]*)?");
 
    
    

    
}
}
