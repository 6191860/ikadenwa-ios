//
//  UserPanel.m
//  Ikadenwa
//
//  Created by omochimetaru on 2016/03/14.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import "UserPanel.h"

#import "ViewLoader.h"

@implementation UserPanel

- (CGFloat)cornerRadius {
    return self.layer.cornerRadius;
}
- (void)setCornerRadius:(CGFloat)cornerRadius {
    self.layer.cornerRadius = cornerRadius;
}

+ (UserPanel *)load {
    UserPanel * it = (UserPanel *)[[[ViewLoader alloc] init] loadViewFromNib:@"UserPanel"];
    it.cornerRadius = 4.0;
    return it;
}

@end
