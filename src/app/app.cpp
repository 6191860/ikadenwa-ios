//
//  App.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/16.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "app.h"



namespace app {

    void App::Test1() {
        any_test_ = std::make_shared<AnyTest>();
        ert_test_ = std::make_shared<ErtTest>();
        
        
//        any_test_->Run();
        ert_test_->Run();
    }
}