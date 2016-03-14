//
//  User.h
//  Ikadenwa
//
//  Created by omochimetaru on 2016/03/13.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import <Foundation/Foundation.h>
#include <string>
#include <nwr/easyrtc/easyrtc.h>

@protocol Context;

@interface User : NSObject

@property(nonatomic, assign) NSObject<Context> * context;
@property(nonatomic, assign) std::string easyrtcid;
@property(nonatomic, assign) std::string name;
@property(nonatomic, assign) bool joined;
@property(nonatomic, assign) bool connected;
@property(nonatomic, assign) bool muted;
@property(nonatomic, assign) int volume;
@property(nonatomic, assign) nwr::Optional<std::string> stream;

- (instancetype)initWithContext:(NSObject<Context> *)context
                      easyrtcId:(const std::string &)easyrtcid
                           name:(const std::string &)name
                         joined:(bool)joined;
- (void)toggle;
- (void)connect;
- (void)embedStream:(const std::shared_ptr<nwr::jsrtc::MediaStream> &)stream;
- (void)removeStream;
- (void)requestUnmute;
- (void)requestMute;
- (void)_requestMuteWithFlag:(bool)flag;
- (void)muteWithFlag:(bool)flag;


@end
