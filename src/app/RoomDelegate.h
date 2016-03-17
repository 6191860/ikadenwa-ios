//
//  RoomDelegate..h
//  Ikadenwa
//
//  Created by omochimetaru on 2016/03/16.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import <Foundation/Foundation.h>

#include <nwr/easyrtc/easyrtc.h>

#import "MyScrollView.h"
#import "UserDelegate.h"

@protocol RoomDelegate

- (std::shared_ptr<nwr::ert::Easyrtc>)easyrtc;

- (NSObject<UserDelegate> *)userDelegate;

- (NSTimeInterval)animationDuration;
- (MyScrollView *)roomScrollView;
- (CGRect)roomUserPanelFrameAt:(int)index;

- (void)roomOnLoggedIn;
- (void)roomOnErrorWithCode:(NSString *)code text:(NSString *)text;

@end