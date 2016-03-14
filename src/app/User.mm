//
//  User.m
//  Ikadenwa
//
//  Created by omochimetaru on 2016/03/13.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import "User.h"

#import "Context.h"

using namespace nwr;
using namespace nwr::jsrtc;

@implementation User

- (instancetype)initWithContext:(NSObject<Context> *)context
                      easyrtcId:(const std::string &)easyrtcid
                           name:(const std::string &)name
                         joined:(bool)joined
{
    self = [super init];
    if (!self) { return nil; }
    
    _context = context;
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
    _context.easyrtc->Call(_easyrtcid, None(),
                           [self](const std::string & easyrtcid,
                              const std::string & mediatype)
                           {
                               _connected = true;
                           },
                           [self](const std::string & error_code,
                              const std::string & error_text)
                           {
                               _connected = false;
                           },
                           [](bool accepted, const std::string & easyrtcid)
                           {
                               
                           });
}

- (void)embedStream:(const std::shared_ptr<nwr::jsrtc::MediaStream> &)stream
{
    _connected = true;
    _muted = false;
    _stream = Some(std::string("stream_") + _easyrtcid);
//    this.audio = document.createElement('audio');
//    this.audio.setAttribute('id', this.stream);
//    this.audio.autoplay = true;
//    document.body.appendChild(this.audio);
//    easyrtc.setVideoObjectSrc(this.audio, stream);
}

- (void)removeStream
{
//    document.body.removeChild(this.audio);
    _stream = None();
}

- (void)requestUnmute {
    if (!_connected) {
        [self connect];
    }
    else {
        [self _requestMuteWithFlag:false];
    }
}

- (void)requestMute {
    if (_connected) {
        [self _requestMuteWithFlag:true];
    }
}

- (void)_requestMuteWithFlag:(bool)flag {
    _context.easyrtc->SendData(_easyrtcid,
                               "mute",
                               Any(Any::ObjectType
                                   {
                                       { "mute", Any(flag) }
                                   }),
                               nullptr);
    [self muteWithFlag:flag];
}

- (void)muteWithFlag:(bool)flag {
//    TODO
//    easyrtc.muteVideoObject(this.audio, flg);
    _muted = flag;
}

@end
