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

@interface AppDelegate : UIResponder<UIApplicationDelegate> {
}

@property(nonatomic, retain) UIWindow * window;

- (BOOL)application:(UIApplication *)application
didFinishLaunchingWithOptions:(NSDictionary *)launchOptions;

- (void)applicationWillTerminate:(UIApplication *)application;

@end

