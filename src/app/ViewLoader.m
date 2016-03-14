//
//  ViewLoader.m
//  Ikadenwa
//
//  Created by omochimetaru on 2016/03/14.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import "ViewLoader.h"

@implementation ViewLoader

- (UIView *)loadViewFromNib:(NSString *)nibName {
    [[NSBundle mainBundle] loadNibNamed:nibName owner:self options:nil];
    return self.view;
}

@end
