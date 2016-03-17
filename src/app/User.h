//
//  User.h
//  Ikadenwa
//
//  Created by omochimetaru on 2016/03/13.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include <string>
#include <nwr/easyrtc/easyrtc.h>

#import "UserPanel.h"
#import "UserDelegate.h"

@interface User : NSObject

@property(nonatomic, weak) NSObject<UserDelegate> * delegate;
@property(nonatomic, strong) NSString * easyrtcid;
@property(nonatomic, strong) NSString * name;
@property(nonatomic, assign) double joined;
@property(nonatomic, assign) BOOL isMyself;
@property(nonatomic, assign) BOOL connected;
@property(nonatomic, assign) BOOL muted;
@property(nonatomic, assign) int volume;
@property(nonatomic, assign) std::shared_ptr<nwr::jsrtc::MediaStreamTrack> audioTrack;

@property(nonatomic, strong) UserPanel * view;

- (instancetype)initWithDelegate:(NSObject<UserDelegate> *)delegate
                       easyrtcId:(NSString *)easyrtcid
                            name:(NSString *)name
                          joined:(double)joined;
- (void)toggle;
- (void)connect;
- (void)embedStream:(const std::shared_ptr<nwr::jsrtc::MediaStream> &)stream;
- (void)removeStream;
- (void)requestUnmute;
- (void)requestMute;
- (void)_requestMuteWithFlag:(BOOL)flag;
- (void)muteWithFlag:(BOOL)flag;

- (void)onConnectButton;
- (void)onOffButton;
- (void)onOnButton;

- (UIColor *)whiteColor;
- (UIColor *)grayColor;
- (UIColor *)darkGrayColor;
- (UIColor *)blackColor;
- (void)updateView;

@end

User * UserFind(NSArray<User *> * users, int * index, BOOL (^pred)(User *));
User * UserFindByEasyrtcid(NSArray<User *> * users, int * index, NSString * easyrtcid);

