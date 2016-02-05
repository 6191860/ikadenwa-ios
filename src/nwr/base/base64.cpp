//
//  base64.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/20.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "base64.h"

#include <openssl/ssl.h>

#include "util.h"

namespace nwr {
    void Base64Encode(const Data & data, Data & dest_str) {
        BIO * bio = BIO_new(BIO_s_mem());
        
        BIO * base64 = BIO_new(BIO_f_base64());
        bio = BIO_push(base64, bio);
        
        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        BIO_write(bio, &data[0], (int)data.size());
        BIO_flush(bio);
        
        BUF_MEM * buf_mem;
        BIO_get_mem_ptr(bio, &buf_mem);
        
        dest_str.resize(buf_mem->length);
        std::copy(buf_mem->data, buf_mem->data + buf_mem->length, dest_str.begin());
        
        BIO_free_all(bio);
    }
    
    int Base64CalcDecodedSize(const Data & str) {
        const char * p = reinterpret_cast<const char *>(&str[0]);
        const int len = static_cast<int>(str.size());
        
        int padding = 0;
        if (2 <= len &&
            p[len-2] == '=' &&
            p[len-1] == '=')
        {
            padding = 2;
        } else if (1 <= len &&
                   p[len-1] == '=')
        {
            padding = 1;
        }
        
        return (len * 3) / 4 - padding;
    }
    
    void Base64Decode(const Data & str, Data & dest_data) {
        const int decoded_size = Base64CalcDecodedSize(str);
        dest_data.resize(decoded_size);
        
        const int str_len = static_cast<int>(str.size());
        BIO * bio = BIO_new_mem_buf(const_cast<uint8_t *>(&str[0]), str_len);
        
        BIO * base64 = BIO_new(BIO_f_base64());
        bio = BIO_push(base64, bio);
        
        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        int len = BIO_read(bio, &dest_data[0], str_len);
        if (len != decoded_size) {
            Fatal(Format("assertion failure: len(%d) == decoded_size(%d)", len, decoded_size));
        }
        
        BIO_free_all(bio);
    }
}