//
//  MyScrollView.m
//  Ikadenwa
//
//  Created by omochimetaru on 2016/03/14.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import "MyScrollView.h"

@implementation MyScrollView

- (BOOL)touchesShouldCancelInContentView:(UIView *)view {
    if ([view isKindOfClass:[UIButton class]]) {
        return YES;
    }
    
    return [super touchesShouldCancelInContentView:view];
}

@end
