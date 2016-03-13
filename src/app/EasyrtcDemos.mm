//
//  EasyrtcDemos.m
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/24.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import "EasyrtcDemos.h"

#import "EasyrtcDemo1ViewController.h"
#import "EasyrtcDemo2ViewController.h"
#import "EasyrtcDemo3ViewController.h"


void EasyrtcDemo1Open(UIViewController * current) {
    UIViewController * vc = [[EasyrtcDemo1ViewController alloc] initWithNibName:@"EasyrtcDemo1ViewController" bundle:nil];
    [current presentViewController:vc animated:YES completion:nil];
}

void EasyrtcDemo2Open(UIViewController * current) {
    UIViewController * vc = [[EasyrtcDemo2ViewController alloc] initWithNibName:@"EasyrtcDemo2ViewController" bundle:nil];
    [current presentViewController:vc animated:YES completion:nil];
}

void EasyrtcDemo3Open(UIViewController * current) {
    UIViewController * vc = [[EasyrtcDemo3ViewController alloc] initWithNibName:@"EasyrtcDemo3ViewController" bundle:nil];
    [current presentViewController:vc animated:YES completion:nil];
}