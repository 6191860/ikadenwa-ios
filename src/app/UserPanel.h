//
//  UserPanel.h
//  Ikadenwa
//
//  Created by omochimetaru on 2016/03/14.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface UserPanel : UIView

@property(nonatomic, retain) IBOutlet UILabel * nameLabel;
@property(nonatomic, retain) IBOutlet UIButton * myselfDummyButton;
@property(nonatomic, retain) IBOutlet UIButton * connectButton;
@property(nonatomic, retain) IBOutlet UIButton * offButton;
@property(nonatomic, retain) IBOutlet UIButton * onButton;


@property(nonatomic, assign) CGFloat cornerRadius;

+ (UserPanel *)load;

@end
