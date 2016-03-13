//
//  BaseViewController.h
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/27.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface BaseViewController : UIViewController

- (BOOL)isForeground;
- (BOOL)isAppeared;
- (BOOL)isStarted;
- (void)onStart;
- (void)onStop;

@end
