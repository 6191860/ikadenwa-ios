//
//  parser.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/19.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

//  https://github.com/socketio/engine.io-parser

#pragma once

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <algorithm>

#include <nwr/base/env.h>
#include <nwr/base/string.h>
#include <nwr/base/base64.h>
#include <nwr/base/websocket.h>

#include "packet.h"

namespace nwr {
namespace eio {
    
    int parser_protocol();
    
    Packet MakeParserErrorPacket(const std::string & error);
    
    Websocket::Message EncodePacket(const Packet & packet);
    Websocket::Message EncodeBuffer(const Packet & packet);
    Packet DecodePacket(const Websocket::Message & message);
    Packet DecodeBase64Packet(const Websocket::Message & message);
    
}
}


