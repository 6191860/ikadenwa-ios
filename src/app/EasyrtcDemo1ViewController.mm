//
//  easyrtc_demo_1.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/24.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import "EasyrtcDemo1ViewController.h"

using namespace nwr;

@implementation EasyrtcDemo1ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    nwr_test_set_ = std::make_shared<app::NwrTestSet>();
    nwr_test_set_->TestSio0();
}

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    
    _easyrtc = ert::Easyrtc::Create("http://192.168.1.6:8080/",
                                    ObjcPointerMake(self.view));
    _easyrtc->EnableDebug(true);
    [self connect];
}

- (void)viewWillDisappear:(BOOL)animated {
    
    
    
    [super viewWillDisappear:animated];
}


- (void)addToConversation:(const std::string &)who
msgType:(const std::string &)msgType
content:(const nwr::Any &)content
{
    NSMutableString * text = [NSMutableString stringWithString:_textView.text];
    if (text.length != 0) {
        [text appendString:@"\n"];
    }
    
    std::string content_str = content.AsString() || std::string();
    
    [text appendFormat:@"%s: %s", who.c_str(), content_str.c_str()];
    _textView.text = text;
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
    
}

//
//function convertListToButtons (roomName, occupants, isPrimary) {
//    var otherClientDiv = document.getElementById("otherClients");
//    while (otherClientDiv.hasChildNodes()) {
//        otherClientDiv.removeChild(otherClientDiv.lastChild);
//    }
//    
//    for(var easyrtcid in occupants) {
//        var button = document.createElement("button");
//        button.onclick = function(easyrtcid) {
//            return function() {
//                sendStuffWS(easyrtcid);
//            };
//        }(easyrtcid);
//        var label = document.createTextNode("Send to " + easyrtc.idToName(easyrtcid));
//        button.appendChild(label);
//        
//        otherClientDiv.appendChild(button);
//    }
//    if( !otherClientDiv.hasChildNodes() ) {
//        otherClientDiv.innerHTML = "<em>Nobody else logged in to talk to...</em>";
//    }
//}
//
//
//function sendStuffWS(otherEasyrtcid) {
//    var text = document.getElementById("sendMessageText").value;
//    if(text.replace(/\s/g, "").length === 0) { // Don"t send just whitespace
//        return;
//    }
//    
//    easyrtc.sendDataWS(otherEasyrtcid, "message",  text);
//    addToConversation("Me", "message", text);
//    document.getElementById("sendMessageText").value = "";
//}
//
//

- (void)loginSuccess:(const std::string &)easyrtcid
{
    NSLog(@"I am %s", easyrtcid.c_str());
}
- (void)loginFailure:(const std::string &)code
text:(const std::string &)text
{
    _easyrtc->ShowError(code, text);
}

@end