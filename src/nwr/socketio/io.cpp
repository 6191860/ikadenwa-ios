//
//  io.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/11.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "io.h"

#include "url.h"
#include "manager.h"

namespace nwr {
namespace sio {
    std::map<std::string, std::shared_ptr<Manager>> cache_;
    
    IoParams::IoParams():
    force_new(false),
    multiplex(true)
    {}
    
    std::shared_ptr<Socket> Io(const std::string & uri, const eio::Socket::ConstructorParams & params) {
        auto parsed = UrlMake(uri);
        auto source = parsed.source;
        auto id = parsed.id;
        auto path = parsed.path;
        
        bool same_namespace = cache_[id] && cache_[id]->nsps()[path];
        
        bool new_connection = params.force_new ||
        params.multiplex == false || same_namespace;
        
        std::shared_ptr<Manager> io;
        
        if (new_connection) {
            printf("[%s] ignoring socket cache for %s\n", __PRETTY_FUNCTION__, source.c_str());
            io = Manager::Create(source, params);
        } else {
            if (!cache_[id]) {
                printf("[%s] new io instance for %s\n", __PRETTY_FUNCTION__, source.c_str());
                cache_[id] = Manager::Create(source, params);
            }
            io = cache_[id];
        }
        
        return io->GetSocket(path);
    }
}
}