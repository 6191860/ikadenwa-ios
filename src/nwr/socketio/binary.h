//
//  binary.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/06.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <vector>
#include <memory>
#include <tuple>
#include <nwr/base/data.h>

#include "packet.h"

namespace nwr {
namespace sio {
    std::tuple<Packet, std::vector<DataPtr>> DeconstructPacket(const Packet & packet);
    Packet ReconstructPacket(Packet packet, const std::vector<DataPtr> & buffers);
}
}