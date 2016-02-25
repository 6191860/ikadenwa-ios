//
//  RTCHtmlMediaElementView.m
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/22.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import "NWRHtmlMediaElementView.h"

#include "media_stream.h"

using namespace nwr;
using namespace nwr::jsrtc;

static CGRect CGSizeFit(CGSize size, CGRect bounds) {
    CGFloat x, y, w, h;
    if (size.width * CGRectGetHeight(bounds) >= CGRectGetWidth(bounds) * size.height) {
        w = CGRectGetWidth(bounds);
        h = w * size.height / size.width;
    } else {
        h = CGRectGetHeight(bounds);
        w = h * size.width / size.height;
    }
    x = CGRectGetMinX(bounds) + 0.5 * (CGRectGetWidth(bounds) - w);
    y = CGRectGetMinY(bounds) + 0.5 * (CGRectGetHeight(bounds) - h);
    return CGRectMake(x, y, w, h);
}

@interface NWRHtmlMediaElementView() <RTCEAGLVideoViewDelegate>

@property(nonatomic, retain, readonly) RTCVideoRendererAdapter * glVideoViewAdapter;

- (void)commonInit;
@end

@implementation NWRHtmlMediaElementView

- (instancetype)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        [self commonInit];
    }
    return self;
}

- (nullable instancetype)initWithCoder:(NSCoder *)aDecoder {
    self = [super initWithCoder: aDecoder];
    if (self) {
        [self commonInit];
    }
    return self;
}

- (void)commonInit {
    _glVideoView = [[RTCEAGLVideoView alloc] initWithFrame:self.bounds];
    _glVideoView.delegate = self;
    [self addSubview:_glVideoView];
    _glVideoView.translatesAutoresizingMaskIntoConstraints = YES;
    
    _glVideoViewAdapter = [[RTCVideoRendererAdapter alloc] initWithNativeRenderer:_glVideoView];
    
    _videoSize = CGSizeMake(0, 0);
    
    _videoSizeEmitter = std::make_shared<Emitter<CGSize>>();
}

- (void)dealloc {
    self.srcObject = nullptr;
}

- (void)setVideoSize:(CGSize)videoSize {
    _videoSize = videoSize;
    _videoSizeEmitter->Emit(videoSize);
    [self setNeedsLayout];
}

- (void)layoutSubviews {
    [super layoutSubviews];
    
    CGRect videoSizeFrame = CGSizeFit(_videoSize, self.bounds);
    _glVideoView.frame = videoSizeFrame;
}

- (void)setSrcObject:(std::shared_ptr<nwr::jsrtc::MediaStream>)srcObject {
    if (_srcObject) {
        for (const std::shared_ptr<MediaStreamTrack> & track : _srcObject->video_tracks()) {
            track->RemoveVideoRenderer(*_glVideoViewAdapter.nativeVideoRenderer);
        }
    }
    _srcObject = srcObject;
    if (srcObject) {
        if (srcObject->video_tracks().size() > 0) {
            srcObject->video_tracks().front()->AddVideoRenderer(*_glVideoViewAdapter.nativeVideoRenderer);
        }
    }
    
    self.videoSize = CGSizeZero;
}

-(void)videoView:(RTCEAGLVideoView *)videoView didChangeVideoSize:(CGSize)size {
    self.videoSize = size;
}

@end
