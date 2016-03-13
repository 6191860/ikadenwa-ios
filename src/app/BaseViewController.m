//
//  BaseViewController.m
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/27.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import "BaseViewController.h"

@interface BaseViewController ()
@property(nonatomic, assign) BOOL isForeground;
@property(nonatomic, assign) BOOL isAppeared;
@property(nonatomic, assign) BOOL isStarted;
@end

@implementation BaseViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    _isForeground = [UIApplication sharedApplication].applicationState != UIApplicationStateBackground;
    _isAppeared = NO;
    _isStarted = NO;
    
    [[NSNotificationCenter defaultCenter]
     addObserver:self selector:@selector(onForeground)
     name:UIApplicationWillEnterForegroundNotification object:nil];
    [[NSNotificationCenter defaultCenter]
     addObserver:self selector:@selector(onBackground)
     name:UIApplicationDidEnterBackgroundNotification object:nil];
}

- (void)onForeground {
    self.isForeground = YES;
}

- (void)onBackground {
    self.isForeground = NO;
}

- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
    
    self.isAppeared = YES;
}

- (void)viewDidDisappear:(BOOL)animated {
    self.isAppeared = NO;
    
    [super viewDidDisappear:animated];
}

- (void)setIsForeground:(BOOL)isForeground {
    _isForeground = isForeground;
    [self updateStarted];
}

- (void)setIsAppeared:(BOOL)isAppeared {
    _isAppeared = isAppeared;
    [self updateStarted];
}

- (void)updateStarted {
    BOOL value = _isForeground && _isAppeared;
    if (_isStarted != value) {
        _isStarted = value;
        if (value) {
            [self onStart];
        } else {
            [self onStop];
        }
    }
}

- (void)onStart {}
- (void)onStop {}


@end
