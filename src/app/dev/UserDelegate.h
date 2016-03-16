//
//  UserDelegate.h
//  Ikadenwa
//
//  Created by omochimetaru on 2016/03/15.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import <Foundation/Foundation.h>

#include <nwr/easyrtc/easyrtc.h>

@class User;

@protocol UserDelegate

- (std::shared_ptr<nwr::ert::Easyrtc>)easyrtc;

@end


