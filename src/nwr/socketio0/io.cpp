//
//  io.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/25.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "io.h"

namespace nwr {
namespace sio0 {
    std::string Io::version_ = "0.9.16";
    int Io::protocol_ = 1;
    std::map<std::string, std::shared_ptr<CoreSocket>> Io::sockets_;
    
    std::shared_ptr<Socket> Io::Connect(const std::string & url, const SocketOptions & details) {
        
        auto url_parts = ParseUrl(url);
        if (!url_parts.port) {
            if (url_parts.scheme == "http") {
                url_parts.port = Some(80);
            } else {
                url_parts.port = Some(443);
            }
        }
        
        SocketOptions options = details;
        auto & o = options;
        if (!o.host) { o.host = Some(url_parts.hostname); }
        if (!o.secure) { o.secure = Some(url_parts.scheme == "https"); }
        if (!o.port) { o.port = Some(url_parts.port || (url_parts.scheme == "https" ? 443 : 80)); }
        if (!o.query) { o.query = Some(QueryStringEncode(url_parts.query)); }

        std::string unique_url = Format("%s://%s:%d",
                                        *o.secure ? "https" : "http",
                                        (*o.host).c_str(),
                                        (*o.port));
        
        std::shared_ptr<CoreSocket> socket;
        bool force_new = options.force_new_connection || false;
        if (force_new || !HasKey(sockets_, unique_url)) {
            socket = CoreSocket::Create(options);
        }
        if (!force_new && socket) {
            sockets_[unique_url] = socket;
        }
        
        if (!socket) {
            socket = sockets_[unique_url];
        }
        
        // if path is different from '' or /
        std::string path = url_parts.path;
        path = path.length() > 1 ? path : "";

        return socket->Of(path);
    }
}
}