//
//  AppDelegate.h
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/16.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import <UIKit/UIKit.h>

#include "app/app.h"

@interface AppDelegate : UIResponder<UIApplicationDelegate> {
    app::App * app_;
}

@property(nonatomic, retain) UIWindow * window;

- (BOOL)application:(UIApplication *)application
    didFinishLaunchingWithOptions:(NSDictionary *)launchOptions;

@end
