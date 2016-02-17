//
//  parser.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/06.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "parser.h"

#include "binary.h"

namespace nwr {
namespace sio {
    int parser_protocol() {
        return 4;
    }
    
    std::vector<eio::PacketData> Encoder::Encode(const Packet & packet) {
        printf("%s\n", __PRETTY_FUNCTION__);
        
        if (packet.type == PacketType::BinaryEvent || packet.type == PacketType::BinaryAck) {
            return EncodeAsBinary(packet);
        } else {
            std::string encoding = EncodeAsString(packet);
            return { eio::PacketData(encoding) };
        }
    }
    
    std::string EncodeAsString(const Packet & packet) {
        std::string str;
        bool nsp = false;
        
        // first is type
        str += PacketTypeToString(packet.type);
        
        // attachments if we have them
        if (packet.type == PacketType::BinaryEvent || packet.type == PacketType::BinaryAck) {
            str += Format("%d", packet.attachments);
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
            str += packet.data.ToJsonString();
        }
        
//        debug('encoded %j as %s', obj, str);
        
        return str;
    }
    
    std::vector<eio::PacketData> EncodeAsBinary(const Packet & obj) {
        auto deconstruction = DeconstructPacket(obj);
        std::string pack = EncodeAsString(std::get<0>(deconstruction));
        
        std::vector<eio::PacketData> buffers;
        
        buffers = Map(std::get<1>(deconstruction), [](const DataPtr & data){
            return eio::PacketData(data);
        });

        buffers.insert(buffers.begin(), eio::PacketData(pack)); // add packet info to beginning of data list
        return buffers; // write all the buffers
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
                if (reconstructor_->recon_pack().attachments == 0) {
                    
                    decoded_emitter_->Emit(packet);
                }
            } else { // non-binary full packet
                decoded_emitter_->Emit(packet);
            }
                
        } else {
            if (!reconstructor_) {
                Fatal("got binary data when not reconstructing a packet");
            } else {
                Optional<Packet> packet = reconstructor_->TakeBinaryData(data.binary);
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
            p.attachments = atoi(buf.c_str());
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
            auto data = JsonParse(str.substr(i));
            if (!data) { return ParserError(); }
            p.data = Any::FromJson(*data);
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
    
    Optional<Packet> BinaryReconstructor::TakeBinaryData(const DataPtr &data) {
       buffers_.push_back(data);
        if (buffers_.size() == recon_pack_.attachments) { // done with buffer list
            auto packet = ReconstructPacket(recon_pack_, buffers_);
            FinishedReconstruction();
            return Some(packet);
        }
        return None();
    }
    
    void BinaryReconstructor::FinishedReconstruction() {
        buffers_.clear();
    }
    
    Packet ParserError() {
        Packet packet;
        packet.type = PacketType::Error;
        packet.data = Any("parser error");
        return packet;
    }
}
}