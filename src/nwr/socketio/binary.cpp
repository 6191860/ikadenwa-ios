//
//  binary.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/06.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "binary.h"

namespace nwr {
namespace sio {
    Any _DeconstructPacket(const Any & data, std::vector<DataPtr> & buffers);
    Any _ReconstructPacket(Any data, const std::vector<DataPtr> & buffers,
                           int & cur_place_holder);
    bool _HasBinary(const Any & data);
    
    std::tuple<Packet, std::vector<DataPtr>> DeconstructPacket(const Packet & packet) {
        std::vector<DataPtr> buffers;
        Any packetData = packet.data;

        Packet pack = packet;
        pack.data = _DeconstructPacket(packetData, buffers);
        pack.attachments = static_cast<int>(buffers.size());
        
        return std::make_tuple(pack, buffers);
    }
    
    Any _DeconstructPacket(const Any & data, std::vector<DataPtr> & buffers) {
        
        if (data.type() == Any::Type::Data) {
            Any placeholder = Any(Any::ObjectType {
                { "_placeholder", Any(true) },
                { "num", Any(static_cast<int>(buffers.size())) }
            });
            buffers.push_back(data.AsData().value());
            return placeholder;
        } else if (data.type() == Any::Type::Array) {
            Any new_data = Any(Any::ArrayType{});
            for (int i = 0; i < data.count(); i++) {
                new_data.SetAt(i, _DeconstructPacket(data.GetAt(i), buffers));
            }
            return new_data;
        } else if (data.type() == Any::Type::Object) {
            Any new_data(Any::ObjectType{});
            for (auto key : data.keys()) {
                new_data.SetAt(key, _DeconstructPacket(data.GetAt(key), buffers));
            }
            return new_data;
        }
        
        return data;
    }
    
    Packet ReconstructPacket(Packet packet, const std::vector<DataPtr> & buffers) {
        int cur_place_holder = 0;
        
        packet.data = _ReconstructPacket(packet.data, buffers, cur_place_holder);
        packet.attachments = 0; // no longer useful
        return packet;
    }
    
    Any _ReconstructPacket(Any data, const std::vector<DataPtr> & buffers,
                           int & cur_place_holder)
    {
        if (data.HasKey("_placeholder")) {
            auto buf = buffers[data.GetAt("num").AsInt().value()]; // appropriate buffer (should be natural order anyway)
            return Any(buf);
        } else if (data.type() == Any::Type::Array) {
            for (int i = 0; i < data.count(); i++) {
                data.SetAt(i, _ReconstructPacket(data.GetAt(i), buffers, cur_place_holder));
            }
            return data;
        } else if (data.type() == Any::Type::Object) {
            for (auto key : data.keys()) {
                data.SetAt(key, _ReconstructPacket(data.GetAt(key), buffers, cur_place_holder));
            }
            return data;
        }
        return data;
    }
    
    bool HasBinary(const Any & data) {
        return _HasBinary(data);
    }
    
    bool _HasBinary(const Any & data) {
        if (data.type() == Any::Type::Data) {
            return true;
        }
        if (data.type() == Any::Type::Array) {
            for (int i = 0; i < data.count(); i++) {
                if (_HasBinary(data.GetAt(i))) {
                    return true;
                }
            }
        } else if (data.type() == Any::Type::Object) {
            for (auto key : data.keys()) {
                if (_HasBinary(data.GetAt(key))) {
                    return true;
                }
            }
        }
        return false;
    }
}
}