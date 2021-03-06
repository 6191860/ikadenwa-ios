//
//  RTCHtmlMediaElementView.h
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/22.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import <UIKit/UIKit.h>

#include <memory>

#include <nwr/base/emitter.h>
#include <nwr/base/lib_webrtc.h>

namespace nwr {
namespace jsrtc {
    class MediaStream;
    class MediaStreamTrack;
}
}

@interface NWRHtmlMediaElementView : UIView

@property(nonatomic, assign) std::shared_ptr<nwr::jsrtc::MediaStream> srcObject;
@property(nonatomic, assign, readonly) std::shared_ptr<nwr::jsrtc::MediaStreamTrack> srcObjectVideoTrack;
@property(nonatomic, retain, readonly) RTCEAGLVideoView * glVideoView;
@property(nonatomic, retain, readonly) RTCVideoRendererAdapter * glVideoViewAdapter;
@property(nonatomic, assign, readonly) CGSize videoSize;
@property(nonatomic, retain, readonly) RTCCameraPreviewView * cameraPreviewView;

@property(nonatomic, assign, readonly) std::shared_ptr<nwr::Emitter<CGSize>> videoSizeEmitter;

@end
