//
//  DebugMenuViewController.m
//  Ikadenwa
//
//  Created by omochimetaru on 2016/03/13.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import "DebugMenuViewController.h"

#import "EasyrtcDemo1ViewController.h"
#import "EasyrtcDemo2ViewController.h"
#import "EasyrtcDemo3ViewController.h"
#import "IkadenwaRoomViewController.h"
#import "ikadenwaEntranceViewController.h"

@implementation DebugMenuViewController

- (IBAction)onDemo1{
    UIViewController * vc = [[EasyrtcDemo1ViewController alloc] initWithNibName:@"EasyrtcDemo1ViewController" bundle:nil];
    [self presentViewController:vc animated:YES completion:nil];
}

- (IBAction)onDemo2{
    UIViewController * vc = [[EasyrtcDemo2ViewController alloc] initWithNibName:@"EasyrtcDemo2ViewController" bundle:nil];
    [self presentViewController:vc animated:YES completion:nil];
}

- (IBAction)onDemo3{
    UIViewController * vc = [[EasyrtcDemo3ViewController alloc] initWithNibName:@"EasyrtcDemo3ViewController" bundle:nil];
    [self presentViewController:vc animated:YES completion:nil];
}

- (IBAction)onIkadenwa {
    UIViewController * vc = [[IkadenwaRoomViewController alloc] initWithNibName:@"IkadenwaRoomViewController" bundle:nil];
    
    [self presentViewController:vc animated:YES completion:nil];
}

- (IBAction)onEntrance {
    UIViewController * vc = [[IkadenwaEntranceViewController alloc] initWithNibName:@"IkadenwaEntranceViewController" bundle:nil];
    [self presentViewController:vc animated:YES completion:nil];
}

@end