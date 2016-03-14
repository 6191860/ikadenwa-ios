//
//  Room.m
//  Ikadenwa
//
//  Created by omochimetaru on 2016/03/13.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#import "Room.h"

#import "Context.h"
#import "User.h"

using namespace nwr;
using namespace nwr::jsrtc;

@implementation Room

- (instancetype)initWithContext:(NSObject<Context> *)context
{
    self = [super init];
    if (!self) { return nil; }
    _context = context;
    _easyrtcid = None();
    _users = {};
    _userDict = {};
    _documentTitle = "";
    _roomName = "";
    _userName = "";
    _localStream = nullptr;
    _localMonitor = false;

    //    this.inputVolume = '';
//    ko.track(this);
    
    return self;
}

- (void)activateWithRoomName:(const std::string &)roomName {
    _roomName = roomName;
    _easyrtcid = None();
    _users = {};
    _userDict = {};
    _documentTitle = roomName;
    _userName = "ikadenwa-ios-test"; // TODO
    [self join];
}

- (void)deactivate {
    if (_easyrtcid) {
        [self _logout];
    }
}

- (void)join {
    auto easyrtc = _context.easyrtc;
    
    easyrtc->set_user_name(_userName);
    
    easyrtc->set_room_occupant_listener([self](const Optional<std::string> & roomNameOpt,
                                           const std::map<std::string, Any> & occupants,
                                           const Any & isPrimary)
                                        {
                                            auto roomName = roomNameOpt.value();
                                            
                                            if (roomName != _roomName) {
                                                return;
                                            }
                                            
                                            std::vector<User *> users = {};
                                            
                                            for (std::string easyrtcid : Keys(occupants)) {
                                                Any user = occupants.at(easyrtcid);
                                                
                                                bool found = false;
                                                
                                                for (int i = 0; i < _users.size(); i++) {
                                                    User * _user = _users[i];
                                                    if (_user.easyrtcid == easyrtcid) {
                                                        users.push_back(_user);
                                                        _users.erase(_users.begin() + i);
                                                        found = true;
                                                        break;
                                                    }
                                                }
                                                if (!found) {
                                                    User * newUser = [[User alloc] initWithContext:_context
                                                                                         easyrtcId:user.GetAt("easyrtcid").AsString().value()
                                                                                              name:user.GetAt("username").AsString().value()
                                                                                            joined:user.GetAt("roomJoinTime").AsInt().value() != 0];
                                                    _userDict[newUser.easyrtcid] = newUser;
                                                    users.push_back(newUser);
                                                }
                                            }
                                            
                                            _users = users;
                                            
                                            std::sort(_users.begin(), _users.end(), [](User * a, User * b) {
                                                return a.joined > b.joined ? 1 : -1;
                                            });
                                        });
    
    easyrtc->SetPeerListener(None(), None(),
                             [self](const std::string & easyrtcid,
                                const std::string & messageType,
                                const Any & content,
                                const Any & targeting)
                             {
                                 if (messageType == "mute") {
                                     [_userDict[easyrtcid] muteWithFlag:content.GetAt("mute").AsBoolean().value()];
                                 }
                             });
    easyrtc->set_stream_acceptor([self](const std::string & easyrtcid,
                                        const std::shared_ptr<MediaStream> & stream,
                                        const std::string & stream_name)
                                 {
                                     [_userDict[easyrtcid] embedStream:stream];
                                 });
    easyrtc->set_on_stream_closed([self](const std::string & easyrtcid,
                                         const std::shared_ptr<MediaStream> & stream,
                                         const std::string & stream_name)
                                  {
                                      [_userDict[easyrtcid] removeStream];
                                  });
    
    easyrtc->set_accept_checker([self](const std::string & easyrtcid,
                                       const std::function<void (bool,
                                                                 const Optional<std::vector<std::string>> &)> & callback)
                                {
                                    Timer::Create(TimeDuration(0.01), [callback]() {
                                        callback(true, None());
                                    });
                                });
    //bug in original
    easyrtc->set_on_error([self](const Any & error) {
        std::string errorCode = error.GetAt("errorCode").AsString().value();
        std::string message = error.GetAt("errorText").AsString().value();
        [self handleErrorWithCode:errorCode text:message];
    });
    easyrtc->JoinRoom(_roomName,
                      nullptr,
                      [](const std::string & room_name)
                      {
                          
                      },
                      [self]
                      (const std::string & errorCode, const std::string & message, const std::string & room_name)
                      {
                          [self handleErrorWithCode:errorCode text:message];
                      });
    easyrtc->EnableAudio(true);
    easyrtc->EnableVideo(false);
    easyrtc->EnableVideoReceive(false);
    easyrtc->InitMediaSource(None(),
                             [self, easyrtc](const std::shared_ptr<MediaStream> & stream)
                             {
                                 easyrtc->Connect("squid",
                                                  [self, easyrtc](const std::string & easyrtcid)
                                                  {
                                                      _easyrtcid = Some(easyrtcid);
                                                      
                                                      _localStream = easyrtc->GetLocalStream(None());
                                                      
                                                      
//                                                      var audio = document.createElement('audio');
//                                                      audio.setAttribute('id', 'localStream');
//                                                      audio.autoplay = true;
//                                                      audio.muted = true;
//                                                      document.body.appendChild(audio);
//                                                      easyrtc.setVideoObjectSrc(audio, _this.localStream);
                                                      
                                                      _localMonitor = false;
                                                  },
                                                  [self](const std::string & code, const std::string & text)
                                                  {
                                                      [self handleErrorWithCode:code text:text];
                                                  });
                             },
                             [self](const std::string & code, const std::string & message)
                             {
                                 [self handleErrorWithCode:code text:message];
                             });
}

- (void)muteAll {
    for (User * user : _users) {
        [user requestMute];
    }
}

- (void)unmuteAll {
    for (User * user : _users) {
        [user requestUnmute];
    }
}

- (void)toggleMonitor {
    //TODO
//    easyrtc.muteVideoObject(document.getElementById('localStream'), this.localMonitor);
    _localMonitor = !_localMonitor;
}

- (void)_logout {
    if (_localMonitor) {
        [self toggleMonitor];
    }
    
    _context.easyrtc->HangupAll();
    _context.easyrtc->Disconnect();

    _easyrtcid = None();
    
//    var localStream = document.getElementById('localStream');
//    localStream.parentElement.removeChild(localStream);
//    this.localStream = null;
//    sessionStorage.setItem('userName', '');
}

- (void)logout {
    [self _logout];
//    TODO
//    router.navigate('');
}

- (void)handleErrorWithCode:(const std::string &)code
                       text:(const std::string &)text
{
    printf("error %s; %s\n", code.c_str(), text.c_str());
}

@end
