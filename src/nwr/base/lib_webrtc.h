//
//  lib_webrtc.h
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/21.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#define WEBRTC_POSIX
#define WEBRTC_IOS

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"

#include <TargetConditionals.h>

#include <webrtc/base/scoped_ptr.h>
#include <webrtc/base/refcount.h>
#include <webrtc/base/thread.h>
#include <webrtc/base/ssladapter.h>

#include <webrtc/api/audiotrack.h>
#include <webrtc/api/datachannel.h>
#include <webrtc/api/datachannelinterface.h>
#include <webrtc/api/jsep.h>
#include <webrtc/api/mediaconstraintsinterface.h>
#include <webrtc/api/mediastream.h>
#include <webrtc/api/mediastreamtrack.h>
#include <webrtc/api/mediastreaminterface.h>
#include <webrtc/api/peerconnectioninterface.h>
#include <webrtc/api/videotrack.h>

#ifdef __OBJC__
#   import <webrtc/api/objc/RTCVideoRenderer.h>
#   import <webrtc/api/objc/RTCEAGLVideoView.h>
#   import <webrtc/api/objc/avfoundationvideocapturer.h>
#   import <webrtc/api/objc/RTCVideoRendererAdapter+Private.h>
#endif

#pragma clang diagnostic pop

