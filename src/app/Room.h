//
//  Room.h
//  Ikadenwa
//
//  Created by omochimetaru on 2016/03/13.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import <Foundation/Foundation.h>
#include <vector>
#include <map>
#include <algorithm>
#include <nwr/easyrtc/easyrtc.h>

#import "User.h"
#import "RoomDelegate.h"

@interface Room : NSObject

@property(nonatomic, weak) NSObject<RoomDelegate> * delegate;
@property(nonatomic, strong) NSString * easyrtcid;
@property(nonatomic, strong) NSArray<User *> * users;
@property(nonatomic, strong) NSString * documentTitle;
@property(nonatomic, strong) NSString * roomName;
@property(nonatomic, strong) NSString * userName;
@property(nonatomic, assign) std::shared_ptr<nwr::jsrtc::MediaStream> localStream;
@property(nonatomic, assign) BOOL localMonitor;

- (instancetype)initWithDelegate:(NSObject<RoomDelegate> *)delegate;
- (User *)userForEasyrtcid:(NSString *)easyrtcid;
- (void)activateWithRoomName:(NSString *)roomName;
- (void)deactivate;
- (void)join;
- (void)muteAll;
- (void)unmuteAll;
- (void)_logout;
- (void)logout;
- (void)handleErrorWithCode:(NSString *)code
                       text:(NSString *)text;



- (UserPanel *)createUserViewAt:(int)index;
- (void)deleteUserView:(UserPanel *)view;
- (void)moveUserView:(UserPanel *)view to:(int)index;


@end
