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

//    void DeconstructPacket(const Packet & packet) {
//        std::vector<std::shared_ptr<Data>> buffers;
//        
//        
//        function _deconstructPacket(data) {
//            if (!data) return data;
//            
//            if (isBuf(data)) {
//                var placeholder = { _placeholder: true, num: buffers.length };
//                buffers.push(data);
//                return placeholder;
//            } else if (isArray(data)) {
//                var newData = new Array(data.length);
//                for (var i = 0; i < data.length; i++) {
//                    newData[i] = _deconstructPacket(data[i]);
//                }
//                return newData;
//            } else if ('object' == typeof data && !(data instanceof Date)) {
//                var newData = {};
//                for (var key in data) {
//                    newData[key] = _deconstructPacket(data[key]);
//                }
//                return newData;
//            }
//            return data;
//        }
//        
//        Packet pack = packet;
//        
//        
//        
//        
//        pack.data = _deconstructPacket(packetData);
//        pack.attachments = buffers.length; // number of binary 'attachments'
//        return {packet: pack, buffers: buffers};
//    }
}
}