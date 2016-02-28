//
//  EasyrtcDemo2ViewController.h
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/28.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import <UIKit/UIKit.h>

#include <string>
#include <map>

#include <nwr/easyrtc/easyrtc.h>

#import "BaseViewController.h"

@interface EasyrtcDemo2ViewController : BaseViewController {
    std::string self_easyrtcid_;
    
    std::vector<nwr::Any> occupants_;
    
    std::map<std::string, bool> connect_list_;
    std::map<std::string, bool> channel_is_active_; // tracks which channels are active
}

@property(nonatomic, retain) IBOutlet UILabel * myNameLabel;
@property(nonatomic, retain) IBOutlet UITextView * sendTextView;
@property(nonatomic, retain) IBOutlet UITextView * receiveTextView;
@property(nonatomic, retain) NSMutableArray<UIView *> * destViews;

@property(nonatomic, assign) std::shared_ptr<nwr::ert::Easyrtc> easyrtc;

@end
