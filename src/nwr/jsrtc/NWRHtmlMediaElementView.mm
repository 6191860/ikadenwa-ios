//
//  RTCHtmlMediaElementView.m
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/22.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import "NWRHtmlMediaElementView.h"

#include "media_stream.h"
#include "media_stream_track.h"

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
- (void)commonInit;
- (void)removeInnerVideoView;
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
    _videoSize = CGSizeMake(0, 0);
    _videoSizeEmitter = std::make_shared<Emitter<CGSize>>();
}

- (void)dealloc {    
    self.srcObject = nullptr;
}

- (void)removeInnerVideoView {
    if (_glVideoView) {
        [_glVideoView removeFromSuperview];
        _glVideoView = nil;
        _glVideoViewAdapter = nil;
    }
    if (_cameraPreviewView) {
        [_cameraPreviewView removeFromSuperview];
        _cameraPreviewView = nil;
    }
}

- (void)setVideoSize:(CGSize)videoSize {
    _videoSize = videoSize;
    _videoSizeEmitter->Emit(videoSize);
    [self setNeedsLayout];
}

- (void)layoutSubviews {
    [super layoutSubviews];
    
    if (_glVideoView) {
        CGRect videoSizeFrame;
        if (_videoSize.width == 0.0 || _videoSize.height == 0.0) {
            videoSizeFrame = CGRectZero;
        } else {
            videoSizeFrame = CGSizeFit(_videoSize, self.bounds);
        }
        _glVideoView.frame = videoSizeFrame;
    }
    
    if (_cameraPreviewView) {
        _cameraPreviewView.frame = self.bounds;
    }
}

- (void)setSrcObject:(std::shared_ptr<nwr::jsrtc::MediaStream>)srcObject {
    if (_srcObjectVideoTrack) {
        
        printf("[NWRHtmlMediaElementView] _srcObjectVideoTrack=%p\n", _srcObjectVideoTrack.get());
        
        if (_glVideoView) {
            _srcObjectVideoTrack->RemoveVideoRenderer(*_glVideoViewAdapter.nativeVideoRenderer);
        }
        
        if (_cameraPreviewView) {
            _cameraPreviewView.captureSession = nil;
        }
    }
    _srcObject = srcObject;
    _srcObjectVideoTrack = nullptr;
    
    if (srcObject) {
        if (srcObject->video_tracks().size() > 0) {
            _srcObjectVideoTrack = srcObject->video_tracks().front();
        }
    }
    
    if (_srcObjectVideoTrack) {
        if (_srcObjectVideoTrack->remote()) {
            if (!_glVideoView) {
                [self removeInnerVideoView];
                
                _glVideoView = [[RTCEAGLVideoView alloc] initWithFrame:self.bounds];
                _glVideoView.delegate = self;
                [self addSubview:_glVideoView];
                _glVideoView.translatesAutoresizingMaskIntoConstraints = YES;
                _glVideoViewAdapter = [[RTCVideoRendererAdapter alloc] initWithVideoRenderer:_glVideoView];
            }
            
            _srcObjectVideoTrack->AddVideoRenderer(*_glVideoViewAdapter.nativeVideoRenderer);
            
            self.videoSize = CGSizeZero;
        } else {
            if (!_cameraPreviewView) {
                [self removeInnerVideoView];
                
                _cameraPreviewView = [[RTCCameraPreviewView alloc] initWithFrame:self.bounds];
                [self addSubview:_cameraPreviewView];
            }
            
            auto av_video_capturer = static_cast<webrtc::AVFoundationVideoCapturer *>(_srcObjectVideoTrack->inner_video_source()->GetVideoCapturer());
            
            _cameraPreviewView.captureSession = av_video_capturer->GetCaptureSession();
            
            self.videoSize = CGSizeMake(640, 480);
        }
    } else {
        [self removeInnerVideoView];
        self.videoSize = CGSizeZero;
    }
    
    
}

-(void)videoView:(RTCEAGLVideoView *)videoView didChangeVideoSize:(CGSize)size {
    self.videoSize = size;
}

@end
