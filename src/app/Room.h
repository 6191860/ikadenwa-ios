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

@protocol Context;
@class User;

@interface Room : NSObject

@property(nonatomic, assign) NSObject<Context> * context;
@property(nonatomic, assign) nwr::Optional<std::string> easyrtcid;
@property(nonatomic, assign) std::vector<User *> users;
@property(nonatomic, assign) std::map<std::string, User *> userDict;
@property(nonatomic, assign) std::string documentTitle;
@property(nonatomic, assign) std::string roomName;
@property(nonatomic, assign) std::string userName;
@property(nonatomic, assign) std::shared_ptr<nwr::jsrtc::MediaStream> localStream;
@property(nonatomic, assign) bool localMonitor;

- (instancetype)initWithContext:(NSObject<Context> *)context;
- (void)activateWithRoomName:(const std::string &)roomName;
- (void)deactivate;
- (void)join;
- (void)muteAll;
- (void)unmuteAll;
- (void)_logout;
- (void)logout;

- (void)handleErrorWithCode:(const std::string &)code
                       text:(const std::string &)text;


@end
