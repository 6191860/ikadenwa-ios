//
//  objc_pointer.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/22.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "objc_pointer.h"

namespace nwr {
    ObjcPointer::ObjcPointer():
    impl_(std::make_shared<ObjcPointerImpl>())
    {}
    
    ObjcPointer::ObjcPointer(std::nullptr_t _): ObjcPointer(){}
    
    
    std::shared_ptr<const ObjcPointerImpl> ObjcPointer::impl() const {
        return impl_;
    }
    
    std::shared_ptr<ObjcPointerImpl> ObjcPointer::impl() {
        return impl_;
    }
    
    bool ObjcPointer::operator== (const ObjcPointer & cmp) const {
        return impl()->pointer == cmp.impl()->pointer;
    }
    bool ObjcPointer::operator< (const ObjcPointer & cmp) const {
        return impl()->pointer < cmp.impl()->pointer;
    }
    
    ObjcPointer::operator bool() const {
        return impl()->pointer != nil;
    }

    ObjcPointer ObjcPointerMake(id pointer) {
        ObjcPointer ret;
        ret.impl()->pointer = pointer;
        return ret;
    }
    
    id ObjcPointerGet(const ObjcPointer & p) {
        return p.impl()->pointer;
    }

}