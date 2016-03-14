//
//  EasyrtcDemo3ViewController.m
//  Ikadenwa
//
//  Created by omochimetaru on 2016/03/12.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import "EasyrtcDemo3ViewController.h"

#import "AppDelegate.h"

using namespace nwr;

class EasyrtcDemo3ViewControllerUserAgent : public ert::UserAgentInterface {
public:
    EasyrtcDemo3ViewController * __weak owner_;
    virtual ObjcPointer GetElementById(const ert::ElementId & id) {
        if (id == "selfVideo") {
            return ObjcPointerMake(owner_.myVideoArea);
        } else if (id == "callerVideo"){
            return ObjcPointerMake(owner_.peerVideoArea);
        } else {
            return nullptr;
        }
    }
    virtual void AddElement(const ObjcPointer & element) {
        UIView * view = ObjcPointerGet(element);
        [owner_.view addSubview:view];
    }
};

@interface EasyrtcDemo3ViewController ()

@end

@implementation EasyrtcDemo3ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    _destViews = [NSMutableArray array];
    
    _user_agent = std::make_shared<EasyrtcDemo3ViewControllerUserAgent>();
    _user_agent->owner_ = self;
}

- (void)onStart {
    _easyrtc = ert::Easyrtc::Create(GetStaticAppDelegate().rtc_factory,
                                    "http://192.168.1.6:8080/",
                                    _user_agent.get());
    _easyrtc->EnableDebug(true);
    [self connect];
}

- (void)onStop {
    if (_easyrtc) {
        _easyrtc->Close();
        _easyrtc = nullptr;
    }
}

- (void)connect {
    _easyrtc->set_video_dims(640, 480, None());
    _easyrtc->set_room_occupant_listener([self](const Optional<std::string> & room_name,
                                                const std::map<std::string, Any> & occupant_list,
                                                const Any & is_primary)
                                         {
                                             [self convertListToButtons:room_name
                                                              occupants:occupant_list
                                                              isPrimary:is_primary];
                                         });
    _easyrtc->EasyApp("easyrtc.audioVideoSimple",
                      Some(std::string("selfVideo")),
                      { "callerVideo" },
                      [self](const std::string & easyrtcid)
                      {
                          [self loginSuccess:easyrtcid];
                      },
                      [self](const std::string & errorCode, const std::string & message)
                      {
                          [self loginFailure:errorCode text:message];
                      });
}

