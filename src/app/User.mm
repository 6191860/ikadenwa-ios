//
//  User.m
//  Ikadenwa
//
//  Created by omochimetaru on 2016/03/13.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import "User.h"

using namespace nwr;
using namespace nwr::jsrtc;

@implementation User

- (instancetype)initWithDelegate:(NSObject<UserDelegate> *)delegate
                       easyrtcId:(NSString *)easyrtcid
                            name:(NSString *)name
                          joined:(double)joined
{
    self = [super init];
    if (!self) { return nil; }
    
    _delegate = delegate;
    _easyrtcid = easyrtcid;
    _name = name;
    _joined = joined;
    _connected = false;
    _muted = true;
    _volume = 100;
    
//    ko.track(this);
//    ko.getObservable(this, 'volume').subscribe(function (volume) {
//        if (_this.audio) {
//            _this.audio.volume = volume / 100;
//        }
//    });
    
    return self;
}

- (void)setView:(UserPanel *)view {
    if (_view) {
        [NSException raise:@"invalid state" format:@""];
    }
    
    _view = view;
    [self updateView];
    
    [view.connectButton addTarget:self action:@selector(onConnectButton)
                 forControlEvents:UIControlEventTouchUpInside];
    [view.offButton addTarget:self action:@selector(onOffButton)
             forControlEvents:UIControlEventTouchUpInside];
    [view.onButton addTarget:self action:@selector(onOnButton)
            forControlEvents:UIControlEventTouchUpInside];
}


- (void)toggle {
    if (!_connected) {
        [self connect];
    }
    else if (_muted) {
        [self requestUnmute];
    }
    else {
        [self requestMute];
    }
}

- (void)connect {
    auto easyrtc = _delegate.easyrtc;
    easyrtc->Call(ToString(_easyrtcid), None(),
                  [self](const std::string & easyrtcid,
                         const std::string & mediatype)
                  {
                      _connected = true;
                      [self updateView];
                  },
                  [self](const std::string & error_code,
                         const std::string & error_text)
                  {
                      _connected = false;
                      [self updateView];
                  },
                  [](bool accepted, const std::string & easyrtcid)
                  {
                      
                  });
}

- (void)embedStream:(const std::shared_ptr<nwr::jsrtc::MediaStream> &)stream
{
    _connected = true;
    _muted = false;
    [self updateView];
    
    _audioTrack = stream->audio_tracks().front();
    if (_audioTrack) {
        webrtc::AudioSourceInterface * audioSource = _audioTrack->inner_audio_source();
        audioSource->SetVolume(10.0);
    }
    
//    this.audio = document.createElement('audio');
//    this.audio.setAttribute('id', this.stream);
//    this.audio.autoplay = true;
//    document.body.appendChild(this.audio);
//    easyrtc.setVideoObjectSrc(this.audio, stream);
}

- (void)removeStream
{
//    document.body.removeChild(this.audio);
    
    _audioTrack = nil;
}

- (void)requestUnmute {
    if (!_connected) {
        [self connect];
    }
    else {
        [self _requestMuteWithFlag:NO];
    }
}

- (void)requestMute {
    if (_connected) {
        [self _requestMuteWithFlag:YES];
    }
}

- (void)_requestMuteWithFlag:(BOOL)flag {
    auto easyrtc = _delegate.easyrtc;
    easyrtc->SendData(ToString(_easyrtcid),
                      "mute",
                      Any(Any::ObjectType
                          {
                              { "mute", Any(flag) }
                          }),
                      nullptr);
    [self muteWithFlag:flag];
}

- (void)muteWithFlag:(BOOL)flag {
//    TODO
//    easyrtc.muteVideoObject(this.audio, flg);
    
    if (_audioTrack) {
        webrtc::AudioSourceInterface * audioSource = _audioTrack->inner_audio_source();
        audioSource->SetVolume(!flag ? 10.0 : 0.0);
    }
    
    _muted = flag;
    [self updateView];
}

- (void)onConnectButton {
    [self requestUnmute];
}
- (void)onOffButton {
    [self requestMute];
}
- (void)onOnButton {
    [self requestUnmute];
}

- (UIColor *)whiteColor {
    return [UIColor colorWithRed:1.0 green:1.0 blue:1.0 alpha:1.0];
}
- (UIColor *)grayColor {
    return [UIColor colorWithRed:0.8 green:0.8 blue:0.8 alpha:1.0];
}
- (UIColor *)darkGrayColor {
    return [UIColor colorWithRed:0.5 green:0.5 blue:0.5 alpha:1.0];
}
- (UIColor *)blackColor {
    return [UIColor colorWithRed:0.0 green:0.0 blue:0.0 alpha:1.0];
}

- (void)updateView {
    
    _view.nameLabel.text = _name;
    
    BOOL isOn = _connected && !_muted;
    if (isOn) {
        _view.backgroundColor = [self whiteColor];
        _view.nameLabel.textColor = [self blackColor];
    } else {
        _view.backgroundColor = [self grayColor];
        _view.nameLabel.textColor = [self darkGrayColor];
    }
    
    if (!_connected) {
        _view.connectButton.hidden = NO;
        _view.offButton.hidden = YES;
        _view.onButton.hidden = YES;
    } else {
        _view.connectButton.hidden = YES;
        _view.offButton.hidden = NO;
        _view.onButton.hidden = NO;
    }
    
    if (_muted) {
        _view.offButton.enabled = NO;
        _view.onButton.enabled = YES;
    } else {
        _view.offButton.enabled = YES;
        _view.onButton.enabled = NO;
    }
}

@end

User * UserFind(NSArray<User *> * users, int * index, BOOL (^pred)(User *)) {
    for (int i = 0; i < users.count; i++) {
        User * user = users[i];
        if (pred(user)) {
            if (index) { *index = i; }
            return user;
        }
    }
    if (index) { *index = -1; }
    return nil;
}

User * UserFindByEasyrtcid(NSArray<User *> * users, int * index, NSString * easyrtcid) {
    return UserFind(users, index, ^(User * x){
        return [x.easyrtcid isEqualToString:easyrtcid];
    });
}