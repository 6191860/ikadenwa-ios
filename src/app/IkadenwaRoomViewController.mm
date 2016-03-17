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

- (void)viewDidLoad {
    [super viewDidLoad];
    
    _allOffButton.layer.cornerRadius = 4.0;
    _allOffButton.clipsToBounds = YES;
    _allOnButton.layer.cornerRadius = 4.0;
    _allOnButton.clipsToBounds = YES;
}

- (void)onStart {
    self.automaticallyAdjustsScrollViewInsets = NO;
    self.title = _roomName;
    
    [self startLoadingCover];
    
    _userAgent = std::make_shared<IkadenwaRoomUserAgent>();
    _userAgent->owner = self;
    _easyrtc = ert::Easyrtc::Create(GetStaticAppDelegate().rtc_factory,
                                    "https://ikadenwa.ink/",
                                    _userAgent.get());
    
    _room = [[Room alloc] initWithDelegate:self];
    [_room activateWithRoomName:_roomName userName:_userName];
}

- (void)onStop {
    [self stopLoadingCover];
    
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

- (IBAction)onLeaveButton {
    [self dismissViewControllerAnimated:YES completion:nil];
}

- (IBAction)onAllOffButton {
    [_room muteAll];
}

- (IBAction)onAllOnButton {
    [_room unmuteAll];
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

- (void)roomOnLoggedIn {
    [self stopLoadingCover];
}

- (void)roomOnErrorWithCode:(NSString *)code text:(NSString *)text {
    if ([self isLoadingCoverOn]) {
        [UIView animateWithDuration:[self animationDuration]
                         animations:^(){
                             _loadingMark.alpha = 0.0;
                         }];
    }
    
    NSString * message = [NSString stringWithFormat:@"code=[%@]\ntext=[%@]", code, text];
    UIAlertController * alert = [UIAlertController alertControllerWithTitle:@"エラー"
                                                                    message:message
                                                             preferredStyle:UIAlertControllerStyleAlert];
    UIAlertAction * okButton = [UIAlertAction actionWithTitle:@"OK"
                                                        style:UIAlertActionStyleDefault
                                                      handler:^(UIAlertAction * action){
                                                          [self onLeaveButton];
                                                      }];
    [alert addAction:okButton];
    [self presentViewController:alert animated:YES completion:nil];
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

- (BOOL)isLoadingCoverOn{
    return _loadingCover.superview != nil && _loadingCover.alpha == 1.0;
}

- (void)startLoadingCover {
    if ([self isLoadingCoverOn]) {
        return;
    }
    
    if (_loadingCover.superview != nil) {
        [_loadingCover removeFromSuperview];
    }
    
    [self.view addSubview:_loadingCover];
    
    CGFloat topY = CGRectGetMaxY(_topBar.frame);
    CGFloat bottomY = CGRectGetMaxY(_bottomBar.frame);
    _loadingCover.frame = CGRectMake(0, topY,
                                     CGRectGetWidth(self.view.bounds),
                                     bottomY - topY);
    _loadingCover.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    _loadingCover.alpha = 0.0;
    [UIView animateWithDuration:[self animationDuration]
                     animations:^(){
                         _loadingCover.alpha = 1.0;
                     }];
    _loadingMark.alpha = 1.0;
    
    CAKeyframeAnimation * animation = [CAKeyframeAnimation animationWithKeyPath:@"transform.rotation"];
    animation.values = @[@(0.0), @(M_PI_2), @(M_PI), @(3 * M_PI_2), @(2 * M_PI)];
    animation.keyTimes = @[@(0.0), @(0.25), @(0.5), @(0.75), @(1.0)];
    animation.duration = 2.0;
    animation.repeatCount = HUGE_VALF;
    [_loadingMark.layer addAnimation:animation forKey:@"rotationAnimation"];
}

- (void)stopLoadingCover {
    if (![self isLoadingCoverOn]) {
        return;
    }
    
    [UIView animateWithDuration:[self animationDuration]
                     animations:^(){
                         _loadingCover.alpha = 0.0;
                     }
                     completion:^(BOOL finihsed){
                         [_loadingCover removeFromSuperview];
                     }];
}


@end
