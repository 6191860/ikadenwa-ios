//
//  EasyrtcDemo3ViewController.h
//  Ikadenwa
//
//  Created by omochimetaru on 2016/03/12.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import <UIKit/UIKit.h>

#include <nwr/easyrtc/easyrtc.h>

#import "BaseViewController.h"

class EasyrtcDemo3ViewControllerUserAgent;

@interface EasyrtcDemo3ViewController : BaseViewController {
    std::shared_ptr<nwr::ert::Easyrtc> _easyrtc;
    std::shared_ptr<EasyrtcDemo3ViewControllerUserAgent> _user_agent;
    std::string _self_easyrtcid;
    std::vector<nwr::Any> _occupants;
    std::map<std::string, bool> _channel_is_active;
    
    std::shared_ptr<nwr::jsrtc::RtcPeerConnectionFactory> _pcf;
    std::shared_ptr<nwr::jsrtc::MediaStreamTrack> _test_track;
}

@property(nonatomic, retain) IBOutlet UILabel * myNameLabel;
@property(nonatomic, retain) IBOutlet NWRHtmlMediaElementView * myVideoArea;
@property(nonatomic, retain) IBOutlet NWRHtmlMediaElementView * peerVideoArea;

@property(nonatomic, retain) NSMutableArray<UIView *> * destViews;


@end
