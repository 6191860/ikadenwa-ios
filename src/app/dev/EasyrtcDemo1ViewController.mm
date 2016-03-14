//
//  easyrtc_demo_1.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/24.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import "EasyrtcDemo1ViewController.h"

#include "AppDelegate.h"

using namespace nwr;

@implementation EasyrtcDemo1ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    _destButtons = [NSMutableArray array];
    
//    nwr_test_set_ = std::make_shared<app::NwrTestSet>();
//    nwr_test_set_->TestSio0();
}

- (void)onStart {
    NSLog(@"onStarted");
    _easyrtc = ert::Easyrtc::Create(GetStaticAppDelegate().rtc_factory,
                                    "http://192.168.1.6:8080/",
                                    nullptr);
    _easyrtc->EnableDebug(true);
    [self connect];
}

- (void)onStop {
    
    _easyrtc->Close();
    _easyrtc = nullptr;
    
    NSLog(@"onStopped");
}

- (void)addToConversation:(const std::string &)who
msgType:(const std::string &)msgType
content:(const nwr::Any &)content
{
    NSMutableString * text = [NSMutableString stringWithString:_receiveTextView.text];

    std::string content_str = content.AsString() || std::string();

    [text appendFormat:@"%@: %@\n", ToNSString(who), ToNSString(content_str)];
    _receiveTextView.text = text;
}

- (void)connect {
    _easyrtc->SetPeerListener(None(), None(),
                              [self](const std::string & user,
                                     const std::string & type,
                                     const Any & msg,
                                     const Any & targeting)
                              {
                                  [self addToConversation:user msgType:type content:msg];
                              });
    _easyrtc->set_room_occupant_listener([self](const nwr::Optional<std::string> & roomName,
                                                const std::map<std::string, nwr::Any> & occupants,
                                                const nwr::Any & isPrimary)
                                         {
                                             [self convertListToButtons:roomName occupants:occupants isPrimary:isPrimary];
                                         });
    _easyrtc->Connect("easyrtc.instantMessaging",
                      [self](const std::string & id)
                      {
                          [self loginSuccess:id];
                      },
                      [self](const std::string & code, const std::string & message)
                      {
                          [self loginFailure:code text:message];
                      });    
}

- (void)convertListToButtons:(const Optional<std::string> &)roomName
occupants:(const std::map<std::string, Any> &)occupants
isPrimary:(const Any &)isPrimary
{
    for (UIButton * button in _destButtons) {
        [button removeFromSuperview];
    }
    [_destButtons removeAllObjects];
    
    occupants_.clear();
    for (const auto & easyrtcid : Keys(occupants)) {
        auto occupant = occupants.at(easyrtcid);
        occupants_.push_back(occupant);        
        
        UIButton * button = [UIButton buttonWithType:UIButtonTypeSystem];
        [self.view addSubview:button];
        [_destButtons addObject:button];
        
        [button setTitle:[NSString stringWithFormat:@"send to %@", ToNSString(easyrtcid)]
                forState:UIControlStateNormal];
        [button addTarget:self action:@selector(onDestButton:)
         forControlEvents:UIControlEventTouchUpInside];
    }
    
    for (int i = 0; i < _destButtons.count; i++) {
        UIButton * button = _destButtons[i];
        [button setTranslatesAutoresizingMaskIntoConstraints:NO];
        
        if (i == 0) {
            [self.view addConstraints:
             [NSLayoutConstraint
              constraintsWithVisualFormat:@"V:[a]-(8)-[v]"
              options:0 metrics:nil
              views:@{@"a": _sendTextView,
                      @"v": button}]];
        } else {
            [self.view addConstraints:
             [NSLayoutConstraint
              constraintsWithVisualFormat:@"V:[a]-(8)-[v]"
              options:0 metrics:nil
              views:@{@"a": _destButtons[i-1],
                      @"v": button}]];
        }
        
        [self.view addConstraints:
         [NSLayoutConstraint
          constraintsWithVisualFormat:@"|-(8)-[v]"
          options:0 metrics:nil
          views:@{@"v": button}]];
    }
}

- (void)loginSuccess:(const std::string &)easyrtcid
{
    _myNameLabel.text = [NSString stringWithFormat:@"I am %@", ToNSString(easyrtcid)];
}
- (void)loginFailure:(const std::string &)code
text:(const std::string &)text
{
    _easyrtc->ShowError(code, text);
}

- (IBAction)onTapGesture:(UITapGestureRecognizer *)recr {
    [self.view endEditing:YES];
}

- (void)onDestButton:(UIButton *)button {
    int index = (int)[_destButtons indexOfObject:button];
    const Any & occupant = occupants_[index];
    std::string dest_id = occupant.GetAt("easyrtcid").AsString().value();
    
    auto text = Format("%s", [_sendTextView.text UTF8String]);

    _easyrtc->SendDataWS(Any(dest_id),
                         "message",
                         Any(text),
                         nullptr);
    [self addToConversation:"Me" msgType:"message" content:Any(text)];
    _sendTextView.text = @"";
}

- (IBAction)onCloseButton {
    [self dismissViewControllerAnimated:true completion:nil];
}

@end