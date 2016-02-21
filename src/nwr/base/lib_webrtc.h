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

#include <webrtc/base/scoped_ptr.h>
#include <webrtc/base/refcount.h>
#include <webrtc/base/thread.h>
#include <webrtc/base/ssladapter.h>

#include <talk/app/webrtc/audiotrack.h>
#include <talk/app/webrtc/datachannel.h>
#include <talk/app/webrtc/datachannelinterface.h>
#include <talk/app/webrtc/jsep.h>
#include <talk/app/webrtc/mediaconstraintsinterface.h>
#include <talk/app/webrtc/mediastream.h>
#include <talk/app/webrtc/mediastreamtrack.h>
#include <talk/app/webrtc/mediastreaminterface.h>
#include <talk/app/webrtc/peerconnectioninterface.h>
#include <talk/app/webrtc/videotrack.h>

#pragma clang diagnostic pop

