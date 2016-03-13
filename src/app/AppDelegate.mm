//
//  App.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/16.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "AppDelegate.h"

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application
didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    _rtc_factory = std::make_shared<nwr::jsrtc::RtcPeerConnectionFactory>();
    
    bool initialized = rtc::InitializeSSL();
    if (!initialized) {
        nwr::Fatal("InitializeSSL failed");
    }
    
    return YES;
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    bool initialized = rtc::CleanupSSL();
    if (!initialized) {
        nwr::Fatal("CleanupSSL failed");
    }
    
    _rtc_factory = nullptr;
}

@end

AppDelegate * GetStaticAppDelegate() {
    return (AppDelegate *)[UIApplication sharedApplication].delegate;
}

