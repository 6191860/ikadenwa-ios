//
//  EasyrtcDemos.m
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/24.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import "EasyrtcDemos.h"

#import "EasyrtcDemo1ViewController.h"

void EasyrtcDemo1Open(UIViewController * current) {
    UIViewController * vc = [[EasyrtcDemo1ViewController alloc] initWithNibName:@"EasyrtcDemo1ViewController" bundle:nil];
    [current presentViewController:vc animated:YES completion:nil];
}