- (void)convertListToButtons:(const Optional<std::string> &)roomName
                   occupants:(const std::map<std::string, Any> &)occupants
                   isPrimary:(const Any &)isPrimary
{
    for (UIView * view in _destViews) {
        [view removeFromSuperview];
    }
    [_destViews removeAllObjects];
    
    _occupants.clear();
    for (const auto & easyrtcid : Keys(occupants)) {
        auto occupant = occupants.at(easyrtcid);
        _occupants.push_back(occupant);
        
        printf("%s\n", occupant.ToJsonString().c_str());
        
        UIView * destView = [[UIView alloc] initWithFrame:CGRectZero];
        [self.view addSubview:destView];
        [_destViews addObject:destView];
        
        UILabel * nameLabel = [[UILabel alloc] initWithFrame:CGRectZero];
        [destView addSubview:nameLabel];
        nameLabel.tag = 1;
        nameLabel.font = [UIFont systemFontOfSize:12.0];
        nameLabel.text = ToNSString(_easyrtc->IdToName(easyrtcid));
        nameLabel.translatesAutoresizingMaskIntoConstraints = NO;
        [destView addConstraints:
         [NSLayoutConstraint
          constraintsWithVisualFormat:@"V:|-(0)-[v]"
          options:0 metrics:nil views:@{@"v": nameLabel}]];
        [destView addConstraints:
         [NSLayoutConstraint
          constraintsWithVisualFormat:@"H:|-(0)-[v]"
          options:0 metrics:nil views:@{@"v": nameLabel}]];
        
        UIButton * connectButton = [UIButton buttonWithType:UIButtonTypeSystem];
        [destView addSubview:connectButton];
        connectButton.tag = 2;
        connectButton.titleLabel.font = [UIFont systemFontOfSize:12.0];
        [connectButton setTitle:@"connect" forState:UIControlStateNormal];
        [connectButton addTarget:self action:@selector(onConnectButton:) forControlEvents:UIControlEventTouchUpInside];
        connectButton.translatesAutoresizingMaskIntoConstraints = NO;
        [destView addConstraints:
         [NSLayoutConstraint
          constraintsWithVisualFormat:@"V:|-(0)-[v]"
          options:0 metrics:nil views:@{@"n": nameLabel, @"v": connectButton}]];
        [destView addConstraints:
         [NSLayoutConstraint
          constraintsWithVisualFormat:@"H:[n]-(4)-[v]"
          options:0 metrics:nil views:@{@"n": nameLabel, @"v": connectButton}]];
    }
    
    for (int i = 0; i < _destViews.count; i++) {
        UIView * destView = _destViews[i];
        destView.translatesAutoresizingMaskIntoConstraints = NO;
        
        if (i == 0) {
            [self.view addConstraints:
             [NSLayoutConstraint
              constraintsWithVisualFormat:@"V:[a]-(8)-[v]"
              options:0 metrics:nil
              views:@{@"a": _myNameLabel,
                      @"v": destView}]];
        } else {
            [self.view addConstraints:
             [NSLayoutConstraint
              constraintsWithVisualFormat:@"V:[a]-(8)-[v]"
              options:0 metrics:nil
              views:@{@"a": _destViews[i-1],
                      @"v": destView}]];
        }
        
        [self.view addConstraints:
         [NSLayoutConstraint
          constraintsWithVisualFormat:@"V:[v(h)]"
          options:0 metrics:@{@"h": @(30)}
          views:@{@"v": destView}]];
        
        [self.view addConstraints:
         [NSLayoutConstraint
          constraintsWithVisualFormat:@"H:|-(8)-[v]-(8)-|"
          options:0 metrics:nil
          views:@{@"v": destView}]];
    }
    
    for (const auto & easyrtcid : Keys(occupants)) {
        [self updateButtonState:easyrtcid];
    }
}

- (void)updateButtonState:(const std::string &)other_easyrtcid {
    bool is_connected = _channel_is_active[other_easyrtcid];
    
    int index = -1;
    for (int i = 0; i < _occupants.size(); i++) {
        const auto & occupant = _occupants[i];
        std::string userid = occupant.GetAt("easyrtcid").AsString().value();
        if (other_easyrtcid == userid) {
            index = i;
            break;
        }
    }
    
    if (index == -1) { return; }
    
    UIButton * connectButton = [_destViews[index] viewWithTag:2];
    if (connectButton) {
        connectButton.enabled = !is_connected;
    }
}


- (void)performCall:(std::string)otherEasyrtcid {
    _easyrtc->HangupAll();
    
    _easyrtc->Call(otherEasyrtcid,
                   None(),
                   [](const std::string &, const std::string &) {
                   },
                   [](const std::string &, const std::string &) {
                   },
                   [](bool, const std::string &){
                   });
}

- (void)loginSuccess:(const std::string &)easyrtcid
{
    _self_easyrtcid = easyrtcid;
    _myNameLabel.text = [NSString stringWithFormat:@"I am %@", ToNSString(easyrtcid)];
}

- (void)loginFailure:(const std::string &)code
                text:(const std::string &)text
{
    _easyrtc->ShowError(code, "failure to login");
}

- (void)onConnectButton:(UIButton *)button {
    int index = -1;
    for (int i = 0; i < _destViews.count; i++) {
        UIView * destView = _destViews[i];
        if (button == [destView viewWithTag:2]) {
            index = i;
            break;
        }
    }
    if (index == -1) { return; }
    
    std::string user = _occupants[index].GetAt("easyrtcid").AsString().value();
    [self performCall:user];
    
}

- (IBAction)onCloseButton {
    [self dismissViewControllerAnimated:true completion:nil];
}



@end
