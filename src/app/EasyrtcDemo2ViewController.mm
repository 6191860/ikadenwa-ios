//
//  EasyrtcDemo2ViewController.m
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/28.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import "EasyrtcDemo2ViewController.h"

using namespace nwr;

@implementation EasyrtcDemo2ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    _destViews = [NSMutableArray array];
}

- (void)onStarted {
    
    jsrtc::MediaTrackConstraints consts;
    bool x;
    size_t a = 0;
    
    webrtc::FindConstraint(&consts.inner_constraints(),
                           webrtc::MediaConstraintsInterface::kOfferToReceiveAudio, &x,
                           &a);
    
    _easyrtc = ert::Easyrtc::Create("http://192.168.1.6:8080/",
                                    ObjcPointerMake(self.view));
    _easyrtc->EnableDebug(true);
    [self connect];
}

- (void)onStopped {
    if (_easyrtc) {
        _easyrtc->Close();
        _easyrtc = nullptr;
    }
}

- (void)connect {
    _easyrtc->EnableDebug(true);
    _easyrtc->EnableDataChannels(true);
    _easyrtc->EnableVideo(false);
    _easyrtc->EnableAudio(false);
    _easyrtc->EnableVideoReceive(false);
    _easyrtc->EnableAudioReceive(false);
    _easyrtc->set_data_channel_open_listener([self](const std::string & other_party, bool v){
        [self openListener:other_party];
    });
    _easyrtc->set_data_channel_close_listener([self](const std::string & other_party) {
        [self closeListener:other_party];
    });
    _easyrtc->SetPeerListener(None(), None(),
                              [self](const std::string & who,
                                     const std::string & msg_type,
                                     const Any & content,
                                     const Any & target)
                              {
                                  [self addToConversation:who msgType:msg_type content:content];
                              });
    _easyrtc->set_room_occupant_listener([self](const Optional<std::string> & room_name,
                                                 const std::map<std::string, Any> & occupant_list,
                                                 const Any & is_primary)
                                         {
                                             [self convertListToButtons:room_name
                                                              occupants:occupant_list
                                                              isPrimary:is_primary];
                                         });
    _easyrtc->Connect("easyrtc.dataMessaging",
                      [self](const std::string & easyrtcid)
                      {
                          [self loginSuccess:easyrtcid];
                      },
                      [self](const std::string & errorCode, const std::string & message)
                      {
                          [self loginFailure:errorCode text:message];
                      });
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

- (void)openListener:(const std::string &)other_party {
    channel_is_active_[other_party] = true;
    [self updateButtonState:other_party];
}

- (void)closeListener:(const std::string &)other_party {
    channel_is_active_[other_party] = false;
    [self updateButtonState:other_party];
}

- (void)convertListToButtons:(const Optional<std::string> &)roomName
                   occupants:(const std::map<std::string, Any> &)occupants
                   isPrimary:(const Any &)isPrimary
{
    for (UIView * view in _destViews) {
        [view removeFromSuperview];
    }
    [_destViews removeAllObjects];
    
    occupants_.clear();
    for (const auto & easyrtcid : Keys(occupants)) {
        auto occupant = occupants.at(easyrtcid);
        occupants_.push_back(occupant);
        
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
        
        UIButton * sendButton = [UIButton buttonWithType:UIButtonTypeSystem];
        [destView addSubview:sendButton];
        sendButton.tag = 3;
        sendButton.titleLabel.font = [UIFont systemFontOfSize:12.0];
        [sendButton setTitle:@"send" forState:UIControlStateNormal];
        [sendButton addTarget:self action:@selector(onSendButton:) forControlEvents:UIControlEventTouchUpInside];
        sendButton.translatesAutoresizingMaskIntoConstraints = NO;
        [destView addConstraints:
         [NSLayoutConstraint
          constraintsWithVisualFormat:@"V:|-(0)-[v]"
          options:0 metrics:nil views:@{@"n": nameLabel, @"v": sendButton}]];
        [destView addConstraints:
         [NSLayoutConstraint
          constraintsWithVisualFormat:@"H:[l]-(4)-[v]"
          options:0 metrics:nil views:@{@"l": connectButton ,@"v": sendButton}]];        
    }
    
    for (int i = 0; i < _destViews.count; i++) {
        UIView * destView = _destViews[i];
        destView.translatesAutoresizingMaskIntoConstraints = NO;
        
        if (i == 0) {
            [self.view addConstraints:
             [NSLayoutConstraint
              constraintsWithVisualFormat:@"V:[a]-(8)-[v]"
              options:0 metrics:nil
              views:@{@"a": _sendTextView,
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
    bool is_connected = channel_is_active_[other_easyrtcid];
    
    int index = -1;
    for (int i = 0; i < occupants_.size(); i++) {
        const auto & occupant = occupants_[i];
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
    
    UIButton * sendButton = [_destViews[index] viewWithTag:3];
    if (sendButton) {
        sendButton.enabled = is_connected;
    }
}

- (void)startCall:(const std::string &)other_easyrtcid {
    if (_easyrtc->GetConnectStatus(other_easyrtcid) == ert::Easyrtc::ConnectStatus::NotConnected)
    {
        _easyrtc->Call(other_easyrtcid, None(),
                       [self, other_easyrtcid](const std::string & caller, const std::string & media)
                       {
                           if (media == "datachannel") {
                               self->connect_list_[other_easyrtcid] = true;
                           }
                       },
                       [self, other_easyrtcid](const std::string & error_code, const std::string & error_text){
                           self->connect_list_[other_easyrtcid] = false;
                           self->_easyrtc->ShowError(error_code, error_text);
                       },
                       [self](bool was_accepted, const std::string & user){
                           
                       });
    }
    else {
        _easyrtc->ShowError("ALREADY-CONNECTED",
                            Format("already connected to %s", _easyrtc->IdToName(other_easyrtcid).c_str()));
    }
}

- (void)sendStuffP2P:(const std::string &)other_easyrtcid {
    auto text = ToString(_sendTextView.text);
    
    if (_easyrtc->GetConnectStatus(other_easyrtcid) == ert::Easyrtc::ConnectStatus::IsConnected) {
        _easyrtc->SendDataP2P(other_easyrtcid, "msg", Any(text));
    }
    else {
        _easyrtc->ShowError("NOT-CONNECTED",
                            Format("not connected to %s yes.", _easyrtc->IdToName(other_easyrtcid).c_str()));
    }
    
    [self addToConversation:"Me" msgType:"msgtype" content:Any(text)];
    
    _sendTextView.text = @"";
}

- (void)loginSuccess:(const std::string &)easyrtcid
{
    self_easyrtcid_ = easyrtcid;
    _myNameLabel.text = [NSString stringWithFormat:@"I am %@", ToNSString(easyrtcid)];
}

- (void)loginFailure:(const std::string &)code
                text:(const std::string &)text
{
    _easyrtc->ShowError(code, "failure to login");
}

- (IBAction)onTapGesture:(UITapGestureRecognizer *)recr {
    [self.view endEditing:YES];
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
    
    std::string user = occupants_[index].GetAt("easyrtcid").AsString().value();
    [self startCall:user];
    
}
- (void)onSendButton:(UIButton *)button {
    int index = -1;
    for (int i = 0; i < _destViews.count; i++) {
        UIView * destView = _destViews[i];
        if (button == [destView viewWithTag:3]) {
            index = i;
            break;
        }
    }
    if (index == -1) { return; }
    
    std::string user = occupants_[index].GetAt("easyrtcid").AsString().value();
    [self sendStuffP2P:user];
}


- (IBAction)onCloseButton {
    [self dismissViewControllerAnimated:true completion:nil];
}

@end
