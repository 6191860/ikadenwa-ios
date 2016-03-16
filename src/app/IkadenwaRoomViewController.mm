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

@interface IkadenwaRoomViewController () <RoomDelegate, UserDelegate>

@property(nonatomic, assign) std::shared_ptr<IkadenwaRoomUserAgent> userAgent;

@end

@implementation IkadenwaRoomViewController

- (void)onStart {
    _testCount = 0;
    _testIdCount = 0;
    
    self.automaticallyAdjustsScrollViewInsets = NO;
    self.title = @"omochimetaru";
    
    _userAgent = std::make_shared<IkadenwaRoomUserAgent>();
    _userAgent->owner = self;
    _easyrtc = ert::Easyrtc::Create(GetStaticAppDelegate().rtc_factory,
                                    "https://ikadenwa.ink/",
                                    _userAgent.get());
    
    _room = [[Room alloc] initWithDelegate:self];
    [_room activateWithRoomName:@"omochimetaru"];
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


- (void)viewDidLayoutSubviews {
    [super viewDidLayoutSubviews];
    
    self.scrollView.contentSize = [self scrollViewContentSize];
}

- (IBAction)onTestButton {
    switch (_testCount) {
        case 0: {
            NSMutableArray * users = [NSMutableArray arrayWithArray:_room.users];
            User * user = [[User alloc] initWithDelegate:[self userDelegate]
                                               easyrtcId:[NSString stringWithFormat:@"id_%d", _testIdCount]
                                                    name:@"太郎"
                                                  joined:NO];
            _testIdCount += 1;
            [users addObject:user];
            _room.users = users;
            
            break;
        }
            
    }
    
    _testCount += 1;
    
    if (_testCount > 0) {
        _testCount = 0;
    }
}

- (std::shared_ptr<nwr::ert::Easyrtc>)easyrtc {
    return _easyrtc;
}

- (MyScrollView *)roomScrollView {
    return _scrollView;
}

- (NSObject<UserDelegate> *)userDelegate {
    return self;
}

- (NSTimeInterval)animationDuration {
    return 0.25;
}

- (CGRect)roomUserPanelFrameAt:(int)index {
    int row = index / 2;
    int col = index % 2;
    
    CGFloat areaWidth = CGRectGetWidth(_scrollView.frame);
    
    CGFloat w = (areaWidth - 10.0 * 3.0) / 2.0;
    w = (int)w - (int)w % 2;
    CGFloat h = 90;
    
    return CGRectMake(10 + (areaWidth - 10) / 2.0 * col,
                      10 + row * (90 + 10),
                      w, h);
}

- (CGSize)scrollViewContentSize {
    int userNum = (int)_room.users.count;
    int rowNum = 1;
    if (userNum > 0) {
        rowNum = (userNum - 1) / 2 + 1;
    }
    
    int y = 10 + rowNum * (90 + 10);
    
    return CGSizeMake(CGRectGetWidth(_scrollView.frame), y);
}



@end
