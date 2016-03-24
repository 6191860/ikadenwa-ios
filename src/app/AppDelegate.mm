//
//  App.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/16.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import "AppDelegate.h"

#import "IkadenwaEntranceViewController.h"
#import "DebugMenuViewController.h"

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application
didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    _rtc_factory = std::make_shared<nwr::jsrtc::RtcPeerConnectionFactory>();
    
    bool initialized = rtc::InitializeSSL();
    if (!initialized) {
        nwr::Fatal("InitializeSSL failed");
    }
    
    UIWindow * window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
    self.window = window;
    [window makeKeyAndVisible];
    
    BOOL isDebug = [[[NSBundle mainBundle] objectForInfoDictionaryKey:@"isDebug"] boolValue];
    
    UIViewController * vc;
    if (isDebug) {
        vc = [[DebugMenuViewController alloc] initWithNibName:@"DebugMenuViewController" bundle:nil];
    } else {
        vc = [[IkadenwaEntranceViewController alloc] initWithNibName:@"IkadenwaEntranceViewController" bundle:nil];
    }
    
    window.rootViewController = vc;
    
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

