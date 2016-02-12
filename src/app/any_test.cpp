
//  any_test.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/10.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "any_test.h"

int test_index = 0;
#define ASSERT(x) assert_body(x, #x)

void assert_body(bool x, const char * s) {
    if(x) {
        printf("ok %d - %s\n", test_index, s);
    } else {
        printf("not ok %d - %s\n", test_index, s);
    }
    test_index += 1;
}

namespace app {
    using namespace nwr;
    
    void AnyTest::Run() {
        Any a(std::map<std::string, Any> {
            { "a", Any(3) },
            { "b", Any("bbb") },
            { "c", Any(std::vector<Any> {
                Any(11),
                Any(22),
                Any(33)
            }) }
        });
        
        ASSERT(a.GetAt("a").AsInt() == Some(3));
        ASSERT(a.GetAt("b").AsString() == Some(std::string("bbb")));
        ASSERT(a.GetAt("c").GetAt(0).AsInt() == Some(11));
        ASSERT(a.GetAt("c").GetAt(1).AsInt() == Some(22));
        ASSERT(a.GetAt("c").GetAt(2).AsInt() == Some(33));
        
        //  ref copy
        Any b = a;
        b.SetAt("a", Any("aaa"));
        ASSERT(b.GetAt("a").AsString() == Some(std::string("aaa")));
        ASSERT(a.GetAt("a").AsString() == Some(std::string("aaa")));
        
        //  value copy
        Any c = b.GetAt("b");
        c = Any(3);
        ASSERT(c.AsInt() == Some(3));
        ASSERT(b.GetAt("b").AsString() == Some(std::string("bbb")));
        
        Any d = Any(Any::ObjectType {
            { "aa", Any(Any::ObjectType {
                { "bb", Any(1) }
            }) }
        });
        ASSERT(d.GetAt("aa").GetAt("bb").AsInt() == Some(1));
        d.GetAt("aa").SetAt("bb", Any(2));
        ASSERT(d.GetAt("aa").GetAt("bb").AsInt() == Some(2));
        
    }
}