//
//  IkadenwaEntranceViewController.m
//  Ikadenwa
//
//  Created by omochimetaru on 2016/03/17.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import "IkadenwaEntranceViewController.h"

#import "IkadenwaRoomViewController.h"

@interface IkadenwaEntranceViewController ()

@end

@implementation IkadenwaEntranceViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onKeyboardWillShow:)
                                                 name:UIKeyboardWillShowNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onKeyboardWillHide:)
                                                 name:UIKeyboardWillHideNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onKeyboardDidChangeFrame:)
                                                 name:UIKeyboardDidChangeFrameNotification object:nil];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (IBAction)onEnterButton {
    NSString * roomName = _roomNameField.text;
    NSString * userName = _userNameField.text;
    
    if ([roomName isEqualToString:@""]) {
        [self showError:@"部屋の名前を入力してください。"];
        return;
    }
    
    if ([userName isEqualToString:@""]) {
        [self showError:@"あなたの名前を入力してください。"];
        return;
    }
    
    IkadenwaRoomViewController * vc = [[IkadenwaRoomViewController alloc] initWithNibName:@"IkadenwaRoomViewController" bundle:nil];
    vc.roomName = roomName;
    vc.userName = userName;
    [self presentViewController:vc animated:YES completion:nil];
}

- (void)showError:(NSString *)error {
    UIAlertController * alert = [UIAlertController alertControllerWithTitle:@"エラー"
                                                                    message:error
                                                             preferredStyle:UIAlertControllerStyleAlert];
    [alert addAction:[UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault
                                            handler:^(UIAlertAction * action) {
                                                
                                            }]];
    [self presentViewController:alert animated:YES completion:nil];
}

- (IBAction)onBoardTap {
    [self.view endEditing:YES];
}

- (void)onKeyboardWillShow:(NSNotification *)info {
    CGRect endFrameInWindow = [info.userInfo[UIKeyboardFrameEndUserInfoKey] CGRectValue];
    CGRect endFrame = [self.view convertRect:endFrameInWindow fromView:nil];
    [self avoidKeyboardWithFrame:endFrame];
}

- (void)onKeyboardWillHide:(NSNotification *)info {
    _contentPanelYConstraint.constant = 0;
    
    [self.view setNeedsUpdateConstraints];
    [UIView animateWithDuration:0.25 animations:^(){
        [self.view layoutIfNeeded];
    }];
}

- (void)onKeyboardDidChangeFrame:(NSNotification *)info {
    CGRect endFrameInWindow = [info.userInfo[UIKeyboardFrameEndUserInfoKey] CGRectValue];
    CGRect endFrame = [self.view convertRect:endFrameInWindow fromView:nil];
    [self avoidKeyboardWithFrame:endFrame];
}

- (void)avoidKeyboardWithFrame:(CGRect)frame {
    CGFloat keyboardTopY = CGRectGetMinY(frame);    
    CGFloat panelBottomY = CGRectGetMaxY(_contentPanel.frame) + 20.0;
    
    if (keyboardTopY < panelBottomY) {
        CGFloat offset = keyboardTopY - panelBottomY;
        _contentPanelYConstraint.constant += offset;
        
        [self.view setNeedsUpdateConstraints];
        [UIView animateWithDuration:0.25 animations:^(){
            [self.view layoutIfNeeded];
        }];
    }
}


@end
