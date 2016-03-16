//
//  ViewLoader.h
//  Ikadenwa
//
//  Created by omochimetaru on 2016/03/14.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import <Foundation/Foundation.h>

#import <UIKit/UIKit.h>

@interface ViewLoader : NSObject

@property(nonatomic, strong) IBOutlet UIView * view;

- (UIView *)loadViewFromNib:(NSString *)nibName;

@end
