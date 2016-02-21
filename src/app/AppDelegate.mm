//
//  App.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/16.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "AppDelegate.h"

@implementation AppDelegate

using namespace app;

- (BOOL)application:(UIApplication *)application
didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    bool initialized = rtc::InitializeSSL();
    if (!initialized) {
        nwr::Fatal("InitializeSSL failed");
    }
    
    [self runTest];
    
    return YES;
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    bool initialized = rtc::CleanupSSL();
    if (!initialized) {
        nwr::Fatal("CleanupSSL failed");
    }
}

- (void)runTest {
    ert_test_ = std::make_shared<app::ErtTest>();
    ert_test_->Run();
}

@end

