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
#include <nwr/base/lib_webrtc.h>
#include <nwr/jsrtc/rtc_peer_connection_factory.h>

@interface AppDelegate : UIResponder<UIApplicationDelegate> {
}

@property(nonatomic, retain) UIWindow * window;

@property(nonatomic, assign) std::shared_ptr<nwr::jsrtc::RtcPeerConnectionFactory> rtc_factory;

- (BOOL)application:(UIApplication *)application
didFinishLaunchingWithOptions:(NSDictionary *)launchOptions;

- (void)applicationWillTerminate:(UIApplication *)application;

@end

AppDelegate * GetStaticAppDelegate();


