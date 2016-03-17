//
//  IkadenwaRoomViewController.h
//  Ikadenwa
//
//  Created by omochimetaru on 2016/03/13.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import "BaseViewController.h"

#include <nwr/easyrtc/easyrtc.h>

#import "Room.h"
#import "User.h"
#import "MyScrollView.h"

@interface IkadenwaRoomViewController : BaseViewController

@property(nonatomic, assign) std::shared_ptr<nwr::ert::Easyrtc> easyrtc;
@property(nonatomic, retain) Room * room;

@property(nonatomic, retain) IBOutlet MyScrollView * scrollView;
@property(nonatomic, retain) IBOutlet UIView * topBar;
@property(nonatomic, retain) IBOutlet UIView * bottomBar;
@property(nonatomic, retain) IBOutlet UIView * loadingCover;
@property(nonatomic, retain) IBOutlet UIImageView * loadingMark;

@property(nonatomic, assign) int testCount;
@property(nonatomic, assign) int testIdCount;

@end
