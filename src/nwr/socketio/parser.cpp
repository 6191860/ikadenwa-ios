//
//  parser.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/06.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "parser.h"

namespace nwr {
namespace sio {
    int parser_protocol() {
        return 4;
    }
    
    std::vector<DataPtr> Encoder::Encode(const Packet & packet) {
        printf("%s\n", __PRETTY_FUNCTION__);
        
        if (packet.type == PacketType::BinaryEvent || packet.type == PacketType::BinaryAck) {
            return EncodeAsBinary(packet);
        } else {
            auto encoding = EncodeAsString(packet);
            std::vector<DataPtr> ret = { encoding };
            return ret;
        }
    }
    
    DataPtr EncodeAsString(const Packet & packet) {
        std::string str;
        bool nsp = false;
        
        // first is type
        str += PacketTypeToString(packet.type);
        
        // attachments if we have them
        if (packet.type == PacketType::BinaryEvent || packet.type == PacketType::BinaryAck) {
            str += Format("%d", static_cast<int>(packet.attachments.size()));
            str += "-";
        }
        
        // if we have a namespace other than `/`
        // we append it followed by a comma `,`
        if (packet.nsp && *packet.nsp != "/") {
            nsp = true;
            str += *packet.nsp;
        }
        
        // immediately followed by the id
        if (packet.id) {
            if (nsp) {
                str += ",";
                nsp = false;
            }
            str += Format("%d", *packet.id);
        }
        
        // json data
        if (packet.data) {
            if (nsp) { str += ","; }
            str += JsonFormat(*packet.data);
        }
        
//        debug('encoded %j as %s', obj, str);
        
        return std::make_shared<Data>(ToData(str));
    }
    
    std::vector<DataPtr> EncodeAsBinary(const Packet & packet) {
        auto pack = EncodeAsString(packet);
        std::vector<DataPtr> buffers = packet.attachments;
        buffers.insert(buffers.begin(), pack);
        return buffers;
    }
    
    Decoder::Decoder():
    decoded_emitter_(std::make_shared<Emitter<Packet>>())
    {}
    
    Decoder::~Decoder(){}
    
    void Decoder::Add(const eio::PacketData & data) {
        if (data.text) {
        
            Packet packet = DecodeString(*data.text);
            if (packet.type == PacketType::BinaryEvent || packet.type == PacketType::BinaryAck) { // binary packet's json
                reconstructor_ = std::make_shared<BinaryReconstructor>(packet);
                
                // no attachments, labeled binary but no binary data to follow
                if (reconstructor_->recon_pack().attachments.size() == 0) {
                    
                    decoded_emitter_->Emit(packet);
                }
            } else { // non-binary full packet
                decoded_emitter_->Emit(packet);
            }
                
        } else {
            if (!reconstructor_) {
                Fatal("got binary data when not reconstructing a packet");
            } else {
                Optional<Packet> packet = reconstructor_->TakeBinaryData(*data.binary);
                if (packet) { // received final buffer
                    reconstructor_ = nullptr;
                    decoded_emitter_->Emit(*packet);
                }
            }
        }
    }
    
    Packet DecodeString(const std::string & str) {        
        Packet p;
        int i = 0;
        
        // look up type
        if (str.length() <= 0) { return ParserError(); }
        
        auto packetType = PacketTypeFromString(str.substr(0, 1));
        if (!packetType) { return ParserError(); }
        
        p.type = *packetType;
        
        // look up attachments if type binary
        if (p.type == PacketType::BinaryEvent || p.type == PacketType::BinaryAck) {
            std::string buf;
            while (true) {
                i += 1;
                if (i >= str.length()) { break; }
                char c = str[i];
                if (c == '-') { break; }
                buf += c;
            }
            if (!IsDigit(buf) ||
                !(i < str.length() && str[i] == '-')) {
                Fatal("Illegal attachments");
            }
            p.attachments.resize(atoi(buf.c_str()));
        }
        
        // look up namespace (if any)
        if (i + 1 < str.length() && str[i+1] == '/') {
            p.nsp = Some(std::string());
            while (true) {
                i += 1;
                if (i >= str.length()) { break; }
                char c = str[i];
                if (c == ',') { break; }
                p.nsp = Some(*p.nsp + c);
            }
        } else {
            p.nsp = Some(std::string("/"));
        }
        
        // look up id
        if (i + 1 < str.length()) {
            if (isdigit(str[i + 1])) {
                i += 1;
                std::string buf;
                while (true) {
                    if (i >= str.length()) {
                        i -= 1;
                        break; }
                    char c = str[i];
                    if (!isdigit(c)) {
                        i -= 1;
                        break; }
                    buf += c;
                }
                p.id = Some(atoi(buf.c_str()));
            }
        }
        
        // look up json data
        if (i + 1 < str.length()) {
            i += 1;
            Optional<Json::Value> data = JsonParse(str.substr(i));
            if (!data) { return ParserError(); }
            p.data = std::make_shared<Json::Value>(*data);
        }
        
//        debug('decoded %s as %j', str, p);
        return p;
    }
    
    void Decoder::Destroy() {
        if (reconstructor_) {
            reconstructor_->FinishedReconstruction();
            reconstructor_ = nullptr;
        }
    }
    
    BinaryReconstructor::BinaryReconstructor(const Packet & packet):
    recon_pack_(packet)
    {}
    
    Optional<Packet> BinaryReconstructor::TakeBinaryData(const Data &data) {
        buffers_.push_back(std::make_shared<Data>(data));
        if (buffers_.size() == recon_pack_.attachments.size()) { // done with buffer list
            recon_pack_.attachments = buffers_;
            FinishedReconstruction();
            return Some(recon_pack_);
        }
        return None();
    }
    
    void BinaryReconstructor::FinishedReconstruction() {
        buffers_.clear();
    }
    
    Packet ParserError() {
        Packet packet;
        packet.type = PacketType::Error;
        Json::Value data("parser error");
        packet.data = std::make_shared<Json::Value>(data);
        return packet;
    }
}
}