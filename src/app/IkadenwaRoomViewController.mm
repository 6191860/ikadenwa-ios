//
//  IkadenwaRoomViewController.m
//  Ikadenwa
//
//  Created by omochimetaru on 2016/03/13.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import "IkadenwaRoomViewController.h"

#import "AppDelegate.h"

#import "UserPanel.h"

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
    self.automaticallyAdjustsScrollViewInsets = NO;
    self.title = @"omochimetaru";
    
    CGRect frame;
    
    for (int i = 0; i < 4; i++) {
    
        UserPanel * panel0 = [UserPanel load];
        [self.scrollView addSubview:panel0];
        frame = panel0.frame;
        frame.origin = CGPointMake(10, 10 + i * (90 + 10));
        panel0.frame = frame;
        
        UserPanel * panel1 = [UserPanel load];
        [self.scrollView addSubview:panel1];
        frame = panel1.frame;
        frame.origin = CGPointMake(CGRectGetMaxX(panel0.frame) + 10,
                                   CGRectGetMinY(panel0.frame));
        panel1.frame = frame;
        
    }
    
//    frame = self.scrollViewBackBoard.frame;
//    frame.size = CGSizeMake(frame.size.width, 1000);
//    self.scrollViewBackBoard.frame = frame;
//    [self.scrollViewBackBoard setNeedsUpdateConstraints];
    
//    self.scrollView.contentSize = CGSizeMake(self.scrollView.contentSize.width,
//                                             1000);
    return;
    
    _userAgent = std::make_shared<IkadenwaRoomUserAgent>();
    _userAgent->owner = self;
    
    _easyrtc = ert::Easyrtc::Create(GetStaticAppDelegate().rtc_factory,
                                    "https://ikadenwa.ink/",
                                    _userAgent.get());
    
    _room = [[Room alloc] initWithContext:self];
    [_room activateWithRoomName:"omochimetaru"];
}

- (void)viewDidLayoutSubviews {
    [super viewDidLayoutSubviews];
    
    self.scrollView.contentSize = CGSizeMake(320, 1000);
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
