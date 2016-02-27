//
//  easyrtc_demo_1.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/24.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#import <UIKit/UIKit.h>

#include <memory>
#include <nwr/base/http_operation.h>
#include <nwr/easyrtc/easyrtc.h>

#import "BaseViewController.h"
#include "nwr_test_set.h"

@interface EasyrtcDemo1ViewController : BaseViewController {
    std::shared_ptr<app::NwrTestSet> nwr_test_set_;
    std::vector<nwr::Any> occupants_;
}

@property(nonatomic, retain) IBOutlet UILabel * myNameLabel;
@property(nonatomic, retain) IBOutlet UITextView * sendTextView;
@property(nonatomic, retain) IBOutlet UITextView * receiveTextView;
@property(nonatomic, retain) NSMutableArray<UIButton *> * destButtons;

@property(nonatomic, assign) std::shared_ptr<nwr::ert::Easyrtc> easyrtc;
@property(nonatomic, assign) std::string selfEasyrtcid;
- (void)addToConversation:(const std::string &)who
msgType:(const std::string &)msgType
content:(const nwr::Any &)content;
- (void)connect;
- (void)convertListToButtons:(const nwr::Optional<std::string> &)roomName
occupants:(const std::map<std::string, nwr::Any> &)occupants
isPrimary:(const nwr::Any &)isPrimary;
- (void)loginSuccess:(const std::string &)easyrtcid;
- (void)loginFailure:(const std::string &)code
text:(const std::string &)text;

@end