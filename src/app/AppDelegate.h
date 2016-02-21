//
//  App.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/16.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#import <UIKit/UIKit.h>

#include <cstdio>
#include <string>
#include <thread>
#include <memory>

#include <nwr/base/env.h>
#include <nwr/base/string.h>
#include <nwr/base/websocket.h>
#include <nwr/base/timer.h>
#include <nwr/base/base64.h>
#include <nwr/base/time.h>
#include <nwr/base/lib_webrtc.h>
#include <nwr/engineio/yeast.h>
#include <nwr/engineio/socket.h>

#include "eio_test.h"
#include "any_test.h"
#include "sio_test.h"
#include "ert_test.h"

@interface AppDelegate : UIResponder<UIApplicationDelegate> {
    std::shared_ptr<app::EioTest> eio_test_;
    std::shared_ptr<app::AnyTest> any_test_;
    std::shared_ptr<app::SioTest> sio_test_;
    std::shared_ptr<app::ErtTest> ert_test_;
}

- (BOOL)application:(UIApplication *)application
didFinishLaunchingWithOptions:(NSDictionary *)launchOptions;

- (void)applicationWillTerminate:(UIApplication *)application;

- (void)runTest;

@end

