//
//  IkadenwaRoomViewController.m
//  Ikadenwa
//
//  Created by omochimetaru on 2016/03/13.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import "IkadenwaRoomViewController.h"

#import "AppDelegate.h"

using namespace nwr;
using namespace jsrtc;

struct IkadenwaRoomUserAgent : ert::UserAgentInterface {
    IkadenwaRoomViewController * __weak owner;
    virtual ObjcPointer GetElementById(const ert::ElementId & id) {
        return nullptr;
    }
    virtual void AddElement(const ObjcPointer & element) {
        [owner.view addSubview:(UIView *)ObjcPointerGet(element)];
    }
};

@interface IkadenwaRoomViewController ()

@property(nonatomic, assign) std::shared_ptr<IkadenwaRoomUserAgent> userAgent;

@end

@implementation IkadenwaRoomViewController

- (void)onStart {
    _userAgent = std::make_shared<IkadenwaRoomUserAgent>();
    _userAgent->owner = self;
    
    _easyrtc = ert::Easyrtc::Create(GetStaticAppDelegate().rtc_factory,
                                    "https://ikadenwa.ink/",
                                    _userAgent.get());
    
    _room = [[Room alloc] initWithContext:self];
    [_room activateWithRoomName:"omochimetaru"];
}

- (void)onStop {
    if (_room) {
        [_room deactivate];
        _room = nil;
    }
    
    if (_easyrtc) {
        _easyrtc->Close();
        _easyrtc = nullptr;
    }
    _userAgent = nullptr;
}



@end
