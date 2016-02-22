//
//  objc_pointer.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/22.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <memory>
#include <cstddef>
#ifdef __OBJC__
#import <Foundation/Foundation.h>
#endif

namespace nwr {
    struct ObjcPointerImpl;
    
    class ObjcPointer {
    public:
        ObjcPointer();
        ObjcPointer(std::nullptr_t _);
        std::shared_ptr<const ObjcPointerImpl> impl() const;
        std::shared_ptr<ObjcPointerImpl> impl();
        
        bool operator== (const ObjcPointer & cmp) const;
        bool operator< (const ObjcPointer & cmp) const;
        
        explicit operator bool() const;
    private:
        std::shared_ptr<ObjcPointerImpl> impl_;
    };
    
#ifdef __OBJC__
    struct ObjcPointerImpl {
        id pointer;
    };
    
    ObjcPointer ObjcPointerMake(id pointer);
    id ObjcPointerGet(const ObjcPointer & p);
#endif
    
}
