//
//  Context.h
//  Ikadenwa
//
//  Created by omochimetaru on 2016/03/13.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include <nwr/easyrtc/easyrtc.h>

#import <Foundation/Foundation.h>

@protocol Context
@property(nonatomic, assign, readonly) std::shared_ptr<nwr::ert::Easyrtc> easyrtc;
@end
