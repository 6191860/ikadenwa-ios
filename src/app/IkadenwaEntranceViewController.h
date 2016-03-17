//
//  IkadenwaEntranceViewController.h
//  Ikadenwa
//
//  Created by omochimetaru on 2016/03/17.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import "BaseViewController.h"

@interface IkadenwaEntranceViewController : BaseViewController

@property(nonatomic, retain) IBOutlet UIView * contentPanel;
@property(nonatomic, retain) IBOutlet NSLayoutConstraint * contentPanelYConstraint;
@property(nonatomic, retain) IBOutlet UITextField * roomNameField;
@property(nonatomic, retain) IBOutlet UITextField * userNameField;




@end
